import os
import subprocess
from PIL import Image

def generate_dds_texture(n, color, output_name="result.dds"):
    """
    n: 이미지 크기 (픽셀)
    color: RGB 튜플 (예: (255, 0, 0))
    output_name: 최종 저장될 DDS 파일명
    """
    
    # 1. 경로 설정 (스크립트와 같은 위치의 texconv.exe)
    current_dir = os.path.dirname(os.path.abspath(__file__))
    texconv_path = os.path.join(current_dir, "texconv.exe")
    temp_png = "temp_color_source.png"
    
    # texconv.exe 존재 여부 확인
    if not os.path.exists(texconv_path):
        print(f"오류: {texconv_path} 파일을 찾을 수 없습니다.")
        return

    # 2. Pillow를 사용하여 n*n 단색 PNG 생성
    try:
        # DirectX 연동을 위해 RGBA 모드 권장 (필요시 RGB로 변경 가능)
        img = Image.new("RGBA", (n, n), color + (255,)) 
        img.save(temp_png)
        print(f"단계 1: 임시 PNG 생성 완료 ({n}x{n})")
    except Exception as e:
        print(f"PNG 생성 중 오류: {e}")
        return

    # 3. texconv.exe를 사용하여 DDS로 변환
    # 옵션 설명:
    # -f BC7_UNORM: 고품질 압축 포맷 (DirectX 11/12 표준)
    # -y: 기존 파일이 있으면 덮어쓰기
    # -m 1: 밉맵 생성 안 함 (단색 텍스처이므로 1단계만 생성하여 용량 절약)
    #      만약 거리에 따른 밉맵이 필요하면 -m 0 으로 수정하세요.
    command = [
        texconv_path,
        "-f", "BC7_UNORM",
        "-m", "1",
        "-y",
        temp_png
    ]

    try:
        result = subprocess.run(command, capture_output=True, text=True)
        
        if result.returncode == 0:
            # texconv는 기본적으로 입력파일명의 확장자만 바꿈 (temp_color_source.dds)
            generated_dds = temp_png.replace(".png", ".dds")
            
            # 4. 파일 이름 최종 변경 및 정리
            if os.path.exists(output_name):
                os.remove(output_name)
            os.rename(generated_dds, output_name)
            
            print(f"단계 2: DDS 변환 성공 -> {output_name}")
        else:
            print(f"texconv 실행 오류: {result.stderr}")
            
    finally:
        # 임시 PNG 파일 삭제
        if os.path.exists(temp_png):
            os.remove(temp_png)

# --- 사용 예시 ---
if __name__ == "__main__":
    # 원하는 크기(n), 색상(RGB), 파일명 설정
    # 예: 1024x1024 크기의 보라색 텍스처 생성
    generate_dds_texture(512, (0, 256, 0), "Green.dds")