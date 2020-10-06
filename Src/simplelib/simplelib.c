/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		simplelib.c
 * Author:			ZeroVoid
 * Description:		None
 * Bug:				None
 * Version:			0.2.1
 * Data:			2019/09/19 Thu 19:35
 * Todo:			None
 *******************************************************************************/

#include "simplelib.h"
#include "flags.h"

asm(".global _printf_float");

/**
 * @brief	初始化配置
 * @param	cmd_usart   指令通信usart句柄
 * @param   hcan        CAN通信句柄
 * @return	None
 */
void simplelib_init(UART_HandleTypeDef *cmd_usart, CAN_HandleTypeDef *hcan) {
    #ifdef SL_USART_DMA
    if (cmd_usart != NULL) {
        usart_DMA_init(cmd_usart);
    }
    #endif // SL_USART_DMA

    #ifdef SL_CMD
    cmd_init();
    cmd_func_init();
    uprintf("simplelib init done\r\n");
    #endif // SL_CMD
    
    #ifdef SL_CAN
    if (hcan != NULL) {
        can_init(hcan);
        can_func_init();
    }
    #endif // SL_CAN
}

void simplelib_run(void) {
    #ifdef SL_CMD
    if (DMA_RxOK_Flag) {
        usart_exc_DMA();
        DMA_RxOK_Flag = 0;
    }
    #endif // SL_CMD
    #ifdef SL_CAN
    if (can_exc_callback_flag) {
        can_exc_callback();
        can_exc_callback_flag = 0;
    }
    #ifdef SL_NRF_COMM
    // uart_nrf_send();
    #endif // SL_NRF_COMM
    #endif // SL_CAN
    if (send_wave_flag) {
        #ifdef SL_DEBUG
        send_wave(0, 10, 20, 30);
        HAL_Delay(10);
        #endif // SL_DEBUG
    }
}
