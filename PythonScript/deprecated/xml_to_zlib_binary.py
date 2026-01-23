import zlib

def xml_to_compressed_binary(xml_path, bin_path):
    with open(xml_path, 'rb') as f_xml:
        raw_data = f_xml.read()
        
    # zlib을 이용한 압축 (수준 1~9, 9가 최대 압축)
    compressed_data = zlib.compress(raw_data, level=6)
    
    with open(bin_path, 'wb') as f_bin:
        # 나중에 해제할 때를 위해 원본 사이즈를 헤더(4바이트)에 기록하면 좋음
        f_bin.write(len(raw_data).to_bytes(4, 'little'))
        f_bin.write(compressed_data)
        
    print(f"압축 완료: {len(raw_data)} -> {len(compressed_data)} bytes")


filename = 'Medium_v2';
xml_to_compressed_binary(filename + ".xml", filename + ".zbin")