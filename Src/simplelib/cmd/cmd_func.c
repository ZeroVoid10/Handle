#include "cmd_func.h"
#include "can_utils.h"
#include "can_func.h"
#include "flags.h"
#include <stdlib.h>

#ifdef SL_NRF_COMM
#ifdef SL_USART

#include "nrf24l01.h"
#include "stmflash.h"
#include <string.h>
void cmd_nrf_set_tx_addr(int argc, char *argv[]) {
    if (argc < 4 || argc > 6) {
        cmd_err_arg_default_handle(NULL);
        return ;
    }
    uint8_t addr[argc-1];
    uprintf("[NRF]\r\nset nrf addr width %d\r\nset addr: ", argc - 1);
    nrf_set_addr_width(argc - 1);
    memcpy(((uint8_t*)flash_data)+NRF_ADDR_LEN_OFFSET, &nrf_handle.nrf_addr_len, 1);
    for (int i = 1; i < argc; i++) {
        addr[i-1] = (uint8_t) atoi(argv[i]);
        uprintf("%3d ", addr[i-1]);
    }
    uprintf("\r\n");
    nrf_set_tx_addr(addr, argc-1);
    memcpy(((uint8_t*)flash_data) + NRF_ADDR_LEN_OFFSET, &nrf_handle.nrf_addr_len, 1);
    memcpy(flash_data, nrf_tx_addr, 5);
    write_prams();
} 

void cmd_nrf_get_tx_addr(int argc, char *argv[]) {
    uint8_t *addr = NULL, len;
    nrf_get_tx_addr(&addr, &len);
    uprintf("[NRF]\r\ntx\r\naddr len: %d\r\naddr:", len);
    for (int i = 0; i<len; i++) {
        uprintf("%3d ", addr[i]);
    }
    uprintf("\r\n");
}

void cmd_nrf_set_rx_addr(int argc, char *argv[]) {
    if (argc < 2 || argc > 7) {
        cmd_err_arg_default_handle(NULL);
        return ;
    }
    NRF_PIPE pipe = atoi(argv[1]);
    uint8_t addr[argc - 2];
    for (int i = 2; i < argc; i++) {
        addr[i-2] = (uint8_t) atoi(argv[i]);
    }
    if (pipe <= 1 && argc >= 5) {
        nrf_set_rx_addr(pipe, addr, argc - 2);
        nrf_set_addr_width(argc - 2);
        memcpy(((uint8_t*)flash_data) + 5*(pipe+1), nrf_rx_addr[pipe], 5);
    } else if (pipe <= 5) {
        nrf_set_rx_addr(pipe, addr, argc - 2);
        memcpy(((uint8_t*)flash_data) + 19 + pipe, nrf_rx_addr[pipe], 1);
    }
    memcpy(((uint8_t*)flash_data) + NRF_ADDR_LEN_OFFSET, &nrf_handle.nrf_addr_len, 1);
    memcpy(((uint8_t*)flash_data) + 15, nrf_rx_addr_set, 6);
    write_prams();
}

void cmd_nrf_get_rx_addr(int argc, char *argv[]) {
    if (argc == 2) {
        uint8_t *addr = NULL, len;
        NRF_PIPE pipe = atoi(argv[1]);
        if (pipe > 6) {
            cmd_err_arg_default_handle(NULL);
            return;
        }
        nrf_get_rx_addr(pipe, &addr, &len);
        uprintf("[NRF]\r\nrx pipe %d\r\naddr len: %d\r\naddr:", pipe, len);
        for (int i = 0; i<len; i++) {
            uprintf("%3d ", addr[i]);
        }
        uprintf("%s\r\n", nrf_rx_addr_set[pipe]? "EN":"");
    } else {
        cmd_err_arg_default_handle(NULL);
    }
}

void cmd_nrf_enable_rx_pipe(int argc, char *argv[]) {
    if (argc < 2) {
        cmd_err_arg_default_handle(NULL);
    }
    int pipe;
    for (int i = 1; i<argc; i++) {
        pipe = atoi(argv[i]);
        if (pipe >= 0 && pipe < 6) {
            nrf_rx_addr_set[pipe] = true;
            _nrf_enable_pipe_address(0x01<<pipe);
        }
    }
    memcpy(((uint8_t*)flash_data) + 15, nrf_rx_addr_set, 6);
    write_prams();
}

void cmd_nrf_disable_rx_pipe(int argc, char *argv[]) {
    if (argc < 2) {
        cmd_err_arg_default_handle(NULL);
    }
    int pipe;
    for (int i = 1; i<argc; i++) {
        pipe = atoi(argv[i]);
        if (pipe >= 0 && pipe < 6) {
            nrf_rx_addr_set[pipe] = false;
            _nrf_disable_pipe_address(0x01<<pipe);
        }
    }
    memcpy(((uint8_t*)flash_data) + 15, nrf_rx_addr_set, 6);
    write_prams();
}

void cmd_nrf_get_state(int argc, char *argv[]) {
    int i = 0, j = 0, len = nrf_handle.nrf_addr_len;
    uprintf("[NRF] state\r\nChannel %dM\r\nAddr Width %d\r\n", _nrf_get_frequency(),len);
    uprintf("Recived CAN SID: %s\r\n", (nrf_all_can_send)? "ALL":"230");
    uprintf("TX Addr ");
    for (i = 0; i<len; i++) {
        uprintf("%3d ", nrf_tx_addr[i]);
    }
    uprintf("\r\nRx Addr & State\r\n");
    for (i = 0; i<6; i++) {
        uprintf("  PIPE %d: ", i);
        for (j = 0; j<len; j++) {
            uprintf("%3d ", nrf_rx_addr[i][j]);
        }
        uprintf("%s\r\n", nrf_rx_addr_set[i]? "EN":"");
    }
}

void cmd_nrf_get_all_can(int argc, char *argv[]) {
    if (argc == 2) {
        nrf_all_can_send = (uint8_t)atoi(argv[1]);
        uint8_t msg = ((NRF_COMM_CMD_ALL_CAN<<4)|nrf_all_can_send);
        nrf_comm_send(&msg, 1, NRF_UART, NRF_MCU);
    }
}

void cmd_nrf_ping(int argc, char *argv[]) {
    // nrf_tx_data[NRF_PCK_HEADER_SIZE] = (NRF_COMM_CMD_PING<<4);
    /* if (argc == 2) {
        nrf_tx_data[NRF_PCK_HEADER_SIZE] = 
                    (NRF_COMM_CMD_PING<<4)|((uint8_t)atoi(argv[1]));
    } */
    // nrf_handle.nrf_data_from = NRF_UART;
    // nrf_handle.nrf_data_to = NRF_SPI;
    // _nrf_comm_send(nrf_handle.tx_data, 3);
    uint8_t msg = (NRF_COMM_CMD_PING<<4);
    // nrf_comm_block_send(&msg, 1, NRF_UART, NRF_MCU); // small data work well
    nrf_comm_send(&msg, 1, NRF_UART, NRF_MCU);
}
#endif // SL_USART
#endif // SL_NRF_COMM

#ifdef SL_MOTOR_DRIVER
#include "mtr_driver.h"
#ifdef EN_VESC_MOTOR_DRIVER
void cmd_vesc_set_duty(int argc, char *argv[]);
void cmd_vesc_set_rpm(int argc, char *argv[]);
void cmd_vesc_set_pos(int argc, char *argv[]);
void cmd_vesc_set_handbrake(int argc, char *argv[]);
void cmd_vesc_set_current(int argc, char *argv[]);
void cmd_vesc_set_current_brake(int argc, char *argv[]);

void cmd_vesc_set_duty(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        float arg = atof(argv[2]);
        uprintf("id %d set duty %f\r\n", id, arg);
        vesc_set_duty(id, arg);
    } else {
        uprintf("Error Args");
    }
}

void cmd_vesc_set_rpm(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        float arg = atof(argv[2]);
        uprintf("id %d set rpm %f\r\n", id, arg);
        vesc_set_rpm(id, arg);
    } else {
        uprintf("Error Args");
    }
}

void cmd_vesc_set_pos(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        float arg = atof(argv[2]);
        uprintf("id %d set pos %f\r\n", id ,arg);
        vesc_set_pos(id, arg);
    } else {
        uprintf("Error Args");
    }
}

void cmd_vesc_set_handbrake(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        float arg = atof(argv[2]);
        vesc_set_handbrake(id, arg);
    } else {
        uprintf("Error Args");
    }
}

void cmd_vesc_set_current(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        float arg = atof(argv[2]);
        vesc_set_current(id, arg);
    } else {
        uprintf("Error Args");
    }
}

void cmd_vesc_set_current_brake(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        float arg = atof(argv[2]);
        vesc_set_current_brake(id, arg);
    } else {
        uprintf("Error Args");
    }
}
#endif // EN_VESC_MOTOR_DRIVER

#ifdef EN_MOTOR_DRIVER
void cmd_md_set_duty(int argc, char *argv[]);
void cmd_md_set_speed(int argc, char *argv[]);
void cmd_md_set_position(int argc, char *argv[]);

void cmd_md_set_duty(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        int arg = atoi(argv[2]);
        md_set_duty(id, arg);
        uprintf("id %d set duty %d\r\n", id, arg);
    } else {
        uprintf("Error Args\r\n");
    }
}

void cmd_md_set_speed(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        int arg = atoi(argv[2]);
        md_set_speed(id, arg);
        uprintf("id %d speed duty %d\r\n", id, arg);
    } else {
        uprintf("Error Args\r\n");
    }
}

void cmd_md_set_position(int argc, char *argv[]) {
    if (argc == 3) {
        uint8_t id = (uint8_t) atoi(argv[1]);
        int arg = atoi(argv[2]);
        md_set_position(id, arg);
        uprintf("id %d position duty %d\r\n", id, arg);
    } else {
        uprintf("Error Args\r\n");
    }
}

#endif // EN_MOTOR_DRIVER
#endif // SL_MOTOR_DRIVER

void cmd_hello_func(int argc, char *argv[]) {
    #ifdef SL_CMD
    uprintf("hello world\r\n");
    #endif // SL_CMD
}
void cmd_wave_test(int argc, char *argv[]) {
    send_wave_flag ^= 1;
    uprintf(send_wave_flag? "Wave Start\r\n":"Wave Stop\r\n");
}

#ifdef SL_DEBUG

void cmd_can_test(int argc, char *argv[]) {
    uprintf("can send test\r\n");
    can_send_test();
}

void cmd_show_rocker(int argc, char *argv[]) {
    uprintf("show rocker\r\n");
    can_data_show_flag = 1;
}

void cmd_stop_rocker(int argc, char *argv[]) {
    can_data_show_flag = 0;
    uprintf("sotp show rocker\r\n");
}

#endif // SL_DEBUG

void cmd_long(int argc, char *argv[]) {
    uprintf("0123456789012345678901234567890\r\n");
}

void cmd_func_init(void) {

    #ifdef SL_NRF_COMM
    #ifdef SL_USART
    cmd_add("nrf_set_tx", "set tx addr", cmd_nrf_set_tx_addr);
    cmd_add("nrf_get_tx", "get tx addr", cmd_nrf_get_tx_addr);
    cmd_add("nrf_set_rx", "set rx addr", cmd_nrf_set_rx_addr);
    cmd_add("nrf_get_rx", "get rx addr", cmd_nrf_get_rx_addr);
    cmd_add("nrf_rx_pipe_en", "enable nrf rx pipes", cmd_nrf_enable_rx_pipe);
    cmd_add("nrf_rx_pipe_dis", "disblae nrf rx pipes", cmd_nrf_disable_rx_pipe);
    cmd_add("nrf_state", "get nrf state", cmd_nrf_get_state);
    cmd_add("nrf_set_rcan", "set remote can send", cmd_nrf_get_all_can);
    cmd_add("nrf_ping", "ping test", cmd_nrf_ping);
    #else
    cmd_add("hello", "just", cmd_hello_func);
    cmd_add("wave", "", cmd_wave_test);
    cmd_add("test", "", cmd_long);
    #endif 
    #endif // SL_NRF_COMM

    #ifdef SL_MOTOR_DRIVER
    #ifdef EN_MOTOR_DRIVER
    cmd_add("m_duty", "", cmd_md_set_duty);
    cmd_add("m_speed", "", cmd_md_set_speed);
    cmd_add("m_pos", "", cmd_md_set_position);
    #endif // EN_MOTOR_DRIVER
    
    #ifdef EN_VESC_MOTOR_DRIVER
    cmd_add("v_duty", "vesc set duty", cmd_vesc_set_duty);
    cmd_add("v_rpm", "vesc set rpm", cmd_vesc_set_rpm);
    cmd_add("v_pos", "vesc set pos", cmd_vesc_set_pos);
    #endif // EN_VESC_MOTOR_DRIVER
    #endif // SL_MOTOR_DRIVER

    #ifdef SL_DEBUG
    cmd_add("can_test", "test can", cmd_can_test);
    cmd_add("wave", "", cmd_wave_test);
    cmd_add("rocker", "rocker", cmd_show_rocker);
    cmd_add("can_stop", "s", cmd_stop_rocker);
    #endif
}