#ifndef GAME_H
#define GAME_H

#include "stm32f7xx_hal.h"

#define MAX_SEQUENCE 100

void Game_Init(TIM_HandleTypeDef *timer);
void Game_Run(void);

#endif
