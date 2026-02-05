#ifndef TMC2209_H
#define TMC2209_H

#include "main.h"

#define TMC2209_RSENSE      0.11f  
#define TMC2209_VREF        3.3f   

// DIA CHI THANH GHI TMC2209 (Datasheet) 
#define TMC2209_GCONF       0x00
#define TMC2209_GSTAT       0x01
#define TMC2209_IFCNT       0x02
#define TMC2209_IOIN        0x06
#define TMC2209_IHOLD_IRUN  0x10
#define TMC2209_TPOWERDOWN  0x11
#define TMC2209_TSTEP       0x12
#define TMC2209_TPWMTHRS    0x13
#define TMC2209_VACTUAL     0x22
#define TMC2209_CHOPCONF    0x6C
#define TMC2209_PWMCONF     0x70

// cau hinh 
typedef enum {
    MICROSTEP_1   = 0, 
    MICROSTEP_2   = 1,
    MICROSTEP_4   = 2,
    MICROSTEP_8   = 3,
    MICROSTEP_16  = 4,
    MICROSTEP_32  = 5,
    MICROSTEP_64  = 6,
    MICROSTEP_128 = 7,
    MICROSTEP_256 = 8
} TMC2209_Microstep_t;

typedef enum {
    TMC_Enable  = 0, 
    TMC_Disable = 1
} TMC2209_State_t;

typedef enum {
    DIR_CW  = 0,
    DIR_CCW = 1
} TMC2209_Dir_t;

typedef struct {
    UART_HandleTypeDef *huart; 
    uint8_t slave_addr;        
    
    GPIO_TypeDef *en_port;   uint16_t en_pin;
    GPIO_TypeDef *dir_port;  uint16_t dir_pin;
    GPIO_TypeDef *ms1_port;  uint16_t ms1_pin; 
    GPIO_TypeDef *ms2_port;  uint16_t ms2_pin;
    
} TMC2209_HandleTypeDef;

void TMC2209_Init(TMC2209_HandleTypeDef *htmc, UART_HandleTypeDef *huart, uint8_t addr);
void TMC2209_AttachPins(TMC2209_HandleTypeDef *htmc, 
                        GPIO_TypeDef *en_port, uint16_t en_pin,
                        GPIO_TypeDef *dir_port, uint16_t dir_pin,
                        GPIO_TypeDef *ms1_port, uint16_t ms1_pin,
                        GPIO_TypeDef *ms2_port, uint16_t ms2_pin);


void TMC2209_EnableDriver(TMC2209_HandleTypeDef *htmc, TMC2209_State_t state);


void TMC2209_SetCurrent(TMC2209_HandleTypeDef *htmc, uint16_t run_current_ma, uint8_t hold_current_percent);

void TMC2209_SetMicrosteps_UART(TMC2209_HandleTypeDef *htmc, TMC2209_Microstep_t step);

void TMC2209_SetMicrosteps_GPIO(TMC2209_HandleTypeDef *htmc, TMC2209_Microstep_t step);

void TMC2209_SetDirection(TMC2209_HandleTypeDef *htmc, TMC2209_Dir_t dir);


void TMC2209_UseUartForStepping(TMC2209_HandleTypeDef *htmc, uint8_t enable);

#endif