#include "interface.h"
#include <stdio.h>
#include <string.h>

#define MENU_ITEMS_COUNT 5
const char *MENU_ITEMS[] = {
    "1.Manual Run", 
    "2.Set Current", 
    "3.Microstep", 
    "4.Pos. Test",
    "5.Direction"   
};

const char *MICROSTEP_STR[] = {
    "1/8 ", "1/16", "1/32", "1/64"
};

const char *DIR_STR[] = {
    "CW (Clockwise) ", 
    "CCW (Counter)  "
};

static void Draw_MainMenu(App_Context_t *app);
static void Draw_ManualRun(App_Context_t *app);
static void Draw_SetCurrent(App_Context_t *app);
static void Draw_SetMicrostep(App_Context_t *app);
static void Draw_PositionTest(App_Context_t *app);
static void Draw_SetDirection(App_Context_t *app); 

void UI_Init(App_Context_t *app, I2C_LCD_HandleTypeDef *hlcd) {
    app->lcdHandle = hlcd;
    app->currentScreen = SCREEN_MAIN_MENU;
    app->menuIndex = 0;
    
    app->targetSpeed = 0; 
    app->isRunning = 0;
    app->runCurrent = 800; 
    app->microstepEnum = 1; 
    app->direction = 0;       
    app->currentPosCount = 0;
    app->needRefresh = 1;   
}

void UI_HandleEvent(App_Context_t *app, UI_Event_t event) {
    if (event == EVENT_NONE) return;

    if (event == EVENT_BTN_BACK || event == EVENT_BTN_LONG_PRESS) {
        app->currentScreen = SCREEN_MAIN_MENU;
        app->isRunning = 0;
        app->needRefresh = 1;
        lcd_clear(app->lcdHandle);
        return;
    }

    switch (app->currentScreen) {
        case SCREEN_MAIN_MENU:
            if (event == EVENT_ENC_ROTATE_CW) {
                app->menuIndex++;
                if (app->menuIndex >= MENU_ITEMS_COUNT) app->menuIndex = 0;
            } 
            else if (event == EVENT_ENC_ROTATE_CCW) {
                if (app->menuIndex == 0) app->menuIndex = MENU_ITEMS_COUNT - 1;
                else app->menuIndex--;
            }
            else if (event == EVENT_BTN_SHORT_PRESS) {
                switch(app->menuIndex) {
                    case 0: app->currentScreen = SCREEN_MANUAL_RUN; break;
                    case 1: app->currentScreen = SCREEN_SET_CURRENT; break;
                    case 2: app->currentScreen = SCREEN_SET_MICROSTEP; break;
                    case 3: app->currentScreen = SCREEN_POSITION_TEST; break;
                    case 4: app->currentScreen = SCREEN_SET_DIRECTION; break; 
                }
                lcd_clear(app->lcdHandle);
            }
            break;

        case SCREEN_MANUAL_RUN:
            if (event == EVENT_ENC_ROTATE_CW) app->targetSpeed += 10;
            else if (event == EVENT_ENC_ROTATE_CCW) {
                if(app->targetSpeed >= 10) app->targetSpeed -= 10;
            }
            else if (event == EVENT_BTN_SHORT_PRESS) app->isRunning = !app->isRunning;
            break;

        case SCREEN_SET_CURRENT:
            if (event == EVENT_ENC_ROTATE_CW) {
                if(app->runCurrent < 2000) app->runCurrent += 50;
            }
            else if (event == EVENT_ENC_ROTATE_CCW) {
                if(app->runCurrent > 100) app->runCurrent -= 50;
            }
            break;

        case SCREEN_SET_MICROSTEP:
            if (event == EVENT_ENC_ROTATE_CW) {
                if(app->microstepEnum < 3) app->microstepEnum++;
            }
            else if (event == EVENT_ENC_ROTATE_CCW) {
                if(app->microstepEnum > 0) app->microstepEnum--;
            }
            break;
            
        case SCREEN_SET_DIRECTION:
            if (event == EVENT_ENC_ROTATE_CW || event == EVENT_ENC_ROTATE_CCW) {
                app->direction = !app->direction;
            }
            break;
            
        default: break;
    }
    app->needRefresh = 1;
}

void UI_Refresh(App_Context_t *app) {
    if (!app->needRefresh) return; 
    switch (app->currentScreen) {
        case SCREEN_MAIN_MENU: Draw_MainMenu(app); break;
        case SCREEN_MANUAL_RUN: Draw_ManualRun(app); break;
        case SCREEN_SET_CURRENT: Draw_SetCurrent(app); break;
        case SCREEN_SET_MICROSTEP: Draw_SetMicrostep(app); break;
        case SCREEN_POSITION_TEST: Draw_PositionTest(app); break;
        case SCREEN_SET_DIRECTION: Draw_SetDirection(app); break; 
        default: break;
    }
    app->needRefresh = 0;
}

// --- DRAW FUNCTIONS ---

static void Draw_MainMenu(App_Context_t *app) {
    char buffer[32]; 
    lcd_gotoxy(app->lcdHandle, 0, 0);
    sprintf(buffer, "> %s              ", MENU_ITEMS[app->menuIndex]); 
    buffer[16] = '\0'; 
    lcd_puts(app->lcdHandle, buffer);

    lcd_gotoxy(app->lcdHandle, 0, 1);
    uint8_t nextIndex = (app->menuIndex + 1) % MENU_ITEMS_COUNT;
    sprintf(buffer, "  %s              ", MENU_ITEMS[nextIndex]);
    buffer[16] = '\0';
    lcd_puts(app->lcdHandle, buffer);
}

static void Draw_ManualRun(App_Context_t *app) {
    char buffer[32];
    lcd_gotoxy(app->lcdHandle, 0, 0);
    sprintf(buffer, "Speed: %4d RPM", app->targetSpeed);
    lcd_puts(app->lcdHandle, buffer);
    lcd_gotoxy(app->lcdHandle, 0, 1);
    if(app->isRunning) lcd_puts(app->lcdHandle, "Status: RUN [X]");
    else               lcd_puts(app->lcdHandle, "Status: STOP[ ]");
}

static void Draw_SetCurrent(App_Context_t *app) {
    char buffer[32];
    lcd_gotoxy(app->lcdHandle, 0, 0);
    lcd_puts(app->lcdHandle, "Set Current (mA)");
    lcd_gotoxy(app->lcdHandle, 0, 1);
    sprintf(buffer, "I_Run: %4d mA ", app->runCurrent);
    lcd_puts(app->lcdHandle, buffer);
}

static void Draw_SetMicrostep(App_Context_t *app) {
    char buffer[32];
    lcd_gotoxy(app->lcdHandle, 0, 0);
    lcd_puts(app->lcdHandle, "Microstep Config");
    lcd_gotoxy(app->lcdHandle, 0, 1);
    sprintf(buffer, "Step: %s     ", MICROSTEP_STR[app->microstepEnum]);
    lcd_puts(app->lcdHandle, buffer);
}

static void Draw_PositionTest(App_Context_t *app) {
    char buffer[32];
    lcd_gotoxy(app->lcdHandle, 0, 0);
    lcd_puts(app->lcdHandle, "Handwheel Mode  ");
    lcd_gotoxy(app->lcdHandle, 0, 1);
    sprintf(buffer, "Steps: %ld      ", app->currentPosCount);
    lcd_puts(app->lcdHandle, buffer);
}

// --- HÀM V? MENU DIRECTION ---
static void Draw_SetDirection(App_Context_t *app) {
    char buffer[32];
    lcd_gotoxy(app->lcdHandle, 0, 0);
    lcd_puts(app->lcdHandle, "Motor Direction ");
    lcd_gotoxy(app->lcdHandle, 0, 1);
    sprintf(buffer, "Dir: %s   ", DIR_STR[app->direction]);
    lcd_puts(app->lcdHandle, buffer);
}