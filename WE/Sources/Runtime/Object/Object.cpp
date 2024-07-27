#include "Object.h"

XMMATRIX WObject::GetWorldMatrix()
{
    XMMATRIX S = XMMatrixScaling(Transform.Scale.x, Transform.Scale.y, Transform.Scale.z);
    XMMATRIX R = XMMatrixRotationX(Transform.Rotation.x) * XMMatrixRotationY(Transform.Rotation.y) * XMMatrixRotationZ(Transform.Rotation.z);
    XMMATRIX T = XMMatrixTranslation(Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);
    return S*R*T;
}
