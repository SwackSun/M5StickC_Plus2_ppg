/**
 * @file main.cpp
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2023-05-25
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <Arduino.h>
#include <M5GFX.h>
#include <lgfx/v1/panel/Panel_ST7789.hpp>
#include <Button.h>
#include <Wire.h>
#include "OneButton.h"

#define LCD_MOSI_PIN 15
#define LCD_MISO_PIN -1
#define LCD_SCLK_PIN 13
#define LCD_DC_PIN 14
#define LCD_CS_PIN 5
#define LCD_RST_PIN 12
#define LCD_BUSY_PIN -1
#define LCD_BL_PIN 27

#define POWER_HOLD_PIN 4
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#define I2S_DATA_PIN 34
#define I2S_SCLK_PIN 0

#define BUTTON_A 37
#define BUTTON_B 39
#define BUTTON_C 35

#define BUZZ_PIN 2

#define DEVICE_ADDRESS (0x77)
OneButton button1(BUTTON_A, true);
bool is_attachClick = true;

class CLite_GFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    CLite_GFX(void)
    {
        {
            auto cfg = _bus_instance.config();

            cfg.pin_mosi = LCD_MOSI_PIN;
            cfg.pin_miso = LCD_MISO_PIN;
            cfg.pin_sclk = LCD_SCLK_PIN;
            cfg.pin_dc = LCD_DC_PIN;
            cfg.freq_write = 40000000;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();

            cfg.invert = true;
            cfg.pin_cs = LCD_CS_PIN;
            cfg.pin_rst = LCD_RST_PIN;
            cfg.pin_busy = LCD_BUSY_PIN;
            cfg.panel_width = 135;
            cfg.panel_height = 240;
            cfg.offset_x = 52;
            cfg.offset_y = 40;
            // cfg.offset_x     = 0;
            // cfg.offset_y     = 0;

            _panel_instance.config(cfg);
        }
        {                                        // バックライト制御の設定を行います。（必要なければ削除）
            auto cfg = _light_instance.config(); // バックライト設定用の構造体を取得します。

            cfg.pin_bl = 27;    // バックライトが接続されているピン番号
            cfg.invert = false; // バックライトの輝度を反転させる場合 true
            // cfg.freq   = 44100;           // バックライトのPWM周波数
            cfg.freq = 200;      // バックライトのPWM周波数
            cfg.pwm_channel = 7; // 使用するPWMのチャンネル番号

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance); // バックライトをパネルにセットします。
        }

        setPanel(&_panel_instance);
    }
};

/* Display */
CLite_GFX lcd;
LGFX_Sprite *Disbuff;

static void i2c_read(uint8_t registerAddress, uint16_t* registerData)
{
    uint8_t tmp[2] = {0};
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(registerAddress);
    Wire.endTransmission();
    
    Wire.requestFrom(DEVICE_ADDRESS, 2);  // Request one byte from the device
    
    for(int i=0;i<2;i++)
    {
        if (Wire.available()) 
        {
            tmp[i] = Wire.read();  // Read the byte from the I2C device
        }
    }
    *registerData = tmp[0] *256 +  tmp[1];
}

static void i2c_write(uint8_t registerAddress, uint16_t registerData)
{
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(registerAddress);
    Wire.write((uint8_t)(registerData>>8));
    Wire.write((uint8_t)(registerData&0x00FF));
    Wire.endTransmission();
}

static void reg_read(uint8_t addr,uint16_t* data)
{
    if(data!=NULL)
    {
        i2c_read(addr, data);
    }
}

static void reg_write(uint8_t addr,uint16_t data)
{
    i2c_write(addr, data);
}

void setup()
{
    gpio_reset_pin((gpio_num_t)POWER_HOLD_PIN);
    pinMode(POWER_HOLD_PIN, OUTPUT);
    digitalWrite(POWER_HOLD_PIN, HIGH);

    Wire.begin(0, 26);
    lcd.begin();
    lcd.setRotation(3);
    Disbuff = new LGFX_Sprite(&lcd);
    Disbuff->createSprite(lcd.width(), lcd.height());
    lcd.setBrightness(200);
    lcd.fillScreen(TFT_RED);
    lcd.setTextColor(TFT_WHITE, TFT_RED);
    lcd.setTextSize(3);
    lcd.fillScreen(TFT_RED);
    lcd.setCursor(10, 10);
    lcd.printf("ID:0x%02X", 0xFF);

    /* search i2c device */
    int count =0;
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            printf("Found I2C Device:  (0x%02X)\n",i);
            lcd.fillScreen(TFT_RED);
            lcd.setCursor(10, 10);
            lcd.printf("ID:0x%02X", i);
            count++;
            delay(1);
        }
    }

    delay(2000);

    button1.attachClick([]() {
        is_attachClick = true;
    });
}

void loop()
{
    button1.tick();

    if(is_attachClick)
    {
        is_attachClick = false;
        randomSeed(analogRead(33));
        uint16_t write_tmp = random(0, 65536);
        reg_write(0x00,write_tmp);
        lcd.fillRect(10,50,200,40);
        lcd.setCursor(10, 50);
        lcd.printf("WRITE:0x%04X", write_tmp);
        uint16_t read_tmp = 0;
        reg_read(0x00,&read_tmp);
        lcd.fillRect(10,90,200,40);
        lcd.setCursor(10, 90);
        lcd.printf("READ :0x%04X", read_tmp);
    }
    delay(1);
}
