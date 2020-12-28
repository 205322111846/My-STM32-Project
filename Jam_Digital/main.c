#include "main.h"
#include "lcd_16x2.h"
#include "fonts.h"
#include "test.h"
#include "ssd1306.h"
#include "stdio.h"

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;

#define click 1
#define hold 2
char time[20];
char times[10];
int hours = 0;
int minutes = 0;
int seconds = 57;
int miliseconds = 0;
int Ahours = 0;
int Aminutes = 1;
int light = 0;
int alarm = 0;
int count = 0;
enum state{start, display, changeHour, changeMinute, changeAlarmHour, changeAlarmMinute} mystate;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);

uint8_t modeDetect(void);
uint8_t advanceDetect(void);
uint8_t alarmDetect(void);
uint8_t lightDetect(void);
void TIM4_IRQHandler(void);
void getTime(uint8_t mode);
void displayTime(uint8_t mode);
void updateBaseTime(void);
void updateTime(uint8_t mode, uint8_t flag);
void updateAlarmTime(uint8_t mode, uint8_t flag);
void displayState(uint8_t state);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  lcd_init();
  ssd1306_I2C_Init();
  lcd_clear();
  SSD1306_Clear();
  while (1)
  {
	  if(lightDetect())light=1;
	  	  if(alarmDetect()){
	  		  if(alarm == 0)alarm=1;
	  		  else alarm=0;
	  		  HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_RESET);
	  	  }
	  	  if (hours==Ahours && minutes==Aminutes && seconds == 0 && alarm == 1){
	  		  HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_SET);
	  	  }
	  	  if(alarm==1){
	  		  lcd_put_cur(1, 15);
	  		  lcd_send_string("!");
	  	  }
	  	  else
	  	  {
	  		  lcd_put_cur(1, 15);
	  		  lcd_send_string(" ");
	  	  }
	  	  if(light){
	  		  count++;
	  		  HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_SET);
	  		  if(count == 100){
	  			  HAL_GPIO_WritePin(Led_GPIO_Port, Led_Pin, GPIO_PIN_RESET);
	  			  light=0;
	  			  count = 0;
	  		  }
	  	  }
	  	  switch (mystate) {
	  		case start:
	  			lcd_put_cur(0, 1);
	  			lcd_send_string("Digital Clock");
	  			lcd_put_cur(1, 4);
	  			lcd_send_string("Starting");
	  			HAL_Delay(2000);
	  			lcd_clear();
	  			HAL_TIM_Base_Start_IT(&htim2);
	  			mystate = display;
	  			break;
	  		case display:
	  			getTime(1);
	  			displayTime(1);
	  			displayState(1);
	  			if(modeDetect()) mystate = changeHour;
	  			break;
	  		case changeHour:
	  			updateTime(1, advanceDetect());
	  			getTime(1);
	  			displayTime(1);
	  			displayState(2);
	  			if(modeDetect()) mystate = changeMinute;
	  			break;
	  		case changeMinute:
	  			updateTime(2, advanceDetect());
	  			getTime(1);
	  			displayTime(1);
	  			displayState(3);
	  			if(modeDetect()) mystate = changeAlarmHour;
	  			break;
	  		case changeAlarmHour:
	  			updateAlarmTime(1, advanceDetect());
	  			getTime(2);
	  			displayTime(2);
	  			displayState(4);
	  			if(modeDetect()) mystate = changeAlarmMinute;
	  			break;
	  		case changeAlarmMinute:
	  			updateAlarmTime(2, advanceDetect());
	  			getTime(2);
	  			displayTime(2);
	  			displayState(5);
	  			if(modeDetect()) mystate = display;
	  			break;
	  		default:
	  			break;
	  	}
  }
  /* USER CODE END 3 */
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 36000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

void TIM2_IRQHandler(void)
{
	updateBaseTime();
	HAL_GPIO_TogglePin(InLed_GPIO_Port, InLed_Pin);
	HAL_TIM_IRQHandler(&htim2);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(InLed_GPIO_Port, InLed_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Led_Pin|Buzz_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : InLed_Pin */
  GPIO_InitStruct.Pin = InLed_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(InLed_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Light_Pin Alarm_Pin Mode_Pin Advanced_Pin */
  GPIO_InitStruct.Pin = Light_Pin|Alarm_Pin|Mode_Pin|Advanced_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Led_Pin Buzz_Pin */
  GPIO_InitStruct.Pin = Led_Pin|Buzz_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

uint8_t modeDetect(void){
	int flag=0;
	unsigned int Timeout_loop=0;
	unsigned int Timeout_value=900000;
  if(HAL_GPIO_ReadPin(Mode_GPIO_Port,Mode_Pin)==GPIO_PIN_SET) {
	  HAL_Delay(20);
	if(HAL_GPIO_ReadPin(Mode_GPIO_Port,Mode_Pin)) {
		while(HAL_GPIO_ReadPin(Mode_GPIO_Port,Mode_Pin) && Timeout_loop++<=Timeout_value)
			if (Timeout_loop>=Timeout_value)
			{
			flag = hold;
			}
			else
			{
			flag = click;
			}
		}
	}
  return flag;
}
uint8_t advanceDetect(void){
	int flag=0;
	unsigned int Timeout_loop=0;
	unsigned int Timeout_value=200000;
  if(HAL_GPIO_ReadPin(Advanced_GPIO_Port,Advanced_Pin)==GPIO_PIN_SET) {
	  HAL_Delay(20);
	if(HAL_GPIO_ReadPin(Advanced_GPIO_Port,Advanced_Pin)) {
		while(HAL_GPIO_ReadPin(Advanced_GPIO_Port,Advanced_Pin) && Timeout_loop++<=Timeout_value)
			if (Timeout_loop>=Timeout_value)
			{
			flag = hold;
			}
			else
			{
			flag = click;
			}
		}
	}
  return flag;
}
uint8_t alarmDetect(void){
	int flag=0;
  if(HAL_GPIO_ReadPin(Alarm_GPIO_Port,Alarm_Pin)==GPIO_PIN_SET) {
	  HAL_Delay(20);
	if(HAL_GPIO_ReadPin(Alarm_GPIO_Port,Alarm_Pin)) {
		while(HAL_GPIO_ReadPin(Alarm_GPIO_Port,Alarm_Pin));
		flag = click;
		}
	}
  return flag;
}
uint8_t lightDetect(void){
	int flag=0;
  if(HAL_GPIO_ReadPin(Light_GPIO_Port,Light_Pin)==GPIO_PIN_SET) {
	  HAL_Delay(20);
	if(HAL_GPIO_ReadPin(Light_GPIO_Port,Light_Pin)) {
		while(HAL_GPIO_ReadPin(Light_GPIO_Port,Light_Pin));
		flag = click;
		}
	}
  return flag;
}
void getTime(uint8_t mode){
	if(mode == 1){
		sprintf((char*)time,"%02d:%02d:%02d",hours,minutes,seconds);

	}
	if(mode == 2){
		sprintf((char*)time,"%02d:%02d",Ahours,Aminutes);


	}
}
void displayTime(uint8_t mode){
	if (mode == 1) {
		lcd_put_cur(0, 0);
		lcd_send_string("                ");
		lcd_put_cur(0, 4);
		lcd_send_string(time);

	}
	if (mode == 2) {
		lcd_put_cur(0, 0);
		lcd_send_string("                ");
		lcd_put_cur(0, 5);
		lcd_send_string(time);

	}
}
void updateBaseTime(void){
	if(++seconds == 60){
		minutes++;
		seconds = 0;
	}
	if (minutes  == 60) {
		hours++;
		minutes = 0;
	}
	if (hours==24) {
		hours =0;
	}
}
void updateTime(uint8_t mode, uint8_t flag){
	if(mode == 1){
		if(flag == 1){
			hours++;
		}
		if(flag == 2){
			while(HAL_GPIO_ReadPin(Advanced_GPIO_Port, Advanced_Pin) == 1){
				hours++;
				if(hours == 24)hours = 0;
				getTime(1);
				displayTime(1);
				HAL_Delay(350);
			}
		}
	}
	if(mode == 2){
		if(flag == 1){
			minutes++;
		}
		if(flag == 2){
			while(HAL_GPIO_ReadPin(Advanced_GPIO_Port, Advanced_Pin) == 1){
				minutes++;
				if(minutes == 60)minutes=0;
				getTime(1);
				displayTime(1);
				HAL_Delay(200);
			}
		}
	}
}
void updateAlarmTime(uint8_t mode, uint8_t flag){
	if(mode == 1){
		if(flag == 1){
			Ahours++;
			if(Ahours == 24)Ahours = 0;
		}
		if(flag == 2){
			while(HAL_GPIO_ReadPin(Advanced_GPIO_Port, Advanced_Pin) == 1){
				Ahours++;
				if(Ahours == 24)Ahours = 0;
				getTime(2);
				displayTime(2);
				HAL_Delay(350);
			}
		}
	}
	if(mode == 2){
		if(flag == 1){
			Aminutes++;
			if(Aminutes == 60)Aminutes = 0;
		}
		if(flag == 2){
			while(HAL_GPIO_ReadPin(Advanced_GPIO_Port, Advanced_Pin) == 1){
				Aminutes++;
				if(Aminutes == 60)Aminutes=0;
				getTime(2);
				displayTime(2);
				HAL_Delay(200);
			}
		}
	}
}
void displayState(uint8_t state){
	if(state == 1){
		lcd_put_cur(1, 0);
		lcd_send_string("                ");
		lcd_put_cur(1, 2);
		lcd_send_string("Current Time");

	}
	if(state == 2){
		lcd_put_cur(1, 0);
		lcd_send_string("                ");
		lcd_put_cur(1, 2);
		lcd_send_string("Change  Hour");

	}
	if(state == 3){
		lcd_put_cur(1, 0);
		lcd_send_string("                ");
		lcd_put_cur(1, 1);
		lcd_send_string("Change Minutes");

	}
	if(state == 4){
		lcd_put_cur(1, 0);
		lcd_send_string("                ");
		lcd_put_cur(1, 1);
		lcd_send_string("Set Alarm Hour");

	}
	if(state == 5){
		lcd_put_cur(1, 0);
		lcd_send_string("                ");
		lcd_put_cur(1, 0);
		lcd_send_string("Set Alarm Minute");

	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
