# 1. 최적화 이전 초기 기록
![](./v1_LoadTime.png)

# 2. XML 구조 변경
![](./v2_LoadTime.png)

1. Keyframe 블록 이름 단순화
2. frame을 정수타입, 소수점 4자리로 제한
3. left handle은 이전 keyframe의 보간이 BEZIER일때만, right handle은 현재 keyframe의 보간이 BEZIER일때만 생성하도록 변경



#### 2.1. 변경 전
![](./v2_변경_전_구조.png)
![](./v2_변경_전_파일크기.png)

#### 2.2. 변경후
![](./v2_변경_후_구조.png)
![](./v2_변경_후_파일크기.png)