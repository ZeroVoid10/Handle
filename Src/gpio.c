/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */
#include "can_utils.h"
#include "nrf_comm.h"
#include "button.h"

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
GPIO_PinState lastbutton_1 = GPIO_PIN_SET;
GPIO_PinState lastbutton_2 = GPIO_PIN_SET;
GPIO_PinState lastbutton_3 = GPIO_PIN_SET;
GPIO_PinState lastbutton_4 = GPIO_PIN_SET;
GPIO_PinState lastbutton_5 = GPIO_PIN_SET;
GPIO_PinState lastbutton_6 = GPIO_PIN_SET;
GPIO_PinState lastbutton_7 = GPIO_PIN_SET;
GPIO_PinState lastbutton_8 = GPIO_PIN_SET;
GPIO_PinState lastbutton_9 = GPIO_PIN_SET;
GPIO_PinState lastbutton_10 = GPIO_PIN_SET;

int exit_state = 0;
int dial_state = 0;
/* USER CODE END 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|NRF_CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC6 PC7 PC9 
                           PC10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9 
                          |GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC2 PC3 PC4 
                           PC5 PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4 
                          |GPIO_PIN_5|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PBPin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|NRF_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = NRF_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(NRF_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = NRF_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
uint16_t button_state = 0;
void gpio_delayed_button_ctrl(GPIO_PinState* last, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int num) {

    //uint8_t temp[10] = {0};
    if (*last == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET) {
        /*
          str[0] = '0' + dial_state;
          str[1] = '0' + num;
          str[2] = 'd';
          */
        //button_state |= 1 << num;
        // button_state |= BUTTON_DOWN << BUTTON_STATE_POS;
        //can_send_msg(325, str,3);
        *last = GPIO_PIN_SET;
        //memcpy(temp + 8, &button_state, 2);
        can_msg can_cmd = {0};
        can_cmd.ui8[0] = num;
        can_cmd.ui8[1] = button_state;
        can_cmd.ui8[2] = BUTTON_UP;
    //   switch (num) {
    //     case 0:
    //       can_cmd.ui8[0] = CMD_SHOOT;
    //       break;
    //     case 1:
    //       can_cmd.ui8[0] = CMD_SHOOT;
    //       break;
    //     case 6:
    //       can_cmd.ui8[0] = CMD_IDLE;
    //       break;
    //     case 7:
    //       can_cmd.ui8[0] = CMD_MANUAL;
    //       break;
    //     case 8:
    //       can_cmd.ui8[0] = CMD_START;
    //       break;
    //     default:
    //       can_cmd.ui8[0] = CMD_IDLE;
    //     break;
    //   }
        nrf_comm_can_send(SID_BUTTON, &can_cmd);
        button_state = 0;
    } else if (*last == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET) {
        /*
          str[0] = '0' + dial_state;
          str[1] = '0' + num;
          str[2] = 'u';
          uprintf("%s\n",str);
          */
        //button_state |= 1 << num;
        // button_state |= BUTTON_UP << BUTTON_STATE_POS;
        //can_send_msg(325, str,3);
        *last = GPIO_PIN_RESET;
        //memcpy(temp + 8, &button_state, 2);
        can_msg can_cmd = {0};
        can_cmd.ui8[0] = num;
        can_cmd.ui8[1] = button_state;
        can_cmd.ui8[2] = BUTTON_DOWN;
        nrf_comm_can_send(SID_BUTTON, &can_cmd);
        // nrf_comm_send(temp, 10);
        //NRF_Install_TX_Data(temp, 10);
        //NRF_Send_Message_IT();
        // char test_str[] = "hello, world\n";
        // nrf_send_data(temp, 10);
        // nrf_send_data((uint8_t*)test_str, 15);
        //nrf_send_data(temp, 10, false);
        button_state = 0;
    }
}

void gpio_read_dial() {
    button_state = 0;
    if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_SET) {
        button_state |= 0;
    }
        //button_state |= 1 << 15;
    //tmp += 1;
    if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_4) == GPIO_PIN_SET) {
        button_state |= 1 << 1;
    }
        //button_state |= 1 << 14;
    //tmp += 2;
    //dial_state = tmp;
}

void gpio_delayed_button() {
    gpio_read_dial();
    gpio_delayed_button_ctrl(&lastbutton_1, GPIOC, GPIO_PIN_0, 0);   
    gpio_delayed_button_ctrl(&lastbutton_2, GPIOC, GPIO_PIN_1, 1);   
    gpio_delayed_button_ctrl(&lastbutton_3, GPIOC, GPIO_PIN_2, 2);   
    gpio_delayed_button_ctrl(&lastbutton_4, GPIOC, GPIO_PIN_3, 3);  
    gpio_delayed_button_ctrl(&lastbutton_5, GPIOC, GPIO_PIN_4, 4);  
    gpio_delayed_button_ctrl(&lastbutton_6, GPIOC, GPIO_PIN_5, 5);  
    gpio_delayed_button_ctrl(&lastbutton_7, GPIOC, GPIO_PIN_6, 6);  
    gpio_delayed_button_ctrl(&lastbutton_8, GPIOC, GPIO_PIN_7, 7);  
    gpio_delayed_button_ctrl(&lastbutton_9, GPIOC, GPIO_PIN_8, 8);  
    gpio_delayed_button_ctrl(&lastbutton_10, GPIOC, GPIO_PIN_9, 9);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // NRF PIN脚 IRQ接收
    if (nrf_handle.nrf_init_ok && GPIO_Pin == GPIO_PIN_8) {
      nrf_irq_handle();
    }
}
/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
