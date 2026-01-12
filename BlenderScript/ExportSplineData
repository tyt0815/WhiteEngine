import bpy
import json

def export_spline_to_json(filepath):
    obj = bpy.context.active_object
    if obj.type != 'CURVE':
        return

    data = []
    for spline in obj.data.splines:
        for p in spline.bezier_points:
            data.append(
            {
                "ControlPoint": [p.co.x, p.co.z, p.co.y],
                "LeftHandle": [p.handle_left.x, p.handle_left.z, p.handle_left.y],
                "RightHandle": [p.handle_right.x, p.handle_right.z, p.handle_right.y],
                "Property1": p.weight_softbody,
                "Property2": p.radius
            })

    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)

# 실행
export_spline_to_json("D:/WhiteEngine/Resources/JSON/SplineData.json")