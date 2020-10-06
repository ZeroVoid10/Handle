/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		usart_utils.h
 * Description:		USART 封装工具
 * Author:			ZeroVoid
 * Version:			0.1
 * Data:			2019/11/17 Sun 09:47
 * Encoding:		UTF-8
 *******************************************************************************/
#ifndef __USART_UTILS_H
#define __USART_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "simplelib_cfg.h"

#ifdef SL_USART

/**
 * @brief 串口异步通信,DMA使用并配置空闲中断初始化
 * 
 * @param huart 串口句柄指针
 */
void uart_DMA_init(UART_HandleTypeDef *huart);

#endif // SL_USART

#ifdef __cplusplus
}
#endif

#endif /* __USART_UTILS_H */