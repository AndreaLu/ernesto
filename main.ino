#include<TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include<SPI.h>
#include"game.h"
#include"pin_config.h"
#include<OneButton.h>
#include <RotaryEncoder.h>
unsigned long GetMillis() { return millis(); }

/*
TODOS:
   - use sprite as render buffer to avoid flickering
   - connect to SSID: "Lo Spirito Della Nonna" / password: "SoloLeiLaSa"
   - API call to signal game over
   - test with sleeps to reduce CPU usage if possible
   - arc color while selected
   - sounds
   - encoder LEDs
   - start screen
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;
lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
TFT_eSPI tft = TFT_eSPI(170,320); // Create TFT object

Arc* gameArcs;
void ui_task(void *param);

RotaryEncoder encoder(PIN_ENCODE_A, PIN_ENCODE_B, RotaryEncoder::LatchMode::TWO03);
OneButton button(PIN_ENCODE_BTN, true);

void setup() {
    // From TFT_Rainbow example - turns on the display?
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    tft.init();
    tft.setRotation(1);

    
    //tft.begin();
    //tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);


    // Update Embed initialization parameters
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < (lcd_st7789v[i].len & 0x7f); j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }

    button.attachClick(GamePressButton);
    button.attachLongPressStart(GameLongPressButton);
    GameInit();
    gameArcs = GetArcs();

    xTaskCreatePinnedToCore(ui_task, "ui_task", 1024 * 40, NULL, 3, NULL, 1);
}

inline float radtodeg(const float rad) {
    return rad / 3.14159265f * 180.0f;
}

void loop() {
    // Render Loop
    tft.fillScreen(TFT_BLACK);
    int numArc = 0;
    
    while( gameArcs[numArc].enabled ) {
        tft.drawSmoothArc(
        /* center x,y          */ SCREEN_WIDTH/2, SCREEN_HEIGHT/2,
        /* inner, outer radius */ gameArcs[numArc].radius+1, gameArcs[numArc].radius-2,
        /* stard and end angle */ radtodeg(gameArcs[numArc].angle) - 5, radtodeg(gameArcs[numArc].angle) + 5,
        /* fg_col, bg_col      */ TFT_WHITE, TFT_BLACK, 
        /* round edges         */ true
        );
        numArc++;
    }
    // r = arc outer corner radius, ir = arc inner radius. Arc thickness = r-ir+1
    tft.drawSmoothRoundRect( 
    /* topleft x,y */ GetBarX() - 10, SCREEN_HEIGHT/2 -2, 
    /* r,ir        */ 2, 0,
    /* w,h         */ 20, 4,
    /* fg,bg color */ TFT_WHITE, TFT_BLACK,
    /* quadrants   */ 0xF // draw all quadrants
    );
    GameUpdate();
    delay(5);
}


void ui_task(void *param)
{
    //static lv_disp_draw_buf_t draw_buf;
    //static lv_color_t *buf1, *buf2;


    // Update Embed initialization parameters
    /*
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < (lcd_st7789v[i].len & 0x7f); j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
    */


    attachInterrupt( 
        digitalPinToInterrupt(PIN_ENCODE_A), []() {
        encoder.tick();
    }, CHANGE);
    attachInterrupt( 
        digitalPinToInterrupt(PIN_ENCODE_B), []() {
        encoder.tick();
    }, CHANGE);

    while (1) {
        delay(1);
        button.tick();

        RotaryEncoder::Direction dir = encoder.getDirection();
        if (dir != RotaryEncoder::Direction::NOROTATION) {
            if (dir != RotaryEncoder::Direction::CLOCKWISE) {
                GameRotateEncoder( true );
                //xEventGroupSetBits(lv_input_event, LV_ENCODER_CW);
                //xEventGroupSetBits(lv_input_event, LV_ENCODER_LED_CW);
            } else {
                GameRotateEncoder(false);
                //xEventGroupSetBits(lv_input_event, LV_ENCODER_CCW);
                //xEventGroupSetBits(lv_input_event, LV_ENCODER_LED_CCW);
            }
        }
    }

    vTaskDelete(NULL);
}
