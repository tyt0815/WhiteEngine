import bpy
import math

def create_heavy_keyframes(n_frames):
    # 현재 선택된 오브젝트 가져오기
    obj = bpy.context.active_object
    
    if not obj:
        print("선택된 오브젝트가 없습니다!")
        return

    print(f"{obj.name}에 {n_frames}개의 키프레임을 생성을 시작합니다...")

    # 기존 애니메이션 데이터 삭제 (깔끔하게 새로 만들기 위함)
    obj.animation_data_clear()

    for f in range(n_frames + 1):
        # 1. 프레임 설정
        bpy.context.scene.frame_set(f)
        
        # 2. 임의의 수치 변화 (나선형으로 움직이며 크기와 회전이 변함)
        t = f * 0.1
        obj.location = (math.cos(t) * 10, math.sin(t) * 10, f * 0.05)
        obj.rotation_euler = (t, t * 0.5, t * 0.2)
        obj.scale = (1.0 + math.sin(t) * 0.5, 1.0 + math.cos(t) * 0.5, 1.0)

        # 3. 키프레임 삽입 (Location, Rotation, Scale 전부)
        obj.keyframe_insert(data_path="location", index=-1)
        obj.keyframe_insert(data_path="rotation_euler", index=-1)
        obj.keyframe_insert(data_path="scale", index=-1)

    # 4. 타임라인 종료 프레임 업데이트
    bpy.context.scene.frame_end = n_frames
    print(f"완료: {n_frames} 프레임까지 키프레임 생성이 끝났습니다.")

# --- 설정: 여기서 원하는 프레임 수를 입력하세요 ---
create_heavy_keyframes(1000)