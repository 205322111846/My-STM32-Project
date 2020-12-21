#include "main.h"
#include "i2c-lcd.h"

unsigned char Button1_OK(void);
unsigned char Button2_OK(void);
unsigned char Button3_OK(void);
unsigned char Button4_OK(void);

#define doorOpen    HAL_GPIO_WritePin(open_GPIO_Port,open_Pin,GPIO_PIN_SET)
#define doorClose   HAL_GPIO_WritePin(open_GPIO_Port,open_Pin,GPIO_PIN_RESET)
#define buzzerON   HAL_GPIO_WritePin(Buzz_GPIO_Port,Buzz_Pin,GPIO_PIN_SET)
#define buzzerOFF  HAL_GPIO_WritePin(Buzz_GPIO_Port,Buzz_Pin,GPIO_PIN_RESET)
#define YES 1
#define NO 0

enum state {start,startDelay, enterpass, check, open,openDelay, alarm, resetalarm, resetverify} stateku;
volatile uint8_t debounce1=0xFF;
volatile uint8_t debounce2=0xFF;
volatile uint8_t debounce3=0xFF;
volatile uint8_t debounce4=0xFF;
volatile uint8_t tombol=0;
volatile uint8_t kombinasi[4]={0,0,0,0};
volatile uint8_t pressed=NO;
volatile uint8_t count=0;
volatile uint8_t fail_count=0;
volatile uint16_t timeout;

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim4;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void MX_TIM4_Init(uint16_t Periode);

void Task_run(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM4_Init(100);
  HAL_TIM_Base_Start_IT(&htim4);
  lcd_init();
  timeout=0;
  stateku=start;
  //timeout=0;

  while (1)
  {

  }
  /* USER CODE END 3 */
}

void Task_run(void){
	switch(stateku)
		{

		case start:
		{
			buzzerOFF;
			timeout=0;
			count=0;
			pressed=NO;
			stateku=startDelay;
			lcd_clear();
			lcd_put_cur(0, 3);
			lcd_send_string("PENSecure");
			break;
		}

		case startDelay:
		{
			count=0;
			HAL_GPIO_WritePin(enter_GPIO_Port,enter_Pin,GPIO_PIN_SET);
			if (++timeout > 40) // dari scheduler sebesar 2ms*1000= 2 detik
			{
				HAL_GPIO_WritePin(enter_GPIO_Port,enter_Pin,GPIO_PIN_RESET);
				timeout=0;
				stateku=enterpass;
			}
			break;
		}

		case enterpass:
		{
			lcd_clear();
			lcd_put_cur(0, 3);
			lcd_send_string("Enter PIN");
			HAL_GPIO_WritePin(pass_GPIO_Port, pass_Pin, GPIO_PIN_SET);
			if(Button1_OK())
			{
				tombol=1;
				pressed=YES;
				timeout=0;
			}

			if (Button2_OK())
			{
				tombol=2;
				pressed=YES;
				timeout=0;
			}

			if (Button3_OK())
			{
				tombol=3;
				pressed=YES;
				timeout=0;
			}

			if (Button4_OK())
			{
				tombol=4;
				pressed=YES;
				timeout=0;
			}


			if (pressed==YES)
			{
				//geser karakter
				kombinasi[0]=kombinasi[1];
				kombinasi[1]=kombinasi[2];
				kombinasi[2]=kombinasi[3];
				kombinasi[3]=tombol;
				pressed=NO;
				tombol=0;
				count++;
				//			printf("*"); //current state
			}

			//sudah 4 karakter
			if (count>3) stateku=check;

			//jeda memasukkan password lebih dari 3 detik
			if ((++timeout>60) && (count >0))
			{
				stateku=alarm;
				timeout=0;
			}

			break;
		}

		case check:
		{
			if((kombinasi[0]==1)&&(kombinasi[1]==2)&&(kombinasi[2]==3)&&(kombinasi[3]==4))
			{
				stateku=open;
			}
			else
			{
				if (++fail_count >= 3)
				{
					stateku=alarm;
				}
				else
				{
					stateku=start;
					lcd_clear();
					lcd_put_cur(0, 3);
					lcd_send_string("Wrong PIN");
					lcd_put_cur(1, 2);
					lcd_send_string("Re-Enter PIN");
					if(++timeout>30){
						timeout=0;
						count=0;
						//stateku=start;
					}
					//break;
					//stateku=start;
				}
			}

			break;
		}

		case open:
		{
			doorOpen;
			lcd_clear();
			lcd_put_cur(0, 5);
			lcd_send_string("Access");
			lcd_put_cur(1, 4);
			lcd_send_string("Granted");
			HAL_GPIO_WritePin(pass_GPIO_Port, pass_Pin, GPIO_PIN_RESET);
			stateku=openDelay;
			break;
		}

		case openDelay:
		{
			if (++timeout > 40) // dari scheduler sebesar 2ms*500= 1 detik
			{
				doorClose;
				buzzerOFF;
				lcd_clear();
				stateku=start;
				timeout=0;
			}
			break;
		}

		case alarm:
		{
			lcd_clear();
			lcd_put_cur(0, 5);
			lcd_send_string("Access");
			lcd_put_cur(1, 5);
			lcd_send_string("Denied");
			buzzerON;
			if(++timeout > 20)
			{
				lcd_clear();
				count=0;
				pressed=NO;
				stateku=resetalarm;
			}
			break;
		}

		case resetalarm:
		{
			fail_count=0;
			lcd_clear();
			lcd_put_cur(0, 5);
			lcd_send_string("Reset");
			lcd_put_cur(1, 5);
			lcd_send_string("Alarm");
			if(Button1_OK())
			{
				tombol=1;
				pressed=YES;
				timeout=0;
			}

			if (Button2_OK())
			{
				tombol=2;
				pressed=YES;
				timeout=0;
			}

			if (Button3_OK())
			{
				tombol=3;
				pressed=YES;
				timeout=0;
			}

			if (Button4_OK())
			{
				tombol=4;
				pressed=YES;
				timeout=0;
			}

			if (pressed==YES)
			{
				//geser karakter
				kombinasi[0]=kombinasi[1];
				kombinasi[1]=kombinasi[2];
				kombinasi[2]=kombinasi[3];
				kombinasi[3]=tombol;
				pressed=NO;
				tombol=0;
				count++;
			}

			if (count>3) stateku=resetverify;

			break;
		}

		case resetverify:
		{
			if((kombinasi[0]==1)&&(kombinasi[1]==1)&&(kombinasi[2]==1)&&(kombinasi[3]==1))
			{
				lcd_clear();
				lcd_put_cur(0, 5);
				lcd_send_string("Reset");
				lcd_put_cur(1, 4);
				lcd_send_string("Granted");
				if (++timeout > 20){
					stateku=start;
				}
			}

			break;
		}
		}
}

unsigned char Button1_OK(void){
	static unsigned char debounce=0xFF;
	unsigned char detectedFLag=0;
	if(HAL_GPIO_ReadPin(A1_GPIO_Port,A1_Pin)== GPIO_PIN_RESET){
		debounce=(debounce<<1);
	} else {
		debounce= (debounce<<1)|1;
	}
	if (debounce==0x03) {
		detectedFLag=1;
	}
	return detectedFLag;
}

unsigned char Button2_OK(void){
	static unsigned char debounce=0xFF;
	unsigned char detectedFLag=0;
	if(HAL_GPIO_ReadPin(A2_GPIO_Port,A2_Pin)== GPIO_PIN_RESET){
		debounce=(debounce<<1);
	} else {
		debounce= (debounce<<1)|1;
	}
	if (debounce==0x03) {
		detectedFLag=1;
	}
	return detectedFLag;
}

unsigned char Button3_OK(void){
	static unsigned char debounce=0xFF;
	unsigned char detectedFLag=0;
	if(HAL_GPIO_ReadPin(A3_GPIO_Port,A3_Pin)== GPIO_PIN_RESET){
		debounce=(debounce<<1);
	} else {
		debounce= (debounce<<1)|1;
	}
	if (debounce==0x03) {
		detectedFLag=1;
	}
	return detectedFLag;
}

unsigned char Button4_OK(void){
	static unsigned char debounce=0xFF;
	unsigned char detectedFLag=0;
	if(HAL_GPIO_ReadPin(A4_GPIO_Port,A4_Pin)== GPIO_PIN_RESET){
		debounce=(debounce<<1);
	} else {
		debounce= (debounce<<1)|1;
	}
	if (debounce==0x03) {
		detectedFLag=1;
	}
	return detectedFLag;
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM4_Init(uint16_t Periode)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 25000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = (Periode*2)-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM4)
	{
		Task_run();
	}
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, enter_Pin|open_Pin|pass_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : A1_Pin A2_Pin A3_Pin A4_Pin */
  GPIO_InitStruct.Pin = A1_Pin|A2_Pin|A3_Pin|A4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : enter_Pin open_Pin pass_Pin */
  GPIO_InitStruct.Pin = enter_Pin|open_Pin|pass_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Buzz_Pin */
  GPIO_InitStruct.Pin = Buzz_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Buzz_GPIO_Port, &GPIO_InitStruct);

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
