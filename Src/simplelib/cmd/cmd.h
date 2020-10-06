/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		cmd.h
 * Description:		ָ��ʵ��
 * Author:			ZeroVoid
 * Version:			0.1
 * Data:			2019/09/23 Mon 13:46
 *******************************************************************************/

#ifndef __CMD_H
#define __CMD_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "simplelib_cfg.h"

#ifdef SL_CMD
#include "main.h"
#include "usart.h"

#ifdef SL_NRF_COMM
#include "nrf_comm.h"
#include "kfifo.h"
#endif // SL_NRF_COMM

#if defined(STM32F407xx) || defined(STM32F405xx)
#include "stm32f4xx_hal.h"
#endif // STM32F407xx

#define MAX_CMD_ARG_LENGTH 16
#define MAX_CMD_INFO_LENGTH 64
#define MAX_CMD_LINE_LENGTH 128
#define MAX_ARGC 12   //
#define PRINT_BUFFER_SIZE 101

#ifdef SL_NRF_COMM
// #define DMA_BUFFER_SIZE (32-NRF_PCK_HEADER_SIZE)
#define DMA_BUFFER_SIZE 256
typedef struct {
    uint8_t buf[DMA_BUFFER_SIZE + 1];
    uint32_t len;
} uart_buf;

extern uart_buf uart_nrf_send_buf;
extern uint8_t *puart_rx_fifo;
extern __IO uint32_t uart_rx_len;
extern __IO uint32_t uart_irq_cnt;
// extern __IO uint32_t idle_cnt;
#else
#define DMA_BUFFER_SIZE 99
#endif // SL_NRF_COMM

extern UART_HandleTypeDef *pCMD_USART;
extern int DMA_RxOK_Flag;
extern char cmd_line[MAX_CMD_LINE_LENGTH + 1];
extern char *cmd_argv[MAX_ARGC];

struct cmd_info {
    void (*cmd_func)(int argc, char *argv[]);
    char *cmd_usage;
};

void usart_DMA_init();
void usart_exc_DMA();
void HAL_UART_IDLECallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

int cmd_parse(char *cmd_line,int *argc,char *argv[]);
int cmd_exec(int argc,char *argv[]);
void cmd_help_func(int argc,char *argv[]);   
void cmd_init(void);
void cmd_add(char *cmd_name, char *cmd_usage, void (*cmd_func)(int argc, char *argv[]));
void cmd_err_arg_default_handle(char *prompt);

void send_wave(float arg1, float arg2, float arg3, float arg4);
void uprintf(char *fmt, ...);
void uprintf_to(UART_HandleTypeDef *huart, char *fmt, ...);
void uart_nrf_send(void);

#endif // SL_CMD
   
#ifdef __cplusplus
}
#endif
#endif /*__CMD_H */