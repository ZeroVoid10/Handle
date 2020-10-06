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

```C
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

使用`msg_can.i16`读取, 范围-128~127.
