### keyframe 수
- Medium: 10000
- Large: 100000

# 1. 최적화 이전 초기 기록
![](./v1_LoadTime.png)

![](./v1_파일크기.png)

![](./v2_변경_전_구조.png)

# 2. XML 구조 변경
![](./v2_LoadTime.png)

![](./v2_파일크기.png)

![](./v2_변경_후_구조.png)

1. Keyframe 블록 이름 단순화
2. frame을 정수타입, 소수점 4자리로 제한
3. left handle은 이전 keyframe의 보간이 BEZIER일때만, right handle은 현재 keyframe의 보간이 BEZIER일때만 생성하도록 변경


# 3. xml + zlib 압축
![](./v3_LoadTime.png)

![](./v3_파일크기.png)

# 4. binary + zlib 압축

# 5. binary + lz4 압축