#include "tmc2209.h"
#include <math.h> 

static uint8_t calcCRC(uint8_t *datagram, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t currentByte = datagram[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc >> 7) ^ (currentByte & 0x01)) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = (crc << 1);
            }
            currentByte = currentByte >> 1;
        }
    }
    return crc;
}

static void TMC2209_WriteRegister(TMC2209_HandleTypeDef *htmc, uint8_t reg, uint32_t value) {
    uint8_t buffer[8];
    
    buffer[0] = 0x05;                         // Sync byte
    buffer[1] = htmc->slave_addr;             // Slave Address
    buffer[2] = reg | 0x80;                   // Register Address + Write bit (bit 7 = 1)
    buffer[3] = (value >> 24) & 0xFF;         // Data MSB
    buffer[4] = (value >> 16) & 0xFF;
    buffer[5] = (value >> 8) & 0xFF;
    buffer[6] = value & 0xFF;                 // Data LSB
    buffer[7] = calcCRC(buffer, 7);           // CRC

    HAL_UART_Transmit(htmc->huart, buffer, 8, 100);
}

// - Khoi tao
void TMC2209_Init(TMC2209_HandleTypeDef *htmc, UART_HandleTypeDef *huart, uint8_t addr) {
    htmc->huart = huart;
    htmc->slave_addr = addr;
    

	uint32_t gconf_val = 0;
    gconf_val |= (1 << 6); // pdn_disable = 1 (UART control enabled)
    gconf_val |= (1 << 7); // mstep_reg_select = 1 (Microstep by UART register)
    
    TMC2209_WriteRegister(htmc, TMC2209_GCONF, gconf_val);
}

void TMC2209_AttachPins(TMC2209_HandleTypeDef *htmc, 
                        GPIO_TypeDef *en_port, uint16_t en_pin,
                        GPIO_TypeDef *dir_port, uint16_t dir_pin,
                        GPIO_TypeDef *ms1_port, uint16_t ms1_pin,
                        GPIO_TypeDef *ms2_port, uint16_t ms2_pin) {
    htmc->en_port = en_port;   htmc->en_pin = en_pin;
    htmc->dir_port = dir_port; htmc->dir_pin = dir_pin;
    htmc->ms1_port = ms1_port; htmc->ms1_pin = ms1_pin;
    htmc->ms2_port = ms2_port; htmc->ms2_pin = ms2_pin;
}


void TMC2209_EnableDriver(TMC2209_HandleTypeDef *htmc, TMC2209_State_t state) {
    if (state == TMC_Enable) {
        HAL_GPIO_WritePin(htmc->en_port, htmc->en_pin, GPIO_PIN_RESET); // Low = Enable
    } else {
        HAL_GPIO_WritePin(htmc->en_port, htmc->en_pin, GPIO_PIN_SET);   // High = Disable
    }
}

void TMC2209_SetDirection(TMC2209_HandleTypeDef *htmc, TMC2209_Dir_t dir) {
    HAL_GPIO_WritePin(htmc->dir_port, htmc->dir_pin, (dir == DIR_CW) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Công thuc: CS = 32 * sqrt(2) * I_rms * (Rsense + 0.02) / 0.325 - 1
void TMC2209_SetCurrent(TMC2209_HandleTypeDef *htmc, uint16_t run_current_ma, uint8_t hold_current_percent) {
    float I_rms = run_current_ma / 1000.0f; // Ð?i sang A
    float cs_float = (32.0f * 1.414f * I_rms * (TMC2209_RSENSE + 0.02f)) / 0.325f - 1.0f;
    
    int8_t cs = (int8_t)cs_float;
    if (cs < 0) cs = 0;
    if (cs > 31) cs = 31;

    int8_t cs_hold = (cs * hold_current_percent) / 100;
    
    // Ghi vào thanh ghi IHOLD_IRUN (0x10)
    // IHOLD (bit 0-4), IRUN (bit 8-12), IHOLDDELAY (bit 16-19)
    uint32_t data = 0;
    data |= ((uint32_t)cs_hold & 0x1F);       // IHOLD
    data |= (((uint32_t)cs & 0x1F) << 8);     // IRUN
    data |= (6 << 16);                      

    TMC2209_WriteRegister(htmc, TMC2209_IHOLD_IRUN, data);
}

void TMC2209_SetMicrosteps_UART(TMC2209_HandleTypeDef *htmc, TMC2209_Microstep_t step) {
    
    uint8_t mres = 0;
    switch(step) {
        case MICROSTEP_256: mres = 0; break;
        case MICROSTEP_128: mres = 1; break;
        case MICROSTEP_64:  mres = 2; break;
        case MICROSTEP_32:  mres = 3; break;
        case MICROSTEP_16:  mres = 4; break;
        case MICROSTEP_8:   mres = 5; break;
        case MICROSTEP_4:   mres = 6; break;
        case MICROSTEP_2:   mres = 7; break;
        case MICROSTEP_1:   mres = 8; break;
        default: mres = 4; // Default 16
    }

    uint32_t chopconf = 0x10000053; 
    chopconf |= ((uint32_t)mres << 24); // Set MRES bits

    TMC2209_WriteRegister(htmc, TMC2209_CHOPCONF, chopconf);
}

void TMC2209_UseUartForStepping(TMC2209_HandleTypeDef *htmc, uint8_t enable) {
    // 1: Dùng MRES trong CHOPCONF (UART)
    // 0: Dùng chân MS1/MS2
    
    uint32_t gconf_val = (1 << 6); // Luôn set pdn_disable = 1
    if (enable) {
        gconf_val |= (1 << 7);
    } else {
        gconf_val &= ~(1 << 7);
    }
    TMC2209_WriteRegister(htmc, TMC2209_GCONF, gconf_val);
}

// Logic TMC2209 Standalone:
// MS1  MS2  Steps
//  0    0   1/8
//  1    0   1/16
//  0    1   1/32
//  1    1   1/64
void TMC2209_SetMicrosteps_GPIO(TMC2209_HandleTypeDef *htmc, TMC2209_Microstep_t step) {
    TMC2209_UseUartForStepping(htmc, 0);

    GPIO_PinState ms1_state = GPIO_PIN_RESET;
    GPIO_PinState ms2_state = GPIO_PIN_RESET;

    switch(step) {
        case MICROSTEP_8:
            ms1_state = GPIO_PIN_RESET; ms2_state = GPIO_PIN_RESET;
            break;
        case MICROSTEP_16:
            ms1_state = GPIO_PIN_SET;   ms2_state = GPIO_PIN_RESET;
            break;
        case MICROSTEP_32:
            ms1_state = GPIO_PIN_RESET; ms2_state = GPIO_PIN_SET;
            break;
        case MICROSTEP_64:
            ms1_state = GPIO_PIN_SET;   ms2_state = GPIO_PIN_SET;
            break;
        default:
            ms1_state = GPIO_PIN_RESET; ms2_state = GPIO_PIN_RESET;
            break;
    }

    HAL_GPIO_WritePin(htmc->ms1_port, htmc->ms1_pin, ms1_state);
    HAL_GPIO_WritePin(htmc->ms2_port, htmc->ms2_pin, ms2_state);
}