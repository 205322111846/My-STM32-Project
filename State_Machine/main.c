#include "main.h"

TIM_HandleTypeDef htim3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(uint32_t Periode);
void Task_Run();
void Task_Init();

//define task.c
#define Merah 3
#define MerKun 2
#define Hijau 7
#define Kuning 1

#define M_ON HAL_GPIO_WritePin(Merah_GPIO_Port, Merah_Pin, GPIO_PIN_SET)
#define M_OFF HAL_GPIO_WritePin(Merah_GPIO_Port, Merah_Pin, GPIO_PIN_RESET)

#define K_ON HAL_GPIO_WritePin(Kuning_GPIO_Port, Kuning_Pin, GPIO_PIN_SET)
#define K_OFF HAL_GPIO_WritePin(Kuning_GPIO_Port, Kuning_Pin, GPIO_PIN_RESET)

#define H_ON HAL_GPIO_WritePin(Hijau_GPIO_Port, Hijau_Pin, GPIO_PIN_SET)
#define H_OFF HAL_GPIO_WritePin(Hijau_GPIO_Port, Hijau_Pin, GPIO_PIN_RESET)

//variabel task.c
enum state {merah, merkun, hijau, kuning} stateku;
unsigned int Time_State=0;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM3_Init(1000);
  HAL_TIM_Base_Start_IT(&htim3);

  while (1)
  {

  }

}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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

static void MX_TIM3_Init(uint32_t Periode)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 36000-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = (Periode*2)-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3)
	{
		Task_Run();
	}
}

void Task_Init(){
	stateku=merah;
}

void Task_Run(){
	switch (stateku)
	{

	case merah:
			{
				M_ON;
				K_OFF;
				H_OFF;
				if (++Time_State==Merah)
				{
					stateku=merkun;
					Time_State=0;
				}
				if (HAL_GPIO_ReadPin(Push_GPIO_Port, Push_Pin)==GPIO_PIN_SET){
						stateku=merah;
						Time_State=0;
					}
				break;
			}

			case merkun:
			{
				M_ON;
				K_ON;
				H_OFF;
				if (++Time_State==MerKun)
				{
					stateku=hijau;
					Time_State=0;
				}
				if (HAL_GPIO_ReadPin(Push_GPIO_Port, Push_Pin)==GPIO_PIN_SET){
						stateku=merah;
						Time_State=0;
					}
				break;
			}

			case hijau:
			{
				M_OFF;
				K_OFF;
				H_ON;
				if (++Time_State==Hijau)
				{
					stateku=kuning;
					Time_State=0;
				}
				if (HAL_GPIO_ReadPin(Push_GPIO_Port, Push_Pin)==GPIO_PIN_SET){
						stateku=merah;
						Time_State=0;
					}
				break;
			}

			case kuning:
			{
			M_OFF;
			K_ON;
			H_OFF;
			if (++Time_State==Kuning)
				{
				stateku=merah;
				Time_State=0;
				}
				break;
			}

	}
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Merah_Pin|Kuning_Pin|Hijau_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Push_Pin */
  GPIO_InitStruct.Pin = Push_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Push_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Merah_Pin Kuning_Pin Hijau_Pin */
  GPIO_InitStruct.Pin = Merah_Pin|Kuning_Pin|Hijau_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }

}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif
