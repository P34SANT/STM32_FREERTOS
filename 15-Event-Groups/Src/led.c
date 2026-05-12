/*
 * led.c
 *
 *  Created on: May 11, 2026
 *      Author: Peasant
 */


#include "led.h"

GPIO_InitTypeDef ledpin = {0};

void led_init(void){

	__HAL_RCC_GPIOA_CLK_ENABLE();

	ledpin.Pin  = GPIO_PIN_1;
	ledpin.Mode = GPIO_MODE_OUTPUT_PP;
	ledpin.Pull = GPIO_PULLDOWN ;
	ledpin.Speed= GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOA , &ledpin );

}
void led_state(char s){
	if(s == 'h' || s== 'H'){
		HAL_GPIO_WritePin(GPIOA, ledpin.Pin , GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(GPIOA, ledpin.Pin , GPIO_PIN_SET);
	}
}

void led_toggle(void){
	HAL_GPIO_TogglePin(GPIOA, ledpin.Pin );
}
