import xml.etree.ElementTree as ET
import math
from xml.dom import minidom

def generate_large_animation_xml(filename, n_frames, fps=24):
    # 루트 노드 생성
    root = ET.Element("ObjectAnimation", name="LargeTestProjectile")
    
    # 기본 정보 설정
    ET.SubElement(root, "Info", fps=str(fps), frame_start="0", frame_end=str(n_frames))
    
    curves_node = ET.SubElement(root, "AnimationCurves")
    
    # 생성할 커브 정의 (data_path, name)
    curve_definitions = [
        ("location", "LocationX"), ("location", "LocationY"), ("location", "LocationZ"),
        ("rotation_euler", "RotationX"), ("rotation_euler", "RotationY"), ("rotation_euler", "RotationZ"),
        ("scale", "ScaleX"), ("scale", "ScaleY"), ("scale", "ScaleZ")
    ]
    
    for data_path, curve_name in curve_definitions:
        curve = ET.SubElement(curves_node, "Curve", data_path=data_path, name=curve_name)
        
        for frame in range(n_frames + 1):
            f = float(frame)
            
            # 테스트용 데이터 생성 로직 (축별로 변화를 주기 위함)
            if "Location" in curve_name:
                value = math.sin(f * 0.1) * 10.0
            elif "Rotation" in curve_name:
                value = (f / n_frames) * math.pi * 2.0
            elif "Scale" in curve_name:
                value = 1.0 + math.cos(f * 0.1) * 0.5
            else:
                value = 0.0
            
            # 키프레임 추가
            # 핸들(h_left, h_right) 값은 프레임 전후 1.0 간격으로 기본 설정
            ET.SubElement(curve, "Keyframe", 
                          frame=f"{f:.2f}", 
                          value=f"{value:.6f}", 
                          interp="LINEAR", 
                          h_left_x=f"{f-1.0:.4f}", h_left_y=f"{value:.4f}",
                          h_right_x=f"{f+1.0:.4f}", h_right_y=f"{value:.4f}")

    # 보기 좋게 들여쓰기 포함하여 저장
    xml_str = ET.tostring(root, encoding='utf-8')
    pretty_xml_str = minidom.parseString(xml_str).toprettyxml(indent="   ")
    
    with open(filename, "w", encoding="utf-8") as f:
        f.write(pretty_xml_str)

    print(f"성공: {filename}에 {n_frames} 프레임 데이터가 생성되었습니다.")

# 실행: 1000프레임 대용량 테스트 파일 생성
generate_large_animation_xml("D:/temp/Large.xml", n_frames=10000, fps=24)