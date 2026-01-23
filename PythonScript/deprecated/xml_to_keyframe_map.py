import xml.etree.ElementTree as ET
import struct

# EInterpolationType 매핑 (블렌더 문자열 -> C++ Enum)
# C++ 코드의 enum 순서와 반드시 일치해야 합니다.
INTERP_MAP = {
    "LINEAR": 0,
    "BEZIER": 1,
    "CONSTANT": 2,
    "UNDEFINED": 3
}

def xml_to_keyframe_map(xml_input_path, bin_output_path):
    tree = ET.parse(xml_input_path)
    root = tree.getroot()
    
    # AnimationCurves 노드 찾기
    curves_element = root.find("AnimationCurves")
    if curves_element is None:
        return
        
    curves = curves_element.findall("Curve")
    
    with open(bin_output_path, "wb") as f:
        # 1. 전체 커브 개수 기록
        f.write(struct.pack("i", len(curves)))
        
        for curve in curves:
            # 사용자가 지정한 이름 (예: LocationX, Speed 등)
            name = curve.get("name").encode('utf-8')
            keyframes = curve.findall("Keyframe")
            
            # 2. 커브 이름 정보 (길이 + 문자열)
            f.write(struct.pack("i", len(name)))
            f.write(name)
            
            # 3. 이 커브의 키프레임 개수
            f.write(struct.pack("i", len(keyframes)))
            
            # 4. FKeyframe 구조체 데이터 (순서 엄수)
            # 구조체 레이아웃: Frame(f), Value(f), Interp(i), RightH(ff), LeftH(ff)
            # 포맷: 'ffiffff'
            for kp in keyframes:
                frame = float(kp.get("frame"))
                value = float(kp.get("value"))
                
                # 인터폴레이션 변환 (대문자로 오므로 대문자 체크)
                interp_str = kp.get("interp").upper()
                interp_enum = INTERP_MAP.get(interp_str, 0)
                
                # 핸들 좌표 (Right 먼저, 그다음 Left - C++ 구조체 순서에 맞춤)
                # 만약 C++ 구조체 순서가 Left가 먼저라면 아래 rx, ry와 lx, ly 순서를 바꾸세요.
                lx = float(kp.get("h_left_x"))
                ly = float(kp.get("h_left_y"))
                rx = float(kp.get("h_right_x"))
                ry = float(kp.get("h_right_y"))
                
                # 데이터 패킹 (28 bytes 전후 예상)
                f.write(struct.pack("ffiffff", 
                    frame, value, interp_enum, 
                    rx, ry,  # RightHandle (XMFLOAT2)
                    lx, ly   # LeftHandle (XMFLOAT2)
                ))

    print(f"Success: {xml_input_path} -> {bin_output_path}")

filename = 'Medium_v1';
xml_to_keyframe_map(filename + ".xml", filename + ".keyframemap")