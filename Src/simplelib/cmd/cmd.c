/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		cmd.c
 * Description:		指令功能实现
 * Author:			ZeroVoid
 * Version:			0.1
 * Data:			2019/09/19 Thu 19:50
 *******************************************************************************/
// TODO: ZeroVoid	due:10/23	模块化组件
// TODO: ZeroVoid	due:10/7	优化内存分配管理

#include "cmd.h"

#ifdef SL_CMD

#include "hash.h"
#include "flags.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 变量定义 -----------------------------------------------------*/
static const char *delim = ", \r\n\0";
static HashTable cmd_table;
static uint8_t printf_cplt_flag = 0;
#ifdef SL_NRF_COMM
uart_buf uart_buf_;
uart_buf uart_nrf_send_buf;
uart_buf* puart_buf;
uint8_t to_send_buffer[DMA_BUFFER_SIZE] = {0};
// DEFINE_KFIFO(uart_rx_fifo, uart_buf, 16);
__IO int32_t idle_cnt = 0;
__IO uint32_t uart_rx_len = 0;
__IO uint32_t uart_irq_cnt = 0;
#endif // SL_NRF_COMM

/**
 * @brief	兼容V0.1保留变量
 */
UART_HandleTypeDef* pCMD_USART;
char *cmd_argv[MAX_ARGC]; 
uint8_t DMAaRxBuffer[DMA_BUFFER_SIZE];
char DMAUSART_RX_BUF[DMA_BUFFER_SIZE];
int buffer_count = 0;
char uart_buffer[DMA_BUFFER_SIZE];

/* Private Variables -----------------------------------------------------*/
char cmd_dma_rx_buffer[10];


/* private function -----------------------------------------------------*/
static int str_cmp(const void *a, const void *b);
static void _cmd_help(const void *key, void **value, void *c1);


/* cmd实现函数定义 -----------------------------------------------------*/

void usart_DMA_init(UART_HandleTypeDef *cmd_usart) {
    pCMD_USART = cmd_usart;
    #ifdef SL_NRF_COMM
    // kfifo_get_next_buf(&uart_rx_fifo, puart_buf);
    // HAL_UART_Receive_DMA(pCMD_USART, puart_buf->buf, DMA_BUFFER_SIZE);
    HAL_UART_Receive_DMA(pCMD_USART, (uint8_t*)&DMAaRxBuffer, DMA_BUFFER_SIZE);
    #else
    HAL_UART_Receive_DMA(pCMD_USART, (uint8_t *)&DMAaRxBuffer, DMA_BUFFER_SIZE);
    #endif // NRF_COMMls
    __HAL_UART_ENABLE_IT(pCMD_USART,UART_IT_IDLE); // 开启空闲中断
}

void cmd_dma_init(UART_HandleTypeDef *huart) {
    //HAL_UART_Receive_DMA(&huart, )
}

/**
 * @brief	指令初始化函数，仅供模块初始化调用
 * @return	None
 */
void cmd_init(void) {
    if (cmd_table == NULL) {
        cmd_table = HashTable_create(str_cmp, hashStr, NULL);
    }
    #ifdef SL_NRF_COMM
    cmd_add("nrf_help", "nrf communication cmd usage", cmd_help_func);
    #else
    cmd_add("help", "Help", cmd_help_func);
    #endif // SL_NRF_COMM
}

void usart_exc_DMA() {
    int cmd_argc;
    int erro_n;
    /*
    #ifdef SL_NRF_COMM
    if (kfifo_size(&uart_rx_fifo) > 1) {
        DMA_RxOK_Flag = 1;
    }
    kfifo_peek(&uart_rx_fifo, &uart_buf_);
    memcpy(DMAUSART_RX_BUF, uart_buf_.buf, uart_buf_.len);
    #endif // SL_NRF_COMM
    */
    if (DMA_RxOK_Flag) {
        #ifdef SL_NRF_COMM
        memcpy(to_send_buffer, DMAUSART_RX_BUF, uart_rx_len);
        #endif // SL_NRF_COMM
        erro_n = cmd_parse((char *)DMAUSART_RX_BUF, &cmd_argc, cmd_argv);  //解析命令
        erro_n = cmd_exec(cmd_argc, cmd_argv);                             //执行命令
        // UNUSED(erro_n);
        #ifdef SL_NRF_COMM
        if (erro_n == 1) {
            nrf_comm_send(to_send_buffer, uart_rx_len, NRF_UART, NRF_UART);
            memset(to_send_buffer, 0, DMA_BUFFER_SIZE);
        }
        #endif // SL_NRF_COMM
        memset(DMAUSART_RX_BUF, 0, DMA_BUFFER_SIZE);
        buffer_count = 0;
    }
    DMA_RxOK_Flag = 0;
}

// FIXME: ZeroVoid	2020/01/12	 DMA接收存在未知情况时,仅接收一个字符.
void HAL_UART_IDLECallback(UART_HandleTypeDef *huart) {
    // if (huart->Instance == CMD_USART.Instance) {
    if (huart == pCMD_USART) {
        __HAL_UART_CLEAR_IDLEFLAG(huart);  //清除函数空闲标志
        // uint8_t temp;
        #ifdef STM32F407xx
        // temp = huart->Instance->ISR;
        // temp = huart->Instance->IDR;  //读出串口的数据，防止在关闭DMA期间有数据进来，造成ORE错误
        // UNUSED(temp);
        #endif // STM32F407xx
        // temp = hdma_usart3_rx.Instance->CNDTR;
        // huart->hdmarx->XferCpltCallback(huart->hdmarx);
        // //调用DMA接受完毕后的回调函数，最主要的目的是要将串口的状态设置为Ready，否则无法开启下一次DMA
        HAL_UART_DMAStop(pCMD_USART);  //停止本次DMA
        // temp = huart->Instance->RDR;

        #ifdef SL_NRF_COMM
        // nrf_handle.tx_len = strlen(DMAUSART_RX_BUF);
        // nrf_handle.tx_len++; // '\0'

        // nrf_handle.tx_len = 32 - NRF_PCK_HEADER_SIZE;
        // strncpy((char*)(nrf_tx_buffer), DMAUSART_RX_BUF, nrf_handle.tx_len);
        // nrf_handle.tx_len += NRF_PCK_HEADER_SIZE;
        // nrf_handle.nrf_data_from = NRF_UART;
        // nrf_handle.nrf_data_to = NRF_UART;

        // uart_buf_.len = strlen(DMAUSART_RX_BUF);
        // memcpy(uart_buf_.buf, DMAUSART_RX_BUF, uart_buf_.len);
        //uart_buf_.len++;
        //kfifo_put(&uart_rx_fifo, uart_buf_);
        // puart_buf->len = strlen((char*)puart_buf->buf) + 1;
        // puart_buf->len = uart_rx_len;
        /*
        uart_rx_len = 0;
        idle_cnt++;
        kfifo_get_next_buf(&uart_rx_fifo, puart_buf);
        HAL_UART_Receive_DMA(pCMD_USART, puart_buf->buf, DMA_BUFFER_SIZE);
        */
        uart_rx_len = DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(pCMD_USART->hdmarx);
        if (uart_rx_len > 0) {
            idle_cnt++;
            memcpy(DMAUSART_RX_BUF, DMAaRxBuffer, uart_rx_len);
            DMA_RxOK_Flag = 1;
        }
        memset(DMAaRxBuffer, 0, DMA_BUFFER_SIZE);
        HAL_UART_Receive_DMA(pCMD_USART, (uint8_t*)&DMAaRxBuffer, DMA_BUFFER_SIZE);
        
        #ifndef SL_CMD
        nrf_flow_state = NRF_COMM_SEND;
        #endif // SL_CMD

        #endif // SL_NRF_COMM
        
        #ifndef SL_NOCMD
        //if (DMAUSART_RX_BUF[0] != '\0') {
            // DMA_RxOK_Flag = 1;
        //}
        // memset(DMAaRxBuffer, 0, DMA_BUFFER_SIZE);
        // HAL_UART_Receive_DMA(pCMD_USART, (uint8_t *)&DMAaRxBuffer, DMA_BUFFER_SIZE);
        #endif // SL_NOCMD

        
    }
}


/**
 * @brief	将输入分割，并记录参数个数
 * @param	cmd_line	输入指令字符串
 *          argc        指令个数
 *          argv        分割后参数列表
 * @return	None
 */
int cmd_parse(char *cmd_line,int *argc,char *argv[]){
    char *token = strtok(cmd_line, delim);
    int arg_index = 0;

    while(token && arg_index <= MAX_ARGC) {
        argv[arg_index++] = token;
        token = strtok(NULL, delim);
    }
    *argc = arg_index;
    return 0;
}

/**
 * @brief	指令执行函数
 * @param	argc    参数个数
 *          argv    参数列表 DMA
 * @return	0   正常执行返回
 *          1   未找到指令
 */
int cmd_exec(int argc,char *argv[]){
    if (argc == 0) {
        return 2;
    }
    struct cmd_info *cmd = (struct cmd_info*)HashTable_get(cmd_table, argv[0]);
    if (cmd != NULL) {
        // TODO: ZeroVoid	change cmd_func to have a int return.
        cmd->cmd_func(argc, argv);
        // uprintf("[Done]\r\n");
        return 0;
    }
    #ifdef SL_NRF_COMM
    // nrf_flow_state = NRF_COMM_SEND;
    #else 
    uprintf("cmd not find\r\n");
    #endif // SL_NRF_COMM
    return 1;
}

/*
void uart_nrf_send(void) {
    //if (kfifo_is_empty(&uart_rx_fifo)) {
    if (kfifo_len(&uart_rx_fifo) <= 1) {
        //idle_cnt = 0;
        return;
    }
    kfifo_get(&uart_rx_fifo, &uart_nrf_send_buf);
    if (uart_nrf_send_buf.len == 0) {
        return;
    }
    //idle_cnt--;
    nrf_comm_send(uart_nrf_send_buf.buf, uart_nrf_send_buf.len, NRF_UART, NRF_UART);
}
*/

/**
 * @brief	指令帮助函数
 * @param	忽略参数
 * @return	None
 */
void cmd_help_func(int argc,char *argv[]){
    #ifdef SL_NRF_COMM
    uprintf("[NRF] Help:\r\n");
    #else
    uprintf("Help:\r\n");
    #endif // SL_NRF_COMM
    HashTable_map(cmd_table, _cmd_help, NULL);
}

/**
 * @brief	指令添加函数
 * @param	cmd_name    指令名称
 * @param   cmd_usage   指令使用说明
 * @param   cmd_func    指令函数指针 argc 参数个数(含指令名称), argv 参数字符串数组
 * @return	None
 */
void cmd_add(char *cmd_name, char *cmd_usage, void (*cmd_func)(int argc, char *argv[])) {
    // FIXME: ZeroVoid	2019/9/23	 name or usage too long
    struct cmd_info *new_cmd = (struct cmd_info*)malloc(sizeof(struct cmd_info)); 
    char *name = (char*) malloc(strlen(cmd_name)+1);
    char *usage = (char*) malloc(strlen(cmd_usage)+1);
    strcpy(name, cmd_name);
    strcpy(usage, cmd_usage);
    new_cmd->cmd_func = cmd_func;
    new_cmd->cmd_usage = usage;
    HashTable_insert(cmd_table, name, new_cmd);
}

/**
 * @brief	参数错误默认处理函数
 * @param   prompt  提示输出显示;可为NULL
 */
void cmd_err_arg_default_handle(char *prompt) {
    if (prompt) {
        uprintf(prompt);
    } else {
        uprintf("[ERROR] Arg! Nothing Done.\r\n");
    }
}

char print_buffer[PRINT_BUFFER_SIZE];
void uprintf(char *fmt, ...) {
    printf_cplt_flag = 0;
    int size;
    uint32_t timeout = 0;
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    size = vsnprintf(print_buffer, PRINT_BUFFER_SIZE, fmt, arg_ptr);
    va_end(arg_ptr);

    pCMD_USART->gState = HAL_UART_STATE_READY;
    // HAL_UART_Transmit_DMA(pCMD_USART, (uint8_t*)print_buffer, size + 1); // 字符串结尾/0
    // HAL_Delay(1);
    // if (HAL_UART_Transmit_DMA(&CMD_USART, (uint8_t *)print_buffer, size) != HAL_OK) {
    //     HAL_Delay(10);
    // }
    // FIXME: ZeroVoid	2019/10/17	快速输出或某些情况下会出现卡死情况
        /*
    while(pCMD_USART->hdmatx->State != HAL_DMA_STATE_READY) {
        timeout++;
        if (timeout >= 80000) {
            break;
        }
    }
        */
    HAL_UART_Transmit(pCMD_USART, (uint8_t*)print_buffer, size, 1000);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
        // 输出完成后，重新设置USART准备状态，并解锁串口,否则无法再次输出
        /* if (huart == &huart1) {
            printf_cplt_flag = 1;
            // huart->gState = HAL_UART_STATE_READY;
            CMD_USART.gState = HAL_UART_STATE_READY;
            while(CMD_USART.hdmatx->State != HAL_DMA_STATE_READY);
            if (CMD_USART.hdmatx != NULL) {
                CMD_USART.hdmatx->State = HAL_DMA_STATE_READY;
            }
        } */
}

void uprintf_to(UART_HandleTypeDef *huart, char *fmt, ...) {
    int size;
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    size = vsnprintf(print_buffer, PRINT_BUFFER_SIZE, fmt, arg_ptr);
    va_end(arg_ptr);

    // huart->gState = HAL_UART_STATE_READY;
    // HAL_UART_Transmit_DMA(huart, (uint8_t *)print_buffer, size);
    // while(huart->hdmatx->State != HAL_DMA_STATE_READY);
    HAL_UART_Transmit(huart, (uint8_t *)print_buffer, size, 1000);
}

static char s[22] = {'b', 'y', 16, 6};
void send_wave(float arg1, float arg2, float arg3, float arg4) {
    //s[2] = 16;  // length
    //s[3] = 6;   // type
    s[20] = '\r';
    s[21] = '\n';
    // s[22] = '\0';
    memcpy(s + 4, &arg1, sizeof(arg1));
    memcpy(s + 8, &arg2, sizeof(arg1));
    memcpy(s + 12, &arg3, sizeof(arg1));
    memcpy(s + 16, &arg4, sizeof(arg1));
    // printf(s);
    pCMD_USART->gState = HAL_UART_STATE_READY;
    HAL_UART_Transmit_DMA(pCMD_USART, (uint8_t *)s, 22);
    while(pCMD_USART->hdmatx->State != HAL_DMA_STATE_READY);
}

/* private function defined -----------------------------------------------------*/
/**
 * @brief	字符串比较函数，hash表中使用
 */
static int str_cmp(const void *a, const void *b) {
    return strcmp((char*)a, (char*)b) != 0;
}

/**
 * @brief	输出函数使用说明，遍历hash表中使用
 */
static void _cmd_help(const void *key, void **value, void *c1) {
    UNUSED(c1);
    char *usage = ((struct cmd_info*)(*value))->cmd_usage;
    uprintf("%s: %s\r\n", key, usage);
}

#endif // SL_CMD