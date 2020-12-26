#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define User_LED_Pin GPIO_PIN_13
#define User_LED_GPIO_Port GPIOC
#define S1_Pin GPIO_PIN_12
#define S1_GPIO_Port GPIOB
#define S2_Pin GPIO_PIN_13
#define S2_GPIO_Port GPIOB
#define S3_Pin GPIO_PIN_14
#define S3_GPIO_Port GPIOB
#define Start_Pin GPIO_PIN_15
#define Start_GPIO_Port GPIOB
#define Rese_Pin GPIO_PIN_8
#define Rese_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
