/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Smart Stepper - 5 MODES (Added Direction Control)
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "i2c_lcd.h"
#include "encoder.h"
#include "tmc2209.h"
#include "interface.h"
#include <stdio.h>
#include <stdlib.h>

#define TIM_CLOCK_FREQ 1000000 
#define HANDWHEEL_MULTIPLIER  20  

I2C_LCD_HandleTypeDef my_lcd;
Encoder_HandleTypeDef my_encoder;
TMC2209_HandleTypeDef my_tmc;
App_Context_t my_app;

int16_t old_enc_counter = 0;
uint32_t btn_press_start_tick = 0;
uint8_t btn_holding = 0;
uint8_t btn_back_last_state = 1; 

static int16_t last_speed = -1;
static uint8_t last_run_state = 2;
static uint16_t last_current = 0;
static uint8_t last_microstep = 255;
static uint8_t last_direction = 255; // Biến theo dõi chiều quay
static uint8_t last_screen_id = 255;

/* FUNCTION PROTOTYPES */
void SystemClock_Config(void);
void Process_Input_To_Events(void);
void Sync_App_To_Hardware(void);
void Set_Motor_Frequency(uint16_t freq_hz);
void Force_PC13_Init(void); 
void Config_StepPin_As_PWM(void);
void Config_StepPin_As_GPIO(void);
void Manual_Step_Pulse(void);
void Set_Microstep_GPIO(uint8_t enum_val);

/* USER CODE BEGIN 0 */
void Force_PC13_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE(); 
    GPIOC->CRH &= ~(0xF << 20); 
    GPIOC->CRH |= (0x2 << 20);  
}

void Config_StepPin_As_PWM(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; 
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Config_StepPin_As_GPIO(void) {
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); 
}

void Manual_Step_Pulse(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    for(volatile int i=0; i<300; i++); 
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    for(volatile int i=0; i<300; i++); 
}

void Set_Motor_Frequency(uint16_t freq_hz) {
    if (freq_hz < 10) freq_hz = 10; 
    if (freq_hz > 20000) freq_hz = 20000; 
    uint32_t period = (TIM_CLOCK_FREQ / freq_hz) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim1, period);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, period / 2);
}

void Set_Microstep_GPIO(uint8_t enum_val) {
    GPIO_PinState ms1 = GPIO_PIN_RESET;
    GPIO_PinState ms2 = GPIO_PIN_RESET;
    switch(enum_val) {
        case 0: ms1 = GPIO_PIN_SET;   ms2 = GPIO_PIN_SET;   break; // 1/8
        case 1: ms1 = GPIO_PIN_RESET; ms2 = GPIO_PIN_SET;   break; // 1/16
        case 2: ms1 = GPIO_PIN_SET;   ms2 = GPIO_PIN_RESET; break; // 1/32
        case 3: ms1 = GPIO_PIN_RESET; ms2 = GPIO_PIN_RESET; break; // 1/64
        default: ms1 = GPIO_PIN_SET;  ms2 = GPIO_PIN_RESET; break;
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, ms1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, ms2);
}

void Process_Input_To_Events(void) {
    int16_t raw_counter = ENC_GetCounter(&my_encoder);
    
    // --- MENU 4: POS TEST ---
    if (my_app.currentScreen == SCREEN_POSITION_TEST) {
        int16_t diff = (raw_counter / 2) - (old_enc_counter / 2);
        if (diff != 0) {
            // Tự động đảo chiều theo tay vặn (ghi đè cài đặt Menu 5)
            if (diff > 0) TMC2209_SetDirection(&my_tmc, DIR_CW);
            else          TMC2209_SetDirection(&my_tmc, DIR_CCW);
            
            int steps_to_run = abs(diff) * HANDWHEEL_MULTIPLIER;
            if (diff > 0) my_app.currentPosCount += steps_to_run;
            else          my_app.currentPosCount -= steps_to_run;
            
            my_app.needRefresh = 1;
            TMC2209_EnableDriver(&my_tmc, TMC_Enable);
            for(int k=0; k<steps_to_run; k++) {
                Manual_Step_Pulse();
                HAL_Delay(1); 
            }
            old_enc_counter = raw_counter;
        }
    } 
    // --- CÁC MENU KHÁC ---
    else {
        int16_t diff = (raw_counter / 4) - (old_enc_counter / 4);
        if (diff != 0) {
            if (diff > 0) UI_HandleEvent(&my_app, EVENT_ENC_ROTATE_CW);
            else UI_HandleEvent(&my_app, EVENT_ENC_ROTATE_CCW);
            old_enc_counter = raw_counter;
        }
    }

    // Buttons
    if (ENC_IsBtnPressed(&my_encoder)) {
        if (btn_press_start_tick == 0) btn_press_start_tick = HAL_GetTick(); 
        if ((HAL_GetTick() - btn_press_start_tick > 800) && (btn_holding == 0)) {
            UI_HandleEvent(&my_app, EVENT_BTN_LONG_PRESS);
            btn_holding = 1; 
        }
    } else {
        if (btn_press_start_tick != 0) {
            if ((HAL_GetTick() - btn_press_start_tick < 800) && (btn_holding == 0)) {
                UI_HandleEvent(&my_app, EVENT_BTN_SHORT_PRESS);
            }
            btn_press_start_tick = 0;
            btn_holding = 0;
        }
    }
    uint8_t current_back_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10);
    if (current_back_state == 0 && btn_back_last_state == 1) {
        HAL_Delay(20); 
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0) {
            UI_HandleEvent(&my_app, EVENT_BTN_BACK);
        }
    }
    btn_back_last_state = current_back_state;
}

void Sync_App_To_Hardware(void) {
    // 0. CHUYỂN MÀN HÌNH
    if (my_app.currentScreen != last_screen_id) {
        if (my_app.currentScreen == SCREEN_POSITION_TEST) {
            Config_StepPin_As_GPIO();
            TMC2209_EnableDriver(&my_tmc, TMC_Enable);
            my_app.currentPosCount = 0; 
        }
        else if (my_app.currentScreen == SCREEN_MANUAL_RUN) {
            Config_StepPin_As_PWM();
        }
        else {
             HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
             TMC2209_SetCurrent(&my_tmc, my_app.runCurrent, 50);
        }
        
        // Reset last_direction để ép code gửi lại lệnh chiều quay khi thoát Pos Test
        last_direction = 255; 
        
        last_screen_id = my_app.currentScreen;
    }

    // 1.MANUAL RUN
    if (my_app.currentScreen == SCREEN_MANUAL_RUN) {
        if (my_app.isRunning != last_run_state) {
            if (my_app.isRunning) {
                TMC2209_EnableDriver(&my_tmc, TMC_Enable);
                __HAL_TIM_MOE_ENABLE(&htim1); 
                HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); 
            } else {
                HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
                __HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(&htim1);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
            }
            last_run_state = my_app.isRunning;
        }
        if (my_app.targetSpeed != last_speed) {
            Set_Motor_Frequency((uint16_t)my_app.targetSpeed);
            last_speed = my_app.targetSpeed;
        }
    }
    
    // 2. CÀI ĐẶT DIRECTION (Chỉ áp dụng khi không ở Pos Test)
    // Pos Test tự quản lý chiều quay, nên ta không can thiệp
    if (my_app.currentScreen != SCREEN_POSITION_TEST) {
        if (my_app.direction != last_direction) {
            TMC2209_SetDirection(&my_tmc, (TMC2209_Dir_t)my_app.direction);
            last_direction = my_app.direction;
        }
    }
    
    // 3.CÀI ĐẶT KHÁC
    if (my_app.runCurrent != last_current) {
        TMC2209_SetCurrent(&my_tmc, my_app.runCurrent, 50); 
        HAL_Delay(2);
        TMC2209_SetCurrent(&my_tmc, my_app.runCurrent, 50);
        last_current = my_app.runCurrent;
    }

    if (my_app.microstepEnum != last_microstep) {
        Set_Microstep_GPIO(my_app.microstepEnum);
        last_microstep = my_app.microstepEnum;
    }
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init(); 
  MX_TIM1_Init();
  
  Force_PC13_Init();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  my_lcd.hi2c = &hi2c1;
  lcd_init(&my_lcd);
  lcd_clear(&my_lcd);
  
  ENC_Init(&my_encoder, &htim4, GPIOB, GPIO_PIN_5);
  old_enc_counter = ENC_GetCounter(&my_encoder); 
  
  TMC2209_AttachPins(&my_tmc, GPIOA, GPIO_PIN_15, GPIOB, GPIO_PIN_14, GPIOA, GPIO_PIN_11, GPIOA, GPIO_PIN_12);
  HAL_Delay(100);
  TMC2209_Init(&my_tmc, &huart1, 0); 
  TMC2209_SetCurrent(&my_tmc, 800, 50); 
  TMC2209_UseUartForStepping(&my_tmc, 0); 
  Set_Microstep_GPIO(1); 
  
  UI_Init(&my_app, &my_lcd);
  
  lcd_gotoxy(&my_lcd, 0, 0); lcd_puts(&my_lcd, "Stepper Analyzer");
  lcd_gotoxy(&my_lcd, 0, 1); lcd_puts(&my_lcd, "System Ready!");
  HAL_Delay(1000); lcd_clear(&my_lcd);
  UI_Refresh(&my_app);

  while (1) {
    if (my_app.isRunning == 0) {
        static uint32_t led_timer = 0;
        if (HAL_GetTick() - led_timer > 1000) { 
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3); 
            led_timer = HAL_GetTick();
        }
    }
    Process_Input_To_Events();
    Sync_App_To_Hardware();
    UI_Refresh(&my_app);
    HAL_Delay(5); 
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void Error_Handler(void) { __disable_irq(); while (1) {} }
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif