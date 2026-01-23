import xml.etree.ElementTree as ET
import struct
import zlib

# EInterpolationType 매핑 (블렌더 문자열 -> C++ Enum)
INTERP_MAP = {
    "LINEAR": 0,
    "BEZIER": 1,
    "CONSTANT": 2,
    "UNDEFINED": 3
}

def xml_to_keyframe_map_zlib(xml_input_path, bin_output_path):
    tree = ET.parse(xml_input_path)
    root = tree.getroot()
    
    curves_element = root.find("AnimationCurves")
    if curves_element is None:
        return
        
    curves = curves_element.findall("Curve")
    
    # 데이터를 임시로 저장할 바이트 배열
    payload = bytearray()

    # 0. 프레임 정보 기록
    info_element = root.find("Info")
    fps = float(info_element.get("fps"))
    frame_start = float(info_element.get("frame_start"))
    frame_end = float(info_element.get('frame_end'))
    payload.extend(struct.pack("ff", fps, frame_end - frame_start))
    
    
    # 1. 전체 커브 개수 기록 (int)
    payload.extend(struct.pack("i", len(curves)))
    
    for curve in curves:
        name_str = curve.get("name")
        name = name_str.encode('utf-8')
        keyframes = curve.findall("Keyframe")
        
        # 2. 커브 이름 정보 (길이 + 문자열)
        payload.extend(struct.pack("i", len(name)))
        payload.extend(name)
        
        # 3. 이 커브의 키프레임 개수
        payload.extend(struct.pack("i", len(keyframes)))
        
        # 4. FKeyframe 데이터 패킹
        for kp in keyframes:
            frame = float(kp.get("frame"))
            value = float(kp.get("value"))
            interp_enum = INTERP_MAP.get(kp.get("interp").upper(), 0)
            
            lx = float(kp.get("h_left_x"))
            ly = float(kp.get("h_left_y"))
            rx = float(kp.get("h_right_x"))
            ry = float(kp.get("h_right_y"))
            
            # C++ FKeyframe 구조체 순서: Frame, Value, Interp, RightH, LeftH
            payload.extend(struct.pack("ffiffff", 
                frame - frame_start, value, interp_enum, 
                rx- frame_start, ry,  # RightHandle (XMFLOAT2)
                lx- frame_start, ly   # LeftHandle (XMFLOAT2)
            ))

    # 5. 완성된 바이너리 덩어리를 zlib으로 압축
    original_size = len(payload)
    compressed_data = zlib.compress(payload, level=6)

    # 6. 파일 저장 (헤더 4바이트: 원본 크기 + 나머지: 압축 데이터)
    with open(bin_output_path, "wb") as f:
        f.write(struct.pack("I", original_size))
        f.write(compressed_data)

    print(f"Success: {xml_input_path} -> {bin_output_path}")
    print(f"Size: {original_size} -> {len(compressed_data)} bytes (Ratio: {len(compressed_data)/original_size*100:.1f}%)")

filename = 'Medium_v1'
xml_to_keyframe_map_zlib(filename + ".xml", filename + ".keyframemap.zbin")