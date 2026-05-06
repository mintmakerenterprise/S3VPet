#pragma once

// TTGO T-Display S3 / LilyGO T-Display S3 hardware pins used in Darren's WiFiCom firmware.
// Keep hardware pins outside the VPet engine so the VPet can later plug into WiFiCom.

static const int S3_PIN_BTN_NEXT_BACK = 14;  // Original repo BUTTON_1: short=next, long=back
static const int S3_PIN_BTN_CONFIRM   = 0;   // Original repo BUTTON_2: short=confirm
static const int S3_PIN_BAT_ADC       = 4;
static const int S3_PIN_LCD_BL        = 38;

// Landscape logical display size after tft.setRotation(1)
// TTGO T-Display S3 is 320 × 170 in landscape.
static const int S3_SCREEN_W = 320;
static const int S3_SCREEN_H = 170;

// Original DigimonVPet colors/layout.
static const uint16_t S3_VPET_BG_COLOR = 0x86CE;
