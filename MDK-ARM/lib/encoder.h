#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"

// C?u trúc qu?n lý Encoder
typedef struct {
	TIM_HandleTypeDef *htim;    // Timer dùng cho mode Encoder (TIM4)
	GPIO_TypeDef *btn_port;     
	uint16_t btn_pin;         
} Encoder_HandleTypeDef;

void ENC_Init(Encoder_HandleTypeDef *henc, TIM_HandleTypeDef *htim, GPIO_TypeDef *btn_port, uint16_t btn_pin);

int16_t ENC_GetCounter(Encoder_HandleTypeDef *henc);

uint8_t ENC_IsBtnPressed(Encoder_HandleTypeDef *henc);

void ENC_ResetCounter(Encoder_HandleTypeDef *henc);

#endif