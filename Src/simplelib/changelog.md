# SimpleLib/STM32Lib CHANGE LOG

## Version 0.2.1

- 串口相关
  - 部分解耦USART与CMD功能(均在使用IDLE 的DMA)
  - 将CMD_USART相关变成指针
- CAN相关
  - 添加CAN所有接收回调函数
- NRF
  - 修复NRF通信宽度问题
  - 修复NRF 数据字符输出越界问题
  - 修复NRF 快速发送数据, 发生覆盖问题
  - 完善NRF 通信协议部分运行代码. 实现简单的NRF 通信方式
  - NRF 将读取rx数据放到`nrf_main`中. 即简化GPIO irq中执行内容
  - cmd idle中断中添加NRF_COMM相关代码
  - 添加部分nrf_handle功能, 后期完全移植使用handle操作

### BUG or TODO

- TODO: NRF IRQ部分代码需要手动添加到HAL库GPIO回调问题
