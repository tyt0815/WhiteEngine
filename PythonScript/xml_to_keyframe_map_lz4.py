import xml.etree.ElementTree as ET
import struct
import lz4.frame  # lz4.frame 또는 lz4.block 사용 가능
import lz4.block

# EInterpolationType 매핑
INTERP_MAP = {
    "LINEAR": 0,
    "BEZIER": 1,
    "CONSTANT": 2,
    "UNDEFINED": 3
}

def xml_to_keyframe_map_lz4(xml_input_path, bin_output_path):
    tree = ET.parse(xml_input_path)
    root = tree.getroot()
    
    curves_element = root.find("AnimationCurves")
    if curves_element is None: return
    curves = curves_element.findall("Curve")
    
    payload = bytearray()

    # 0. Info 정보 (FPS, Duration)
    info_element = root.find("Info")
    fps = float(info_element.get("fps"))
    frame_start = float(info_element.get("frame_start"))
    frame_end = float(info_element.get('frame_end'))
    payload.extend(struct.pack("ff", fps, frame_end - frame_start))
    
    # 1. 전체 커브 개수
    payload.extend(struct.pack("i", len(curves)))
    
    for curve in curves:
        name_str = curve.get("name")
        name = name_str.encode('utf-8')
        keyframes = curve.findall("Keyframe")
        
        # 2. 커브 이름 정보
        payload.extend(struct.pack("i", len(name)))
        payload.extend(name)
        
        # 3. 키프레임 개수
        payload.extend(struct.pack("i", len(keyframes)))
        
        # 4. FKeyframe 데이터 패킹
        for kp in keyframes:
            frame = float(kp.get("frame"))
            value = float(kp.get("value"))
            interp_enum = INTERP_MAP.get(kp.get("interp").upper(), 0)
            
            lx, ly = float(kp.get("h_left_x")), float(kp.get("h_left_y"))
            rx, ry = float(kp.get("h_right_x")), float(kp.get("h_right_y"))
            
            payload.extend(struct.pack("ffiffff", 
                frame - frame_start, value, interp_enum, 
                rx - frame_start, ry, 
                lx - frame_start, ly 
            ))

    # 5. LZ4 압축 (block 방식이 게임 엔진에서 제어하기 더 편합니다)
    original_size = len(payload)
    # store_size=False로 설정하여 순수 데이터만 얻고, 크기는 우리가 직접 관리합니다.
    compressed_data = lz4.block.compress(payload, store_size=False)

    # 6. 파일 저장 (헤더: 원본 크기 + 데이터: 압축 데이터)
    with open(bin_output_path, "wb") as f:
        f.write(struct.pack("I", original_size))
        f.write(compressed_data)

    print(f"LZ4 Success: {xml_input_path} -> {bin_output_path}")
    print(f"Size: {original_size} -> {len(compressed_data)} bytes ({len(compressed_data)/original_size*100:.1f}%)")

filename = 'Medium_v1'
xml_to_keyframe_map_lz4(filename + ".xml", filename + ".keyframemap.lz4")