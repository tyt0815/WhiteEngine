import bpy
import xml.etree.ElementTree as ET
from xml.dom import minidom
import os

def export_all_fcurves_to_xml(folder_path, obj):
    obj = bpy.context.active_object
    
    # 1. 애니메이션 데이터 존재 확인
    if not obj or not obj.animation_data or not obj.animation_data.action:
        print(f"에러: '{obj.name if obj else 'None'}'에 애니메이션 액션이 없습니다.")
        return

    # 2. XML 루트 생성
    root = ET.Element("ObjectAnimation", name=obj.name)
    root.set("version", f"v{2}")
    
    # 기본 정보 추가
    info = ET.SubElement(root, "Info")
    info.set("fps", str(bpy.context.scene.render.fps))
    info.set("frame_start", str(bpy.context.scene.frame_start))
    info.set("frame_end", str(bpy.context.scene.frame_end))

    curves_element = ET.SubElement(root, "AnimationCurves")
    action = obj.animation_data.action

    # 3. 모든 F-Curves 순회
    for fcurve in action.fcurves:
        data_path = fcurve.data_path
        array_index = fcurve.array_index
        
        # 이름 결정 로직 (f-string 밖에서 미리 처리)
        # DX12 좌표계에 맞춰서 Y, Z 변경
        axis_map = {0: "X", 1: "Z", 2: "Y", 3: "W"}
        
        if data_path in "location":
            prop_display_name = f"Location{axis_map.get(array_index, array_index)}"
        elif data_path in "rotation_euler":
            prop_display_name = f"Rotation{axis_map.get(array_index, array_index)}"
        elif data_path in "scale":
            prop_display_name = f"Scale{axis_map.get(array_index, array_index)}"
        elif data_path.startswith('["'):
            # 커스텀 프로퍼티 이름 추출 (예: ["Speed"] -> Speed)
            clean_name = data_path.strip('[]" ')
            prop_display_name = clean_name
            data_path = "Property"
        else:
            prop_display_name = f"{data_path}_{array_index}"

        curve_node = ET.SubElement(curves_element, "Curve")
        curve_node.set("data_path", data_path)
        # curve_node.set("index", str(array_index))
        curve_node.set("name", prop_display_name)
        
        # 4. 키프레임 포인트 추출
        prev_interp = "LINEAR"
        for keypoint in fcurve.keyframe_points:
            kp_node = ET.SubElement(curve_node, "KF")
            kp_node.set("f", str(int(round(keypoint.co[0]))))
            kp_node.set("v", f"{keypoint.co[1]:.4f}")
            kp_node.set("i", keypoint.interpolation)
            
            if prev_interp == "BEZIER":
                kp_node.set("hlx", f"{keypoint.handle_left[0]:.4f}")
                kp_node.set("hly", f"{keypoint.handle_left[1]:.4f}")
            
            if keypoint.interpolation == "BEZIER":
                kp_node.set("hrx", f"{keypoint.handle_right[0]:.4f}")
                kp_node.set("hry", f"{keypoint.handle_right[1]:.4f}")

            prev_interp = keypoint.interpolation
            

    # 5. 파일 저장
    # 폴더가 없으면 생성
    os.makedirs(os.path.dirname(folder_path), exist_ok=True)
    
    xml_str = minidom.parseString(ET.tostring(root)).toprettyxml(indent="   ")
    file_path = os.path.join(folder_path, obj.name + ".xml")
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(xml_str)
        
    print(f"성공: {file_path}에 데이터가 저장되었습니다.")

# 실행 경로 설정
export_all_fcurves_to_xml("D:/temp", bpy.context.active_object)