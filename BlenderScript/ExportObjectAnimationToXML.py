import bpy
import xml.etree.ElementTree as ET
from xml.dom import minidom
import os
import math

def export_all_fcurves_to_xml(file_path):
    print(f"{file_path} 내보내기 시작...")

    root = ET.Element("ObjectAnimation")
    
    info = ET.SubElement(root, "Info")
    info.set("fps", str(bpy.context.scene.render.fps))
    info.set("frame_start", str(bpy.context.scene.frame_start))
    info.set("frame_end", str(bpy.context.scene.frame_end))

    for obj in bpy.context.selected_objects:
        if not obj or not obj.animation_data or not obj.animation_data.action:
            print(f"에러: '{obj.name}'에 애니메이션 액션이 없습니다.")
            continue
        
        obj_element = ET.SubElement(root, "Object", name=obj.name)
        action = obj.animation_data.action

        for fcurve in action.fcurves:
            data_path = fcurve.data_path
            array_index = fcurve.array_index
            
            # --- 좌표계 변환 로직 (Blender -> DX12) ---
            # Blender(RH, Z-up) -> DX12(LH, Y-up)
            # 위치: X->X, Y->Z, Z->Y
            # 회전: X->-X, Y->-Z, Z->-Y (왼손 좌표계 반전 대응)
            
            axis_map = {0: "X", 1: "Z", 2: "Y"} # 인덱스 스왑 (Blender 1(Y) -> DX 2(Z) / Blender 2(Z) -> DX 1(Y))
            
            bRadian = False
            bFlipValue = False # 부호 반전 여부
            
            if data_path == "location":
                prop_display_name = f"Location{axis_map.get(array_index, array_index)}"
            
            elif data_path == "rotation_euler":
                prop_display_name = f"Rotation{axis_map.get(array_index, array_index)}"
                bRadian = True
                # 왼손 좌표계로 변환 시 모든 축의 회전 방향을 반전시켜야 하는 경우가 많습니다.
                bFlipValue = True 
                
            elif data_path == "scale":
                prop_display_name = f"Scale{axis_map.get(array_index, array_index)}"
                
            elif data_path.startswith('["'):
                clean_name = data_path.strip('[]" ')
                prop_display_name = clean_name
                data_path = "Property"
            else:
                prop_display_name = f"{data_path}_{array_index}"

            curve_node = ET.SubElement(obj_element, "Curve")
            curve_node.set("data_path", data_path)
            curve_node.set("name", prop_display_name)
            
            for keypoint in fcurve.keyframe_points:
                kp_node = ET.SubElement(curve_node, "Keyframe")
                kp_node.set("frame", f"{keypoint.co[0]:.2f}")
                
                # 최종 값 계산
                final_val = keypoint.co[1]
                if bRadian:
                    final_val = math.degrees(final_val)
                
                # 왼손 좌표계 대응을 위한 부호 반전
                if bFlipValue:
                    final_val = -final_val
                
                kp_node.set("value", f"{final_val:.4f}")
                kp_node.set("interp", keypoint.interpolation)
                
                # 핸들 좌표도 동일한 부호 반전 적용 (베지에 곡선 유지용)
                h_left_y = -keypoint.handle_left[1] if bFlipValue else keypoint.handle_left[1]
                h_right_y = -keypoint.handle_right[1] if bFlipValue else keypoint.handle_right[1]
                
                kp_node.set("h_left_x", f"{keypoint.handle_left[0]:.4f}")
                kp_node.set("h_left_y", f"{h_left_y:.4f}")
                kp_node.set("h_right_x", f"{keypoint.handle_right[0]:.4f}")
                kp_node.set("h_right_y", f"{h_right_y:.4f}")

    # 파일 저장 부분
    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    xml_str = minidom.parseString(ET.tostring(root)).toprettyxml(indent="   ")
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(xml_str)
    print(f"성공: {file_path}")

# 경로 r 붙여서 실행
export_all_fcurves_to_xml(r"C:\Users\tyt08\Documents\_Mugung\Project\WhiteEngine\Resources\Temp\ColdLaunch.xml")