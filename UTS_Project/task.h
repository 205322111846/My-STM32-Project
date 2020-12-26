#ifndef TASK_H_
#define TASK_H_

void tombolrunstop(void);
void tombolreset(void);
void sensor1(void);
void sensor2(void);
void sensor3(void);
void kirim_lcd(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif /* TASK_H_ */
