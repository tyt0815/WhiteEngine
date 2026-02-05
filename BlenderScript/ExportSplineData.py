import bpy
import json
import os
import math

def export_spline_to_json(filepath):
    # 디렉토리가 없으면 생성
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    
    obj = bpy.context.active_object
    if not obj or obj.type != 'CURVE':
        print("곡선 오브젝트를 선택해주세요.")
        return

    data = []
    for spline in obj.data.splines:
        # 현재 스플라인의 포인트들 추출
        points = spline.bezier_points
        count = len(points)
        
        if count == 0:
            continue

        # 모든 제어점 순회하며 추가
        for p in points:
            data.append({
                "ControlPoint": [p.co.x, p.co.z, p.co.y],
                "LeftHandle": [p.handle_left.x, p.handle_left.z, p.handle_left.y],
                "RightHandle": [p.handle_right.x, p.handle_right.z, p.handle_right.y],
                "Property1": p.radius,
                "Property2": math.degrees(p.tilt)
            })

        # 핵심: 곡선이 닫혀있다면(Cyclic), 첫 번째 점을 마지막에 복사해서 추가
        if spline.use_cyclic_u:
            first_p = points[0]
            data.append({
                "ControlPoint": [first_p.co.x, first_p.co.z, first_p.co.y],
                "LeftHandle": [first_p.handle_left.x, first_p.handle_left.z, first_p.handle_left.y],
                "RightHandle": [first_p.handle_right.x, first_p.handle_right.z, first_p.handle_right.y],
                "Property1": first_p.radius,
                "Property2": math.degrees(first_p.tilt)
            })

    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)
    
    print(f"Export 완료: {filepath}")

# 실행
export_spline_to_json("./Temp/RingSpline.json")