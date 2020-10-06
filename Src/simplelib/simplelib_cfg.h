/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		simplelib_config.h
 * Description:		SimpleLib: 兼容STM32CubeMX生成项目的组件Lib
 * Version:			0.1.1
 * Author:			ZeroVoid
 * Data:			2019/09/23 Mon 14:30
 * Encoding:        UTF-8
 *******************************************************************************/
#ifndef __SIMPLELIB_CONFIG_H
#define __SIMPLELIB_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sl_def.h"

/*******************************************************************************
 * SimpleLib Configuration
 * @note	SL Simplelib or Stm32Lib
 * @note    VSCode comment shortcut: ctrl + /
 *******************************************************************************/
// #define SL_OS                           // OS系统模块
#define SL_CAN                          // CAN通信模块
// #define SL_USART					    // USART 模块
#define SL_CMD                          // 串口通信模块
#define SL_CMD_DMA                          // 串口通信DMA使用
// #define SL_FLASH                        // Flash 模块
#define SL_NRF                          // NRF通信模块
#define SL_NRF_COMM                     // NRF 通信通信协议
#define SL_NRF_COMM_CMD
// #define SL_NRF_PC
// #define SL_MOTOR_DRIVER					// 驱动卡模块
#define SL_UTILS					    // 通用工具函封装
// #define SL_IMI_SPI                      // 软件SPI封装
// #define SL_AS5047P                      // AS5047P 


// #define SL_DEBUG                        // Simplelib debug macro
#define SL_NRF_DEBUG					 // 
#define SL_COMP_V01					    // 兼容V0.1版本
// #define SL_NOCMD

/* CMD Configuration -----------------------------------------------------*/
#ifdef SL_CMD
#ifndef SL_USART
// #define SL_USART
#endif // n SL_USART
#endif // SL_CMD

/* USART Configuration -----------------------------------------------------*/
#ifdef SL_USART
#define SL_USART_DMA                    // 串口DMA
#endif // SL_USART

/* Motor Driver Configuration -----------------------------------------------------*/
#ifdef SL_MOTOR_DRIVER
#define EN_MOTOR_DRIVER                 // 使能HX 驱动卡
#define EN_VESC_MOTOR_DRIVER             // 使能Benjamin VESC驱动卡
#endif // SL_MOTOR_DRIVER

/* Command Configuration -----------------------------------------------------*/
#ifdef SL_CMD_DMA
#define CMD_MAX_LENGTH					100 // 指令最大长度
#endif // SL_CMD_DMA

/* Flash Configuration -----------------------------------------------------*/
#ifdef SL_FLASH
#define FLASH_SIZE                      25 // Flash float数组大小
#endif // SL_FLASH

/* NRF Configuration -----------------------------------------------------*/
#ifdef SL_NRF
#include "gpio.h"
#include "spi.h"
// FIXME: ZeroVoid	2019/11/17	 spi DMA NRF not work
// #define SL_NRF_DMA                      // 使用DMA通信
#define NRF_COMM_TX_PCK_FIFO_SIZE		16
#define NRF_COMM_RX_PCK_FIFO_SIZE		16
// #define NRF_COMM_EN_EVERY_RX_CALLBACK
// #define SL_NRF_PC

#ifdef STM32F072xB
#define NRF_ADDR_COF					3 // 接收地址变化参数,方便测试
#define NRF_SPI_Handle                  hspi1
#define NRF_SPI_CSN_PIN                 NRF_CS_Pin
#define NRF_SPI_CSN_GPIO_PORT           NRF_CS_GPIO_Port
#define NRF_SPI_IRQ_PIN                 NRF_IRQ_Pin
#define NRF_SPI_IRQ_GPIO_PORT           NRF_IRQ_GPIO_Port
#define NRF_SPI_CE_PIN                  NRF_CE_Pin
#define NRF_SPI_CE_GPIO_PORT            NRF_CE_GPIO_Port
#endif // STM32F072xB

#ifdef STM32F407xx 
#define NRF_ADDR_COF					1 // 接收地址变化参数,方便测试
#define NRF_SPI_Handle                  hspi3
#define NRF_SPI_CSN_PIN                 GPIO_PIN_15
#define NRF_SPI_CSN_GPIO_PORT           GPIOA
#define NRF_SPI_IRQ_PIN                 GPIO_PIN_8
#define NRF_SPI_IRQ_GPIO_PORT           GPIOB
#define NRF_SPI_CE_PIN                  GPIO_PIN_9
#define NRF_SPI_CE_GPIO_PORT            GPIOB
#endif // STM32F407xx

#endif // SL_NRF

/* Other Configuration -----------------------------------------------------*/
#define SL_LOCKED_TIMEOUT               3

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLELIB_CONFIG_H */