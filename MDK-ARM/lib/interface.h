#ifndef INTERFACE_H
#define INTERFACE_H

#include "main.h"
#include "i2c_lcd.h"

typedef enum {
    SCREEN_MAIN_MENU = 0,
    SCREEN_MANUAL_RUN,
    SCREEN_SET_CURRENT,
    SCREEN_SET_MICROSTEP,
    SCREEN_POSITION_TEST,
    SCREEN_SET_DIRECTION    
} UI_Screen_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_ENC_ROTATE_CW,
    EVENT_ENC_ROTATE_CCW,
    EVENT_BTN_SHORT_PRESS,
    EVENT_BTN_LONG_PRESS,
    EVENT_BTN_BACK
} UI_Event_t;

typedef struct {
    UI_Screen_t currentScreen;
    uint8_t menuIndex;
    
    int16_t targetSpeed;     // RPM
    uint8_t isRunning;
    uint16_t runCurrent;     // mA
    uint8_t microstepEnum;   // Index (0-3)
    
    uint8_t direction;       
    
    int32_t currentPosCount; // dem buoc
    
    I2C_LCD_HandleTypeDef *lcdHandle;
    uint8_t needRefresh;
} App_Context_t;

void UI_Init(App_Context_t *app, I2C_LCD_HandleTypeDef *hlcd);
void UI_HandleEvent(App_Context_t *app, UI_Event_t event);
void UI_Refresh(App_Context_t *app);

#endif