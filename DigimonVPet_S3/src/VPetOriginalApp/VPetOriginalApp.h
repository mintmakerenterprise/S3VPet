#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "../GameLogic/Digimon.h"
#include "../GameLogic/ScreenStateMachine.h"
#include "../GameLogic/ESP32DigimonDataLoader.h"
#include "../VPetLCD/VPetLCD.h"
#include "../VPetLCD/VPetLCDMenuBar32p.h"
#include "../VPetLCD/DisplayAdapter/TFT_eSPI_Displayadapter.h"
#include "../VPetLCD/ESP32SpriteManager.h"

#include "../VPetLCD/Screens/AgeWeightScreen.h"
#include "../VPetLCD/Screens/DigimonNameScreen.h"
#include "../VPetLCD/Screens/HeartsScreen.h"
#include "../VPetLCD/Screens/ProgressBarScreen.h"
#include "../VPetLCD/Screens/PercentageScreen.h"
#include "../VPetLCD/Screens/SelectionScreen.h"
#include "../VPetLCD/Screens/ClockScreen.h"
#include "../VPetLCD/Screens/DigimonWatchingScreen.h"
#include "../VPetLCD/Screens/AnimationScreens/EatingAnimationScreen.h"

// Stage 1 goal:
// Preserve the original DigimonVPet main.cpp behavior, but wrap it as a class.
// The standalone .ino owns setup()/loop()/buttons/display.
// Later WiFiCom will also own setup()/loop() and call this class.

class VPetOriginalApp {
public:
  VPetOriginalApp();
  ~VPetOriginalApp();

  void begin(TFT_eSPI* display);
  void update(uint32_t nowMs);
  void render();

  // Original signal mapping from repo:
  // btn1 short = next, btn1 long = back, btn2 short = confirm.
  void pressNext();
  void pressBack();
  void pressConfirm();

  bool isReady() const { return ready; }
  uint32_t getLastDeltaMs() const { return lastDelta; }

private:
  TFT_eSPI* tft = nullptr;
  TFT_eSprite* img = nullptr;
  TFT_eSPI_DisplayAdapter* displayAdapter = nullptr;

  ESP32SpriteManager spriteManager;
  ESP32DigimonDataLoader dataLoader;

  uint16_t digiIndex = DIGIMON_BOTAMON;
  Digimon* digimon = nullptr;

  VPetLCD* screen = nullptr;
  VPetLCDMenuBar32p* menuBar = nullptr;

  V20::DigimonWatchingScreen* digimonScreen = nullptr;
  V20::DigimonNameScreen* digiNameScreen = nullptr;
  V20::AgeWeightScreen* ageWeightScreen = nullptr;
  V20::HeartsScreen* hungryScreen = nullptr;
  V20::HeartsScreen* strengthScreen = nullptr;
  V20::HeartsScreen* effortScreen = nullptr;
  V20::ProgressBarScreen* dpScreen = nullptr;
  V20::PercentageScreen* sPercentageScreen = nullptr;
  V20::PercentageScreen* tPercentageScreen = nullptr;
  V20::SelectionScreen* foodSelection = nullptr;
  V20::SelectionScreen* fightSelection = nullptr;
  V20::ClockScreen* clockScreen = nullptr;
  V20::EatingAnimationScreen* eatingAnimationScreen = nullptr;

  ScreenStateMachine* stateMachine = nullptr;

  const uint8_t numberOfScreens = 13;
  const uint8_t numberOfSignals = 3;
  const uint8_t confirmSignal = 0;
  const uint8_t nextSignal = 1;
  const uint8_t backSignal = 2;

  uint8_t digimonScreenId = 0;
  uint8_t digiNameScreenId = 0;
  uint8_t ageWeightScreenId = 0;
  uint8_t hungryScreenId = 0;
  uint8_t strengthScreenId = 0;
  uint8_t effortScreenId = 0;
  uint8_t dpScreenId = 0;
  uint8_t sPercentageScreenId = 0;
  uint8_t tPercentageScreenId = 0;
  uint8_t foodSelectionId = 0;
  uint8_t fightSelectionId = 0;
  uint8_t clockScreenId = 0;
  uint8_t eatingAnimationScreenId = 0;

  int hours = 23;
  int minutes = 59;
  int seconds = 0;

  uint32_t lastLoopStartedMs = 0;
  uint32_t lastDelta = 0;
  bool ready = false;

  void setupScreens();
  void setupStateMachine();
  void addScreensToStateMachine();
};
