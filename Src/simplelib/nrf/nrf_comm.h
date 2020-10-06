/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		nrf_comm.h
 * Description:		NRF Communication Functions
 * Author:			ZeroVoid
 * Version:			0.2.0
 * Data:			2019/11/08 Fri 21:04
 * Encoding:		UTF-8
 * State:           Writting
 * Laste Data:      2020/01/09 Thu
 *******************************************************************************/
#ifndef __NRF_COMM_H
#define __NRF_COMM_H
// TODO: ZeroVoid	收发队列问题

#ifdef __cplusplus
extern "C" {
#endif
#include "simplelib_cfg.h"
#ifdef SL_NRF_COMM
#include "nrf24l01.h"
#include "can_utils.h"
#include "kfifo.h"

/*******************************************************************************
 * NRF Communication Configuration
 *******************************************************************************/
/* NRF Hardware 选择-----------------------------------------------------*/
#define SL_NRF_HW_CAN                               0x10 // 启用CAN相关

/*******************************************************************************
 * NRF 低版本兼容
 *******************************************************************************/
#define NRF_SPI					                    0x4 // 为兼容V0.1.0

/*******************************************************************************
 * NRF Flash Saved Data Offset(Byte)
 *******************************************************************************/
#define NRF_TX_ADDR_OFFSET					        0 // 
#define NRF_RX_PIPE0_ADDR_OFFSET					5 // 
#define NRF_RX_PIPE1_ADDR_OFFSET					10 // 
#define NRF_RX_SET_OFFSET					        15 // 
#define NRF_RX_PIPE2_5_ADDR_OFFSET					21 // 
#define NRF_ADDR_LEN_OFFSET                         25
#define NRF_NAME_OFFSET					            26 //

/*******************************************************************************
 * NRF Protocol
 *******************************************************************************/
#define NRF_PCK_HEADER_SIZE					        2   // 数据包报头长度/Byte
#define NRF_PCK_SIZE                                (32-NRF_PCK_HEADER_SIZE)
#define NRF_CAN_SID                                 230 // ASCII NRF 加和
#define NRF_COMM_CMD_ALL_CAN                        10
#define NRF_COMM_CMD_PING                           5

#define NRF_COMM_PCK_REFER_HOST_INDEX               0
#define NRF_COMM_PCK_HOST_MSK					    0x0F
#define NRF_COMM_PCK_REFER_MSK                      0xF0
#define NRF_COMM_PCK_CNT_INDEX_INDEX                1
#define NRF_COMM_PCK_CNT_MSK                        0x0F
#define NRF_COMM_PCK_INDEX_MSK                      0xF0

#define NRF_COMM_SET_PCK_REFER_HOST(pk,refer,host)  ((pk)->pck.data_refer_host = (refer<<4)|host)
#define NRF_COMM_SET_PCK_CNT_INDEX(pk,cnt,index)    ((pk)->pck.data_cnt_index = (cnt<<4)|index)
#define NRF_COMM_GET_PCK_HOST(pck)                  ((*((uint8_t*)(pck) + NRF_COMM_PCK_REFER_HOST_INDEX)) & NRF_COMM_PCK_HOST_MSK)
#define NRF_COMM_GET_PCK_REFER(pck)                 (((*((uint8_t*)(pck) + NRF_COMM_PCK_REFER_HOST_INDEX)) & NRF_COMM_PCK_REFER_MSK)>>4)
#define NRF_COMM_GET_PCK_CNT(pck)                   ((*((uint8_t*)(pck) + NRF_COMM_PCK_CNT_INDEX_INDEX)) & NRF_COMM_PCK_CNT_MSK)
#define NRF_COMM_GET_PCK_INDEX(pck)                 (((*((uint8_t*)(pck) + NRF_COMM_PCK_CNT_INDEX_INDEX)) & NRF_COMM_PCK_INDEX_MSK)>>4)

#define NRF_COMM_INC_NRF_UART(host)                 (host&NRF_UART)
#define NRF_COMM_INC_NRF_CAN(host)				    (host&NRF_CAN)
#define NRF_COMM_INC_NRF_MCU(host)					(host&NRF_MCU) 

/**
 * @brief	NRF 处理方式
 */
typedef enum nrf_comm_way {
    NRF_UART = 0x1,
    NRF_CAN = 0x2,
    NRF_MCU = 0x4
} NRF_COMM_WAY;

/**
 * @brief	NRF main function control state
 */
typedef enum nrf_flow_control {
	NRF_IDLE,
    NRF_TX_CALLBACK,
    NRF_RX_CALLBACK,
    NRF_MAX_RT_CALLBACK,
    NRF_COMM_SEND
} NRF_COMM_STATE;

/**
 * @brief	NRF Package struct
 * @note	
 */
typedef struct _nrf_pck {
    /** refer<<4 | host */
    uint8_t data_refer_host;
    /** cnt<<4 | index */
    uint8_t data_cnt_index;
    uint8_t data[33-NRF_PCK_HEADER_SIZE];
} nrf_pck_s; // nrf_pck_struct

/**
 * @brief	NRF Package Union
 * @note	
 */
typedef union _nrf_pck_u {
    uint8_t         ui8[33];
    struct _nrf_pck pck;
} nrf_pck;

/**
 * @brief	NRF Package wrapper
 * @deprecated
 */
typedef struct _nrf_pck_wrap {
    nrf_pck pck;
    uint8_t len;
} nrf_pck_wrapper;

/**
 * @brief	NRF Package Handle
 * @note	NRF 协议发送句柄结构体
 */
typedef struct _nrf_pck_handle {
    nrf_pck     pck;
    uint8_t    data_len;
} nrf_pck_handle;

/**
 * @brief	NRF 工作状态计数器
 * @note	查看NRF当前为止的工作状态
 */
typedef struct {
    uint32_t rx_cnt;
    uint32_t tx_cnt;
    uint32_t max_rt_cnt;
} nrf_comm_counter;

/**
 * @brief	NRF协议通信状态结构体
 * @note	查看某次NRF协议通信后,其工作状态
 */
typedef struct _nrf_comm_handle {
    NRF_COMM_STATE state;
    nrf_comm_counter basic_counter;
    uint32_t tx_timeout_cnt;
    uint32_t last_tx_fail_cnt;
} nrf_comm_handle;


/*******************************************************************************
 * NRF Protocol Val
 *******************************************************************************/
extern uint8_t nrf_tx_buffer[99];
extern NRF_COMM_STATE nrf_flow_state;           // NRF 状态机控制
/*
extern nrf_pck_wrapper nrf_tx_wrapper;
extern nrf_pck_wrapper nrf_rx_wrapper;
*/
extern nrf_pck_handle nrf_tx_handle;
extern nrf_pck_handle nrf_rx_handle;
extern __IO nrf_comm_counter nrf_comm_cnt;

/*******************************************************************************
 * NRF Communication API
 *******************************************************************************/

/**
 * @brief	NRF协议通信状态机控制主函数
 * @note    在main函数的死循环中调用
 */
void nrf_main(void);

/**
 * @brief	NRF协议通信数据非阻塞式发送函数
 * @param	data:   发送数据
 * @param   len:    数据长度(字节)
 * @param   refer:  数据发送方(NRF_MCU|NRF_CAN|NRF_UART)
 * @param   host:   数据接收方处理方式(NRF_MCU|NRF_CAN|NRF_UART)
 */
void 
nrf_comm_send(uint8_t *data, uint32_t len, uint8_t refer, uint8_t host);

/**
 * @brief	NRF协议通信数据阻塞式发送函数
 * @param	data:   发送数据
 * @param   len:    数据长度(字节)
 * @param   refer:  数据发送方(NRF_MCU|NRF_CAN|NRF_UART)
 * @param   host:   数据接收方处理方式(NRF_MCU|NRF_CAN|NRF_UART)
 */
void 
nrf_comm_block_send(uint8_t *data, uint32_t len, uint8_t refer, uint8_t host);

void
nrf_comm_can_send(uint32_t sid, uint8_t* msg);

/* Weak Functions -----------------------------------------------------*/
void nrf_spi_receive_callback(uint8_t *data, int len);
void nrf_can_receive_callback(uint8_t *data, int len);
void nrf_uart_receive_callback(uint8_t *data, int len);
void nrf_receive_callback(uint8_t *data, int len);
void nrf_send_callback(uint8_t *data, int len);
void nrf_max_rt_callback(uint8_t *data, int len);


/*******************************************************************************
 * NRF Communication Private Functions
 *******************************************************************************/
void _can_rx_nrf_callback(uint32_t *id, can_msg *data);
void _nrf_idle(void);

#endif // SL_NRF_COMM
#ifdef __cplusplus
}
#endif

#endif /* __NRF_COMM_H */