#include "main.h"

TIM_HandleTypeDef htim3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_TIM3_Init(uint32_t Periode);

//button
unsigned char button1(void);
unsigned char button2(void);

//task.h
unsigned char bouncing=0xFF;
void task1(void);
void task2(void);
void task3(void);
void task4(void);
#define click 1
int time1, state1, count=0;

//sch
#include <stdint.h>

#define SCH_MAX_TASKS 4
#define RETURN_NORMAL  0
#define RETURN_ERROR  1

typedef struct
{
  void (* pTask)(void);
  uint16_t Delay;
  uint16_t Period;
  uint8_t RunMe;
} sTask;

uint8_t Error_code_G;
sTask SCH_tasks_G[SCH_MAX_TASKS];

// Error codes
//----------------------------------------------------------------------
#define ERROR_SCH_TOO_MANY_TASKS (1)
#define ERROR_SCH_CANNOT_DELETE_TASK (2)

// --Global function here ------------------

void TIM3_IRQHandler(void);
void SCH_init(uint32_t Periode);
void SCH_Start(void);
uint8_t SCH_Delete_Task(const uint8_t TASK_INDEX);
void SCH_update(void);
void SCH_Dispatch_Tasks(void);
uint8_t SCH_Add_Task(void (* pFunction)(),uint16_t DELAY,uint16_t PERIOD);
void SCH_Report_Status(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM3_Init(1);
  uint16_t periode=1000;
  uint16_t delay=0;

  SCH_Add_Task(task1, delay, 10);
  SCH_Add_Task(task2, delay, periode*2);
  SCH_Add_Task(task3, delay, 10);
  SCH_Add_Task(task4, delay, periode);
  	/* Start the scheduler */
  SCH_Start();

  while (1)
  {
	  SCH_Dispatch_Tasks();
  }
  /* USER CODE END 3 */
  return 0;
}

//task.c
void task1(void){
	static unsigned char debounce=0xFF;
	if(HAL_GPIO_ReadPin(T1_GPIO_Port,T1_Pin)== GPIO_PIN_SET){
		debounce=(debounce<<1);
	} else {
		debounce= (debounce<<1)|1;
	}
	if (debounce==0x01) {
		count++;
	}
	HAL_GPIO_WritePin(C1_GPIO_Port, C1_Pin, count & (1<<0));
	HAL_GPIO_WritePin(C2_GPIO_Port, C2_Pin, count & (1<<1));
	HAL_GPIO_WritePin(C3_GPIO_Port, C3_Pin, count & (1<<2));
	HAL_GPIO_WritePin(C4_GPIO_Port, C4_Pin, count & (1<<3));
}

void task2(void){
	HAL_GPIO_TogglePin(L1_GPIO_Port, L1_Pin);
}

void task3(void){
	static unsigned char debounce=0xFF;
	if(HAL_GPIO_ReadPin(T2_GPIO_Port,T2_Pin)== GPIO_PIN_SET){
		debounce=(debounce<<1);
	} else {
		debounce= (debounce<<1)|1;
	}
	if (debounce==0x01) {
		HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, GPIO_PIN_SET);
		time1=0;
		state1=1;
	}
	if(state1){
		if(time1==700){
			HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, GPIO_PIN_RESET);
			time1=0;
			state1=0;
		}
		else {
			++time1;
		}
	}
}

void task4(void){

}

//sch
void SCH_Start(void){
	HAL_TIM_Base_Start_IT(&htim3);
}

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);
	SCH_update();
}

uint8_t SCH_Add_Task(void (* pFunction)(),uint16_t DELAY,uint16_t PERIOD)
{
	uint8_t Index = 0;
	/* First find a gap in the array (if there is one) */
	while ((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS)){
		Index++;
	}
	/* Have we reached the end of the list? */
	if (Index == SCH_MAX_TASKS){
		/* Task list is full
    -> set the global error variable */
		Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
		/* Also return an error code */
		return SCH_MAX_TASKS;
	}
	/* If we're here, there is a space in the task array */
	SCH_tasks_G[Index].pTask = pFunction;
	SCH_tasks_G[Index].Delay = DELAY + 1;
	SCH_tasks_G[Index].Period = PERIOD;
	SCH_tasks_G[Index].RunMe = 0;
	return Index; /* return pos. of task (to allow deletion) */
}

//

void SCH_Dispatch_Tasks(void)
{
	uint8_t Index;
	/* Dispatches (runs) the next task (if one is ready) */
	for (Index = 0; Index < SCH_MAX_TASKS; Index++){
		if (SCH_tasks_G[Index].RunMe > 0){
			(*SCH_tasks_G[Index].pTask)(); /* Run the task */
			SCH_tasks_G[Index].RunMe --; /* Reduce RunMe count */
			/* Periodic tasks will automatically run again
      - if this is a 'one shot' task, delete it */
			if (SCH_tasks_G[Index].Period == 0){
				SCH_Delete_Task(Index);
			}
		}
	}
	/* Report system status */
	//SCH_Report_Status();
	/* The scheduler enters idle mode at this point */
//	sleep_mode();
}


uint8_t SCH_Delete_Task(const uint8_t TASK_INDEX)
{
	unsigned char Return_code;
	if (SCH_tasks_G[TASK_INDEX].pTask == 0){
		/* No task at this location...
    -> set the global error variable */
		Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
		/* ...also return an error code */
		Return_code = RETURN_ERROR;
	}
	else {
		Return_code = RETURN_NORMAL;
	}
	SCH_tasks_G[TASK_INDEX].pTask = 0x0000;
	SCH_tasks_G[TASK_INDEX].Delay = 0;
	SCH_tasks_G[TASK_INDEX].Period = 0;
	SCH_tasks_G[TASK_INDEX].RunMe = 0;
	return Return_code; /* return status */
}

void SCH_update(void)
{
	uint8_t Index;
	/* NOTE: calculations are in *TICKS* (not milliseconds) */
	for (Index = 0; Index < SCH_MAX_TASKS; Index++){
		/* Check if there is a task at this location */
		if (SCH_tasks_G[Index].pTask){
			if (--SCH_tasks_G[Index].Delay == 0){
				/* The task is due to run */
				SCH_tasks_G[Index].RunMe++; /* Inc. 'RunMe' flag */
				if (SCH_tasks_G[Index].Period){
					/* Schedule regular tasks to run again */
					SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
				}
			}
		}
	}
}


/*--------------------------------------------------------*/
void SCH_Report_Status(void)
{
#ifdef SCH_REPORT_ERRORS
	/* ONLY APPLIES IF WE ARE REPORTING ERRORS */
	/* Check for a new error code */
	if (Error_code_G != Last_error_code_G)
	{
		/* Negative logic on LEDs assumed */
		Error_port = 255 - Error_code_G;
		Last_error_code_G = Error_code_G;
		if (Error_code_G != 0)
		{
			Error_tick_count_G = 60000;
		}
		else
		{
			Error_tick_count_G = 0;
		}
	}
	else
	{
		if (Error_tick_count_G != 0)
		{
			if (--Error_tick_count_G == 0)
			{
				Error_code_G = 0; /* Reset error code */
			}
		}
	}
#endif
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
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM3_Init(uint32_t Periode)
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, C1_Pin|C2_Pin|C3_Pin|C4_Pin
                          |L1_Pin|L2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : T1_Pin T2_Pin T3_Pin */
  GPIO_InitStruct.Pin = T1_Pin|T2_Pin|T3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : C1_Pin C2_Pin C3_Pin C4_Pin
                           L1_Pin L2_Pin */
  GPIO_InitStruct.Pin = C1_Pin|C2_Pin|C3_Pin|C4_Pin
                          |L1_Pin|L2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
