#include "main.h"

#define Kembalian HAL_GPIO_WritePin(Chg_GPIO_Port, Chg_Pin, GPIO_PIN_SET)
#define Permen HAL_GPIO_WritePin(Out_GPIO_Port, Out_Pin, GPIO_PIN_SET)

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void sensor(void);
void start();

uint32_t Time_Val=0x3FFFFF;
uint32_t Time_Loop=0;

enum state {s0, s1, s2, s3} stateku; //s0 state awal, s1 koin 1, s2 koin 2, s3 barang keluar dan kembalian
int C5, C1 = 0;//nilai awal koin

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  stateku=s0;
  while (1)
  {
	  switch (stateku)
	  {
	  	  case s0:
	  	  {
	  		  sensor();//membaca sensor, apakah ada uang masuk
	  		  if(C5==3){//apakah uang yang masuk koin 500
	  			  stateku = s1;//lanjut ke state 1
	  		  }
	  		  else if(C1==3){//atau apakah uang yang masuk koin 1000
	  			  stateku = s1;//lanjut ke state 1
	  		  }
	  		  break;
	  	  }

	  	case s1:
	  	{
	  		sensor();//membaca sensor, apakah ada uang yang masuk
	  		if(C5==6){//apakah uang yang masuk koin 500
	  		  stateku = s2;//lanjut ke state 2
	  		}
	  		else if(C5==3&&C1==3){//apakah koin yang masuk 1000 dan sebelumnya koin 500
	  			stateku = s3;//lanjut state 3
	  		}
	  		else if (C1==6){//apakah yang masuk koin 1000
	  			 stateku = s3;//lanjut state 3
	  		}
	  		break;
	  }

	  	case s2:
	  	{
	  		sensor();//membaca sensor, apakah ada uang masuk
	  		if(C5==9){//apakah uang yang masuk koin 500
	  			stateku = s3;//lanjut state 3
	  		}
	  		break;
	  	}

	  	case s3:
	  	{
	  		Permen;//keluarkan permen (led indikator)
	  		if (C1==6)//apakah banyak koin 1000 ada 2 koin
	  		{
	  			Permen;//keluarkan permen (led indikator)
	  			Kembalian;//keluarkan kembalian (led indikator)
	  		}
	  		while (Time_Loop++<=Time_Val);//menghitung delay dengan count manual
	  		if(Time_Loop>=Time_Val)//
	  		{
	  			start();//reset semua kondisi
	  			Time_Loop=0;//reset value Time_Loop
	  			stateku = s0;//kembali ke state 0
	  		}
	  		break;
	  	}
  }
  }
}

void sensor(void){
	if (HAL_GPIO_ReadPin(K5_GPIO_Port, K5_Pin)==GPIO_PIN_SET){//baca pin koin 500
		HAL_Delay(50);//delay bouncing
		if (HAL_GPIO_ReadPin(K5_GPIO_Port, K5_Pin)==GPIO_PIN_SET){//apakah masih terbaca
			++C5;//nilai koin 500 bertambah
		}
	}
	if (HAL_GPIO_ReadPin(K1_GPIO_Port, K1_Pin)==GPIO_PIN_SET){//baca pin koin 1000
			HAL_Delay(50);//delay bouncing
			if (HAL_GPIO_ReadPin(K1_GPIO_Port, K1_Pin)==GPIO_PIN_SET){//apakah masih terbaca
			++C1;//nilai koin 1000 bertambah
			}
		}
}

void start()//reset kondisi awal
{
	C5=0;//reset nilai koin 500
	C1=0;//reset nilai koin 1000
	HAL_GPIO_WritePin(Out_GPIO_Port, Out_Pin, GPIO_PIN_RESET);//matikan indikator permen
	HAL_GPIO_WritePin(Chg_GPIO_Port, Chg_Pin, GPIO_PIN_RESET);//matikan indikator kembalian
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Out_Pin|Chg_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : K5_Pin K1_Pin */
  GPIO_InitStruct.Pin = K5_Pin|K1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Out_Pin Chg_Pin */
  GPIO_InitStruct.Pin = Out_Pin|Chg_Pin;
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
