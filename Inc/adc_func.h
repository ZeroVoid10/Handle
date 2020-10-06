#ifndef __adc_func_H
#define __adc_func_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"
  
#include "can.h"
#include "adc.h"
//#include "lcd.h"
#define SID_ROCKER           324  

struct adc_rocker {
  uint16_t adc_power;
  uint16_t adc_right1;
  uint16_t adc_right2;
  uint16_t adc_left1;
  uint16_t adc_left2;
};

typedef struct{
  int ly;
  int lx;
  int ry;
  int rx;
}Handle;
     
void adc_exe();
extern int adc_flag;
extern uint32_t ADC_Value[150];

extern struct adc_rocker adc_rocker_value;
extern int adc_power_off_flag;
extern Handle chassis_handle;
     
     
#ifdef __cplusplus
}
#endif
#endif /*__ adc_func_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/