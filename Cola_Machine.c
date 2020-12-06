#include "main.h"

#define koin_a 0
#define koin_m 1

#define minum HAL_GPIO_WritePin(Minum_GPIO_Port, Minum_Pin, GPIO_PIN_SET)//output on

#define msk5on HAL_GPIO_WritePin(Msk_500_GPIO_Port, Msk_500_Pin, GPIO_PIN_SET);
#define msk5of HAL_GPIO_WritePin(Msk_500_GPIO_Port, Msk_500_Pin, GPIO_PIN_RESET);

#define msk1on HAL_GPIO_WritePin(Msk_1K_GPIO_Port, Msk_1K_Pin, GPIO_PIN_SET);
#define msk1of HAL_GPIO_WritePin(Msk_1K_GPIO_Port, Msk_1K_Pin, GPIO_PIN_RESET);

#define B_500On HAL_GPIO_WritePin(Btl_500_GPIO_Port, Btl_500_Pin, GPIO_PIN_SET)//batal 500 on
#define B_500Of HAL_GPIO_WritePin(Btl_500_GPIO_Port, Btl_500_Pin, GPIO_PIN_RESET)//batal 500 off

#define B_1KOn HAL_GPIO_WritePin(Btl_1K_GPIO_Port, Btl_1K_Pin, GPIO_PIN_SET)//batal 1k on
#define B_1KOf HAL_GPIO_WritePin(Btl_1K_GPIO_Port, Btl_1K_Pin, GPIO_PIN_RESET)//batal 1k off

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void awal();
void sensor();

enum state {s0, s500, s1k, proses} cola;
int C1, C5 = 0;

uint32_t Time_Val = 0x3FFFFF;
uint32_t Time_Loop = 0;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  cola=s0;
  while (1)
  {
	  switch (cola)
	  {
	  	  case(s0):{
	  		  awal();
	  		  sensor();
	  		  if(C5==1){
	  			  cola=s500;
	  		  }
	  		  if(C1==1){
	  			  cola=s1k;
	  		  }
	  		  break;
	  	  }
	  	  case(s500):{
	  		  if(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_SET){
	  			  HAL_Delay(50);
	  			if(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_RESET){
	  				B_500On;
	  				while(++Time_Loop<=Time_Val);
	  				if(Time_Loop>Time_Val){
	  					awal();
	  					cola=s0;
	  				}
	  			}
	  		  }
	  		  sensor();
	  		  if(C5==2){
	  			  cola=s1k;
	  		  }
	  		  break;
	  	  }
	  	  case(s1k):{
	  		  if(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_SET){
	  			  HAL_Delay(50);
	  			  if(HAL_GPIO_ReadPin(Btl_GPIO_Port, Btl_Pin)==GPIO_PIN_RESET){
	  				  B_1KOn;
	  				while(++Time_Loop<=Time_Val);
	  				if(Time_Loop>Time_Val){
	  					awal();
	  					cola=s0;
	  				}
	  			  }
	  		  }
	  		  if(HAL_GPIO_ReadPin(Prs_GPIO_Port, Prs_Pin)==GPIO_PIN_SET){
	  			  HAL_Delay(50);
	  			  if(HAL_GPIO_ReadPin(Prs_GPIO_Port, Prs_Pin)==GPIO_PIN_RESET){
	  				  cola=proses;
	  			  }
	  		  }
	  		  sensor();
	  		  if((C5>2)||(C1>1)){
	  			  msk5on;
	  			  msk1on;
	  			  while(++Time_Loop<=Time_Val);
	  			  if(Time_Loop>Time_Val){
	  				  msk5of;
	  				  msk1of;
	  				  C5=2;
	  				  C1=1;
	  				  cola=s1k;
	  			  }
	  		  }
	  		  break;
	  	  }
	  	  case(proses):{
	  		  minum;
	  		  while(++Time_Loop<=Time_Val);
	  		  if(Time_Loop>Time_Val){
	  			  awal();
	  			  cola=s0;
	  		  }
	  		  break;
	  	  }
	  }
  }

}

void sensor(void){
	if (HAL_GPIO_ReadPin(C500_GPIO_Port, C500_Pin)==GPIO_PIN_SET){//baca pin koin 500
		HAL_Delay(50);//delay bouncing
		if (HAL_GPIO_ReadPin(C500_GPIO_Port, C500_Pin)==GPIO_PIN_RESET){//apakah pindah state
			++C5;//nilai koin 500 bertambah
		}
	}
	if (HAL_GPIO_ReadPin(C1000_GPIO_Port, C1000_Pin)==GPIO_PIN_SET){//baca pin koin 1000
			HAL_Delay(50);//delay bouncing
			if (HAL_GPIO_ReadPin(C1000_GPIO_Port, C1000_Pin)==GPIO_PIN_RESET){//apakah pindah state
			++C1;//nilai koin 1000 bertambah
			}
		}
}

void awal(){
	cola=s0;
	C5=0;
	C1=0;
	B_500Of;
	B_1KOf;
	HAL_GPIO_WritePin(Minum_GPIO_Port, Minum_Pin, GPIO_PIN_RESET);//minum off
	Time_Loop=0;
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
