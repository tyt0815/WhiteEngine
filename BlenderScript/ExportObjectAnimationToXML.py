import bpy
import xml.etree.ElementTree as ET
from xml.dom import minidom
import os
import math

def export_all_fcurves_to_xml(file_path):
    print(f"{file_path} 내보내기 시작...")

    root = ET.Element("ObjectAnimation")
    
    # 기본 정보 추가
    info = ET.SubElement(root, "Info")
    info.set("fps", str(bpy.context.scene.render.fps))
    info.set("frame_start", str(bpy.context.scene.frame_start))
    info.set("frame_end", str(bpy.context.scene.frame_end))

    for obj in bpy.context.selected_objects:
        if not obj or not obj.animation_data or not obj.animation_data.action:
            print(f"에러: '{obj.name if obj else 'None'}'에 애니메이션 액션이 없습니다.")
            continue
        
        obj_element = ET.SubElement(root, "Object", name=obj.name)
        
        action = obj.animation_data.action

        # 3. 모든 F-Curves 순회
        for fcurve in action.fcurves:
            data_path = fcurve.data_path
            array_index = fcurve.array_index
            
            # 이름 결정 로직 (f-string 밖에서 미리 처리)
            # DX12 좌표계에 맞춰서 Y, Z 변경
            axis_map = {0: "X", 1: "Z", 2: "Y", 3: "W"}
            
            bRadian = False
            if data_path in "location":
                prop_display_name = f"Location{axis_map.get(array_index, array_index)}"
            elif data_path in "rotation_euler":
                prop_display_name = f"Rotation{axis_map.get(array_index, array_index)}"
                bRadian = True
            elif data_path in "scale":
                prop_display_name = f"Scale{axis_map.get(array_index, array_index)}"
            elif data_path.startswith('["'):
                # 커스텀 프로퍼티 이름 추출 (예: ["Speed"] -> Speed)
                clean_name = data_path.strip('[]" ')
                prop_display_name = clean_name
                data_path = "Property"
            else:
                prop_display_name = f"{data_path}_{array_index}"

            curve_node = ET.SubElement(obj_element, "Curve")
            curve_node.set("data_path", data_path)
            # curve_node.set("index", str(array_index))
            curve_node.set("name", prop_display_name)
            
            # 4. 키프레임 포인트 추출
            for keypoint in fcurve.keyframe_points:
                kp_node = ET.SubElement(curve_node, "Keyframe")
                kp_node.set("frame", f"{keypoint.co[0]:.2f}")
                kp_node.set("value", f"{(math.degrees(keypoint.co[1]) if bRadian else keypoint.co[1]):.4f}")
                kp_node.set("interp", keypoint.interpolation)
                
                kp_node.set("h_left_x", f"{keypoint.handle_left[0]:.4f}")
                kp_node.set("h_left_y", f"{keypoint.handle_left[1]:.4f}")
                kp_node.set("h_right_x", f"{keypoint.handle_right[0]:.4f}")
                kp_node.set("h_right_y", f"{keypoint.handle_right[1]:.4f}")
                

    # 5. 파일 저장
    # 폴더가 없으면 생성
    os.makedirs(os.path.dirname(os.path.split(file_path)[0]), exist_ok=True)
    
    xml_str = minidom.parseString(ET.tostring(root)).toprettyxml(indent="   ")
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(xml_str)
        
    print(f"성공: {file_path}에 데이터가 저장되었습니다.")