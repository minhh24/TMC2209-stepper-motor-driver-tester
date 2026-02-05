#include "encoder.h"

// khoi tao
void ENC_Init(Encoder_HandleTypeDef *henc, TIM_HandleTypeDef *htim, GPIO_TypeDef *btn_port, uint16_t btn_pin)
{
	henc->htim = htim;
	henc->btn_port = btn_port;
	henc->btn_pin = btn_pin;

	// kich hoat encoder interface
	HAL_TIM_Encoder_Start(henc->htim, TIM_CHANNEL_ALL);
}

//encoder 4 xung
int16_t ENC_GetCounter(Encoder_HandleTypeDef *henc)
{
	return (int16_t)__HAL_TIM_GET_COUNTER(henc->htim);
}

// kiem tra nut nhan
uint8_t ENC_IsBtnPressed(Encoder_HandleTypeDef *henc)
{
	if(HAL_GPIO_ReadPin(henc->btn_port, henc->btn_pin) == GPIO_PIN_RESET)
	{
		return 1; 
	}
	return 0;
}

// Reset giá tri ve 0
void ENC_ResetCounter(Encoder_HandleTypeDef *henc)
{
	__HAL_TIM_SET_COUNTER(henc->htim, 0);
}