/*
  DigimonVPet S3 Original Stage 1

  Goal:
  - Preserve the original DigimonVPet screen/state-machine behavior.
  - Change only the hardware shell so it runs on TTGO T-Display S3.
  - Keep VPet as a module so WiFiCom can later call vpet.begin/update/render/signals.

  Button mapping:
  - GPIO14 short press: original BUTTON_1 short = next/menu selection
  - GPIO14 long press : original BUTTON_1 long  = back/clock
  - GPIO0  short press: original BUTTON_2 short = confirm
*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "src/S3VPetConfig.h"
#include "src/Button2.h"
#include "src/VPetOriginalApp/VPetOriginalApp.h"

TFT_eSPI tft = TFT_eSPI();
VPetOriginalApp vpet;

Button2 btnNextBack(S3_PIN_BTN_NEXT_BACK);
Button2 btnConfirm(S3_PIN_BTN_CONFIRM);

static bool debugOverlay = false;

static void initDisplay() {
  pinMode(S3_PIN_LCD_BL, OUTPUT);
  digitalWrite(S3_PIN_LCD_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(S3_VPET_BG_COLOR);
}

static void initButtons() {
  btnNextBack.setPressedHandler([](Button2& b) {
    vpet.pressNext();
  });

  btnNextBack.setLongClickHandler([](Button2& b) {
    vpet.pressBack();
  });

  btnConfirm.setPressedHandler([](Button2& b) {
    vpet.pressConfirm();
  });
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("[S3_STAGE1] DigimonVPet original display/state-machine port start");

  analogReadResolution(12);
  randomSeed(analogRead(S3_PIN_BAT_ADC));

  initDisplay();
  initButtons();

  vpet.begin(&tft);
  Serial.println("[S3_STAGE1] VPet begin done");
}

void loop() {
  uint32_t now = millis();

  vpet.update(now);
  vpet.render();

  btnNextBack.loop();
  btnConfirm.loop();

  if (debugOverlay) {
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.fillRect(0, 0, 120, 20, TFT_WHITE);
    uint32_t d = vpet.getLastDeltaMs();
    if (d == 0) d = 1;
    tft.drawString(String(1000.0f / d, 1) + " FPS", 0, 0);
  }
}
