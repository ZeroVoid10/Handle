/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		nrf_comm.c
 * Description:		NRF
 * Author:			ZeroVoid
 * Version:			0.2
 * Data:			2019/11/08 Fri 21:06
 *******************************************************************************/
#include "nrf_comm.h"
#ifdef SL_NRF_COMM

#ifdef SL_CMD
#include "cmd.h"
#include "can_utils.h"
#endif // SL_CMD

#include <string.h>

/*******************************************************************************
 * NRF Communication Val
 *******************************************************************************/

/* Public Val -----------------------------------------------------*/

#define NRF_TX_TIMEOUT                                  60000

uint8_t nrf_tx_buffer[99] = {0};
uint8_t nrf_tx_cnt = 0;
uint8_t nrf_tx_remainder = 0;
uint8_t nrf_rx_cnt;
uint8_t nrf_rx_index;
/*
nrf_pck_wrapper nrf_tx_wrapper = {0, 0};
nrf_pck_wrapper nrf_rx_wrapper = {0, 0};
*/

nrf_pck_handle nrf_tx_handle = {.pck.ui8 = {0}, .data_len = 0};
nrf_pck_handle nrf_rx_handle = {.pck.ui8 = {0}, .data_len = 0};
nrf_pck_handle nrf_rx_irq_handle = {.pck.ui8 = {0}, .data_len = 0};
__IO nrf_comm_counter nrf_comm_cnt = {0,0,0};
__IO nrf_comm_handle gNRF_handle = {NRF_IDLE, {0,0,0}, 0, 0};

/* Private Val -----------------------------------------------------*/
NRF_COMM_STATE nrf_flow_state = NRF_IDLE;
/*
static uint8_t cnt = 0;
static uint16_t nrf_rx_mask;
static uint16_t nrf_rx_record;
*/
static __IO uint8_t _nrf_lock = 0;
static uint8_t _nrf_fifo_send_flag = 0;
DEFINE_KFIFO(nrf_tx_fifo, nrf_pck_handle, NRF_COMM_TX_PCK_FIFO_SIZE);
DEFINE_KFIFO(nrf_rx_fifo, nrf_pck_handle, NRF_COMM_RX_PCK_FIFO_SIZE);


/*******************************************************************************
 * NRF Communication Static Private Function Declaration
 *******************************************************************************/

static void _nrf_receive_callback(uint8_t *data, int len);
static void _nrf_can_rx_callback(uint8_t *data);
static inline void nrf_lock(void);
static inline void nrf_unlock(void);
static inline void nrf_fifo_send(void);
#ifdef SL_NRF_COMM_CMD
static void nrf_comm_cmd(uint8_t* data);
#endif // SL_NRF_COMM_CMD


/*******************************************************************************
 * NRF Communication API Definition
 *******************************************************************************/

void 
nrf_comm_send(uint8_t *data, uint32_t len, uint8_t refer, uint8_t host) {
    if (len == 0 || (len>>10)) {
        return;
    }
    nrf_handle.tx_data = data;
    nrf_handle.tx_len = len;
    uint8_t nrf_tx_pck_cnt = len/(32 - NRF_PCK_HEADER_SIZE);
    uint8_t nrf_tx_last_pck_len = len%(32 - NRF_PCK_HEADER_SIZE);
    uint32_t i = 0;
    nrf_pck_handle* ppck_handle = NULL;
    for (i = 0; i < nrf_tx_pck_cnt; i++) {
        kfifo_get_next_buf(&nrf_tx_fifo, ppck_handle);
        memset(ppck_handle->pck.ui8, 0, 32);
        NRF_COMM_SET_PCK_REFER_HOST(&ppck_handle->pck, refer, host);
        NRF_COMM_SET_PCK_CNT_INDEX(&ppck_handle->pck, nrf_tx_pck_cnt, i);
        memcpy(ppck_handle->pck.pck.data, data + NRF_PCK_SIZE*i, NRF_PCK_SIZE);
        ppck_handle->data_len = 32;
    }
    kfifo_get_next_buf(&nrf_tx_fifo, ppck_handle);
    memset(ppck_handle->pck.ui8, 0, 32);
    NRF_COMM_SET_PCK_REFER_HOST(&ppck_handle->pck, refer, host);
    NRF_COMM_SET_PCK_CNT_INDEX(&ppck_handle->pck, nrf_tx_pck_cnt, i);
    memcpy(ppck_handle->pck.pck.data, data + NRF_PCK_SIZE*i, nrf_tx_last_pck_len);
    ppck_handle->data_len = nrf_tx_last_pck_len + NRF_PCK_HEADER_SIZE;

    _nrf_fifo_send_flag = 1;
    nrf_fifo_send();
}

void 
nrf_comm_block_send(uint8_t *data, uint32_t len, uint8_t refer, uint8_t host) {
    if (len == 0 || (len>>10)) {
        return;
    }
    nrf_handle.tx_data = data;
    nrf_handle.tx_len = len;
    uint32_t nrf_tx_pck_cnt = len/(32 - NRF_PCK_HEADER_SIZE);
    uint32_t nrf_tx_last_pck_len = len%(32 - NRF_PCK_HEADER_SIZE);
    uint32_t i = 0, timeout = 0;
    nrf_pck  pck;
    gNRF_handle.last_tx_fail_cnt = 0;
    for (i = 0; i < nrf_tx_pck_cnt; i++) {
        NRF_COMM_SET_PCK_REFER_HOST(&pck, refer, host);
        NRF_COMM_SET_PCK_CNT_INDEX(&pck, nrf_tx_pck_cnt, i);
        memcpy(pck.pck.data, data + NRF_PCK_SIZE*i, NRF_PCK_SIZE);
        nrf_lock();
        nrf_send_data(pck.ui8, 32);
        timeout = 0;
        while(nrf_lock) {
            timeout++;
            if(timeout >= NRF_TX_TIMEOUT) {
                // 超时取消本次发送,但不改变NRF PRX状态
                NRF_CE_DISABLE();
                _nrf_flush_tx();
                nrf_unlock();
                if (!tx_pipe0_addr_eq && nrf_handle.config->send_crc_ack) {
                    _nrf_set_rx_addr(0, nrf_rx_addr[0], nrf_handle.nrf_addr_len);
                }
                nrf_spi_delay();
                gNRF_handle.last_tx_fail_cnt++;
                break;
            }
        }
    }
    NRF_COMM_SET_PCK_REFER_HOST(&pck, refer, host);
    NRF_COMM_SET_PCK_CNT_INDEX(&pck, nrf_tx_pck_cnt, i);
    memcpy(pck.pck.data, data + NRF_PCK_SIZE*i, nrf_tx_last_pck_len);
    // _nrf_comm_send(nrf_tx_data, NRF_PCK_HEADER_SIZE + nrf_tx_remainder);
    nrf_lock();
    nrf_send_data(pck.ui8, NRF_PCK_HEADER_SIZE + nrf_tx_last_pck_len);
    timeout = 0;
    while(nrf_lock) {
        timeout++;
        if(timeout >= NRF_TX_TIMEOUT) {
            NRF_CE_DISABLE();
            _nrf_flush_tx();
            nrf_unlock();
            if (!tx_pipe0_addr_eq && nrf_handle.config->send_crc_ack) {
                _nrf_set_rx_addr(0, nrf_rx_addr[0], nrf_handle.nrf_addr_len);
            }
            nrf_spi_delay();
            _nrf_set_mode(NRF_PRX);
            nrf_spi_delay();
            NRF_CE_ENABLE();
            gNRF_handle.last_tx_fail_cnt++;
            break;
        }
    }
}

void
nrf_comm_can_send(uint32_t sid, uint8_t* msg) {
    uint8_t send_data[12] = {0};
    memcpy(send_data, &sid, 4);
    memcpy(send_data+4, msg, 8);
    nrf_comm_send(&send_data, 12, NRF_MCU, NRF_CAN);
}

// FIXME: ZeroVoid	2019/11/22	 NRF 连续发送 接收过程卡死问题
void nrf_main(void) {
    switch(nrf_flow_state) {

    case NRF_COMM_SEND:
        // _nrf_comm_send(nrf_handle.tx_data, nrf_handle.tx_len);
        // nrf_comm_send(nrf_tx_buffer, nrf_handle.tx_len, nrf_handle.nrf_data_from, nrf_handle.nrf_data_to);
        // kfifo_get(puart_rx_fifo, &uart_nrf_send_buf);
        // nrf_comm_send(uart_nrf_send_buf.buf, uart_nrf_send_buf.len, NRF_UART, NRF_UART);
        // if (kfifo_is_empty(&uart_rx_fifo)) {
        //     nrf_flow_state = NRF_IDLE;
        // }
        break;

    /* case NRF_RX_CALLBACK:
        nrf_flow_state = NRF_IDLE;
		if (nrf_read_rx_data(nrf_rx_data, &nrf_handle.rx_len, NULL) >= 0) {
            _nrf_receive_callback(nrf_handle.rx_data, nrf_handle.rx_len);
        }
        break; */
    case NRF_TX_CALLBACK:
        nrf_send_callback(nrf_handle.tx_data, nrf_handle.tx_len);
        memset(nrf_tx_data, 0, 32);
        nrf_flow_state = NRF_IDLE;
        break;
    case NRF_MAX_RT_CALLBACK:
        nrf_max_rt_callback(nrf_handle.tx_data, nrf_handle.tx_len);
        #ifdef SL_NRF_PC
        uprintf("[NRF] Max Retry\r\n");
        #endif // SL_NRF_PC
        memset(nrf_tx_data, 0, 32);
        nrf_flow_state = NRF_IDLE;
        break;
    
    default:
        break;
    }
    while(!kfifo_is_empty(&nrf_rx_fifo)) {
        kfifo_get(&nrf_rx_fifo, &nrf_rx_handle);
        _nrf_receive_callback(nrf_rx_handle.pck.ui8, nrf_rx_handle.data_len);
    }
}


__weak void nrf_spi_receive_callback(uint8_t *data, int len) {}
__weak void nrf_can_receive_callback(uint8_t *data, int len) {}
__weak void nrf_uart_receive_callback(uint8_t *data, int len) {}
__weak void nrf_receive_callback(uint8_t *data, int len) {}
__weak void nrf_send_callback(uint8_t *data, int len) {}
__weak void nrf_max_rt_callback(uint8_t *data, int len) {}

/*******************************************************************************
 * NRF Communication Private Function Definition
 *******************************************************************************/

/**
 * @brief	NRF GPIO IRQ Funcion
 * @note	Overload week nrf_irq_hanle @nrf24l01.c
 */
void nrf_irq_handle(void) {
	uint8_t status = _nrf_get_status();
	if (NRF_STATUS_GET_RX_DR(status)) {
        nrf_comm_cnt.rx_cnt++;
		if (nrf_read_rx_data(nrf_rx_irq_handle.pck.ui8, &nrf_rx_irq_handle.data_len,NULL) >= 0) {
		    rx_callback_cnt++;
            // nrf_rx_cnt = NRF_COMM_GET_PCK_CNT(nrf_rx_wrapper.pck.ui8);
            // nrf_rx_index = NRF_COMM_GET_PCK_INDEX(nrf_rx_wrapper.pck.ui8);
            kfifo_put(&nrf_rx_fifo, nrf_rx_irq_handle);
            memset(nrf_rx_irq_handle.pck.ui8, 0, 32);
            /* #ifdef NRF_COMM_EN_EVERY_RX_CALLBACK
            nrf_flow_state = NRF_RX_CALLBACK;
            #else
            if (nrf_rx_cnt == nrf_rx_index) {
                nrf_flow_state = NRF_RX_CALLBACK;
            }
            #endif // NRF_COMM_EN_EVERY_RX_CALLBACK  */
        }
	} else if (NRF_STATUS_GET_TX_DS(status)) {
		_nrf_clear_tx_irq();
		_nrf_flush_tx();
        nrf_comm_cnt.tx_cnt++;
        if (!kfifo_is_empty(&nrf_tx_fifo)) {
            nrf_fifo_send();
        } else {
            nrf_flow_state = NRF_TX_CALLBACK;
            _nrf_fifo_send_flag = 0;
            nrf_unlock();
            if (!tx_pipe0_addr_eq && nrf_handle.config->send_crc_ack) {
                _nrf_set_rx_addr(0, nrf_rx_addr[0], nrf_handle.nrf_addr_len);
            }
            NRF_CE_DISABLE();
            _nrf_set_mode(NRF_PRX);
            nrf_spi_delay();
            NRF_CE_ENABLE();
        }
	} else if (NRF_STATUS_GET_MAX_RT(status)) {
		_nrf_clear_maxrt_irq();
		_nrf_flush_tx();
        nrf_comm_cnt.max_rt_cnt++;
        if (!kfifo_is_empty(&nrf_tx_fifo)) {
            nrf_fifo_send();
        } else {
            nrf_flow_state = NRF_MAX_RT_CALLBACK;
            _nrf_fifo_send_flag = 0;
            nrf_unlock();
            NRF_CE_DISABLE();
            _nrf_set_mode(NRF_PRX);
            nrf_spi_delay();
            NRF_CE_ENABLE();
        }
	}
}

// @deprecated
/*
void _nrf_comm_send(uint8_t *data, int len) {
    data[0] = (nrf_handle.nrf_data_from << 4) | nrf_handle.nrf_data_to;
    nrf_handle.tx_data = data;
    nrf_handle.tx_len = len;
    nrf_send_data(data, len);
}
*/

/* Static Private Functions -----------------------------------------------------*/
static void _nrf_receive_callback(uint8_t *data, int len) {
    // uint8_t cnt = (data[0])
    nrf_rx_cnt = NRF_COMM_GET_PCK_CNT(data);
    nrf_rx_index = NRF_COMM_GET_PCK_INDEX(data);
    uint8_t deal_method = (data[0] & 0x0F);
    uint8_t data_from = (data[0] & 0xF0);
    data += NRF_PCK_HEADER_SIZE;
    len -= NRF_PCK_HEADER_SIZE;
    UNUSED(data_from);
    if (deal_method & NRF_UART) {
        #ifdef SL_CMD
        // uprintf("rx cnt %d\r\n", rx_callback_cnt);
        //uprintf((char*)(data));
        /*
        pCMD_USART->gState = HAL_UART_STATE_READY;
        HAL_UART_Transmit_DMA(pCMD_USART, data, len);
        while(pCMD_USART->hdmatx->State != HAL_DMA_STATE_READY);
        */
        rx_callback_cnt = 0;
        // uprintf((char*)(nrf_rx_data_buffer + NRF_PCK_HEADER_SIZE));
        // pCMD_USART->gState = HAL_UART_STATE_READY;
        // HAL_UART_Transmit_DMA(pCMD_USART, nrf_rx_data + NRF_PCK_HEADER_SIZE, len);
        // while(pCMD_USART->hdmatx->State != HAL_DMA_STATE_READY);
        HAL_UART_Transmit(pCMD_USART, data, len, 1000);
        #endif // SL_CMD
        nrf_uart_receive_callback(data, len);
    }
    if (deal_method & NRF_CAN) {
        _nrf_can_rx_callback(data);
        nrf_can_receive_callback(data, len);
    }
    if (deal_method & NRF_MCU) {
        #ifdef SL_NRF_COMM_CMD
        nrf_comm_cmd(data);
        #endif // SL_NRF_COMM_CMD
        nrf_spi_receive_callback(data, len);
    }
    #ifdef IND_LED_Pin
    HAL_GPIO_TogglePin(IND_LED_GPIO_Port, IND_LED_Pin);
    #endif // IND_LED_Pin
    nrf_receive_callback(data, len);
}

static void _nrf_can_rx_callback(uint8_t *data) {
    uint32_t sid = (uint32_t*) data;
    can_msg *msg = (can_msg*) (data+4);
    can_send_msg((uint16_t)sid, msg);
}

#ifdef SL_NRF_COMM_CMD
static void nrf_comm_cmd(uint8_t* data) {
    uint8_t arg = data[0]&0x0F;
    uint8_t cmd = data[0]>>4;
    // Long Data NRF Test
    // char str_tmp[] = "Ping OK\r\nLong Data Test:\r\n0123456789\r\nabcdefghijklmnopqrst\r\n";
    char str_tmp[] = "Ping OK\r\n";
    switch (cmd) {
    case NRF_COMM_CMD_ALL_CAN:
        nrf_all_can_send = arg;
        break;
    
    case NRF_COMM_CMD_PING:
        nrf_comm_block_send((uint8_t*)str_tmp, 11, NRF_MCU, NRF_UART);
        // HAL_GPIO_TogglePin(IND_LED_GPIO_Port, IND_LED_Pin);
        break;
    
    default:
        break;
    }
}
#endif // SL_NRF_COMM_CMD


#ifdef SL_NRF_HW_CAN
void _can_rx_nrf_callback(uint32_t *id, can_msg *data) {
    memset(nrf_tx_data, 0, 32);
    memcpy(nrf_tx_data, id, 4);
    memcpy(nrf_tx_data + 4, data, 8);
    nrf_comm_send(nrf_tx_data, 12, NRF_CAN, NRF_UART|NRF_MCU);
}
#endif // SL_NRF_HW_CAN

static inline void nrf_fifo_send(void) {
    kfifo_get(&nrf_tx_fifo, &nrf_tx_handle);
    nrf_send_data(nrf_tx_handle.pck.ui8, nrf_tx_handle.data_len);
}

static inline void nrf_lock(void) {
    _nrf_lock = 1;
}

static inline void nrf_unlock(void) {
    _nrf_lock = 0;
}


#endif // SL_NRF_COMM