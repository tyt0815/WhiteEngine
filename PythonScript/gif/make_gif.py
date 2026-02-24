import os
from moviepy import VideoFileClip
import moviepy.video.fx as vfx

def make_gif(input_path, output_path, start_time, end_time, fps=18, width=1200, height=360, resize_first=True):
    """
    resize_first=True: 가로 폭에 맞춰 리사이즈 후 세로를 자름 (비율 유지 시도)
    resize_first=False: 원본 해상도에서 바로 지정한 크기만큼 중앙을 자름 (리사이즈 없음)
    """
    if not os.path.exists(input_path):
        print(f"파일을 찾을 수 없습니다: {input_path}")
        return

    try:
        with VideoFileClip(input_path) as clip:
            # 1. 구간 자르기
            sub_clip = clip.subclipped(start_time, end_time) if hasattr(clip, 'subclipped') else clip.subclip(start_time, end_time)
            
            if resize_first:
                # [방식 A] 리사이즈 후 클리핑 (기존 방식)
                if width:
                    sub_clip = sub_clip.resized(width=width) if hasattr(sub_clip, 'resized') else sub_clip.resize(width=width)
            
            # 현재 클립의 크기 확인
            current_w, current_h = sub_clip.size
            
            # 2. 클리핑 계산 (중앙 기준)
            # 가로(width)와 세로(height)가 원본보다 작을 때만 자르기 수행
            target_w = width if width and width < current_w else current_w
            target_h = height if height and height < current_h else current_h
            
            x1, y1 = (current_w - target_w) // 2, (current_h - target_h) // 2
            x2, y2 = x1 + target_w, y1 + target_h
            
            # 3. 크롭 적용
            if hasattr(sub_clip, 'cropped'):
                sub_clip = sub_clip.cropped(x1=x1, y1=y1, x2=x2, y2=y2)
            else:
                sub_clip = vfx.crop(sub_clip, x1=x1, y1=y1, x2=x2, y2=y2)
                
            print(f"작업 모드: {'리사이즈 후 크롭' if resize_first else '원본에서 바로 크롭'}")
            print(f"최종 크기: {sub_clip.size[0]}x{sub_clip.size[1]}")
            
            # 4. GIF 저장
            sub_clip.write_gif(output_path, fps=fps)
            print(f"성공적으로 저장되었습니다: {output_path}")
            
    except Exception as e:
        print(f"에러 발생: {e}")

if __name__ == "__main__":
    in_path = "BP_MissileSwarmProj_.mp4"
    folder_path, file_name = os.path.split(in_path)
    out_path = os.path.join(folder_path, file_name.split('.')[0] + '.gif')
    
    # 예시 1: 리사이즈 없이 원본에서 1280x720 영역만 중앙에서 도려내기
    make_gif(in_path, out_path, 4, 9, fps = 30, width=1280, height=720, resize_first=True)