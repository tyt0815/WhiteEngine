import os
from moviepy import VideoFileClip
import moviepy.video.fx as vfx

def make_gif(input_path, output_path, start_time, end_time, fps=18, width=1200, height=360):
    if not os.path.exists(input_path):
        print(f"파일을 찾을 수 없습니다: {input_path}")
        return

    try:
        # 1. 영상 불러오기
        with VideoFileClip(input_path) as clip:
            # 2. 구간 자르기 (subclipped 또는 subclip)
            if hasattr(clip, 'subclipped'):
                sub_clip = clip.subclipped(start_time, end_time)
            else:
                sub_clip = clip.subclip(start_time, end_time)
            
            # 3. 가로(Width) 기준으로 먼저 리사이즈 (비율 유지)
            if width:
                if hasattr(sub_clip, 'resized'):
                    sub_clip = sub_clip.resized(width=width)
                else:
                    sub_clip = sub_clip.resize(width=width)
            
            # 4. 세로(Height) 기준으로 중앙 클리핑
            current_w, current_h = sub_clip.size
            if height and current_h > height:
                # 중앙 지점 계산
                y1 = (current_h - height) // 2
                y2 = y1 + height
                
                # cropped 호출 (최신/구버전 호환)
                if hasattr(sub_clip, 'cropped'):
                    sub_clip = sub_clip.cropped(y1=y1, y2=y2)
                else:
                    # 구버전이나 fx 사용 시
                    sub_clip = vfx.crop(sub_clip, y1=y1, y2=y2)
                print(f"클리핑 적용: 중앙 기준 세로 {height}px 남김")
            
            print(f"변환 시작: {start_time} ~ {end_time} ({width}x{height})")
            
            # 5. GIF 저장 (용량 최적화 옵션)
            sub_clip.write_gif(output_path, fps=fps)
            print(f"성공적으로 저장되었습니다: {output_path}")
            
    except Exception as e:
        print(f"에러 발생: {e}")

if __name__ == "__main__":
    # 파일 경로와 시간을 확인하세요
    in_path = "TopAttack 궤적(막힘).mp4"
    folder_path, file_name = os.path.split(in_path)
    out_path = os.path.join(folder_path, file_name.split('.')[0] + '.gif')
    
    make_gif(in_path, out_path, 2.9, 6.9, width=1280, height=720)