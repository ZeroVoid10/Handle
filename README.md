# Handle

Handle with NRF.

接收板接收后发送can消息.

## 按键数据

CAN标准帧, id 325.

使用`can_msg`格式解析。

``` C
typedef union{
    char ch[8];
    uint8_t ui8[8];
    uint16_t ui16[4];
    int16_t i16[4];
    int in[2];
    float fl[2];
    double df;
} can_msg;
````

按键布局

``` C
    6                   0
8       7           1       2
    9                   3
        5           4
        [高位][低位]
```

- `can_msg.ui8[0]`: 按键数字（0-9）
- `can_msg.ui8[1]`: 拨码状态码
- `can_msg.ui8[2]`: `button.h` 中定义`BUTTON_DOWN`或`BUTTON_UP`

## 摇杆数据

CAN ID 324

使用`can_msg.i16`读取, 范围-128~127.

``` C
/** 摇杆坐标系
    ^ y          ^ y
    |            |
x<--         x<--
*/
#define RIGHT_Y 0 // 向前推 为正
#define RIGHT_X 1 // 向左推 为正
#define LEFT_Y  2 // 向前推 为正
#define LEFT_X  3 // 向左推 为正

// 读取实例
can_msg msg;
// 略去读取can接收数据
int16_t right_x = msg.i16[RIGHT_X];
```
