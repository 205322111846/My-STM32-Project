#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
uint8_t C500(void);
uint8_t C1000(void);
uint8_t batal(void);
uint8_t proses(void);
void drop();
void kbl_500();
void kbl_1k();

enum state {s0, s1, s2} stateku;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  stateku=s0;
  while (1)
  {
	  switch(stateku)
	  {
	  case s0:{
		  if(C500()){
			  stateku=s1;
		  }
		  if(C1000()){
			  stateku=s2;
		  }
		  break;
	  }
	  case s1:{
		  if(C500()){
			  stateku=s2;
		  }
		  if(C1000()){
			  kbl_500();
			  stateku=s2;
		  }
		  if(batal()){
			  kbl_500();
			  stateku=s0;
		  }
		  break;
	  }
	  case s2:{
		  if(C500()){
			  kbl_500();
			  stateku=s2;
		  }
		  if(C1000()){
			  kbl_1k();
			  stateku=s2;
		  }
		  if(batal()){
			  kbl_1k();
			  stateku=s0;
		  }
		  if(proses()){
			  drop();
			  stateku=s0;
		  }
		  break;
	  }
	}

  }

}

uint8_t C500(void){
	uint8_t flag=0;
	if(HAL_GPIO_ReadPin(C500_GPIO_Port, C500_Pin)==GPIO_PIN_SET){
		HAL_Delay(50);
		if(HAL_GPIO_ReadPin(C500_GPIO_Port, C500_Pin)==GPIO_PIN_SET){
			while(HAL_GPIO_ReadPin(C500_GPIO_Port, C500_Pin)==GPIO_PIN_SET);
			flag=1;
		}
	}
	return flag;
}

uint8_t C1000(void){
	uint8_t flag=0;
	if(HAL_GPIO_ReadPin(C1000_GPIO_Port, C1000_Pin)==GPIO_PIN_SET){
		HAL_Delay(50);
		if(HAL_GPIO_ReadPin(C1000_GPIO_Port, C1000_Pin)==GPIO_PIN_SET){
			while(HAL_GPIO_ReadPin(C1000_GPIO_Port, C1000_Pin)==GPIO_PIN_SET);
			flag=1;
		}
	}
	return flag;
}

uint8_t batal(void){
	uint8_t flag=0;
	if(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_SET){
		HAL_Delay(50);
		if(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_SET){
			while(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_SET);
			flag=1;
		}
	}
	return flag;
}

uint8_t proses(void){
	uint8_t flag=0;
	if(HAL_GPIO_ReadPin(Prs_GPIO_Port, Prs_Pin)==GPIO_PIN_SET){
		HAL_Delay(50);
		if(HAL_GPIO_ReadPin(Prs_GPIO_Port, Prs_Pin)==GPIO_PIN_SET){
			while(HAL_GPIO_ReadPin(Prs_GPIO_Port, Prs_Pin)==GPIO_PIN_SET);
			flag=1;
		}
	}
	return flag;
}

void drop(){
	HAL_GPIO_WritePin(Minum_GPIO_Port, Minum_Pin, GPIO_PIN_SET);
	HAL_Delay(1500);
	HAL_GPIO_WritePin(Minum_GPIO_Port, Minum_Pin, GPIO_PIN_RESET);
}

void kbl_500(){
	HAL_GPIO_WritePin(Btl_500_GPIO_Port, Btl_500_Pin, GPIO_PIN_SET);
	HAL_Delay(1500);
	HAL_GPIO_WritePin(Btl_500_GPIO_Port, Btl_500_Pin, GPIO_PIN_RESET);
}

void kbl_1k(){
	HAL_GPIO_WritePin(Btl_1K_GPIO_Port, Btl_1K_Pin, GPIO_PIN_SET);
	HAL_Delay(1500);
	HAL_GPIO_WritePin(Btl_1K_GPIO_Port, Btl_1K_Pin, GPIO_PIN_RESET);
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

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Minum_Pin|Btl_500_Pin|Btl_1K_Pin|Msk_500_Pin
                          |Msk_1K_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : C500_Pin C1000_Pin Btl_Pin Prs_Pin */
  GPIO_InitStruct.Pin = C500_Pin|C1000_Pin|Btl_Pin|Prs_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Minum_Pin Btl_500_Pin Btl_1K_Pin Msk_500_Pin
                           Msk_1K_Pin */
  GPIO_InitStruct.Pin = Minum_Pin|Btl_500_Pin|Btl_1K_Pin|Msk_500_Pin
                          |Msk_1K_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

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

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
