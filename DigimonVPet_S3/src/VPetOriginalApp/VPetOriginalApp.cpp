#include "VPetOriginalApp.h"
#include "../S3VPetConfig.h"

VPetOriginalApp::VPetOriginalApp() {}

VPetOriginalApp::~VPetOriginalApp() {
  delete eatingAnimationScreen;
  delete clockScreen;
  delete fightSelection;
  delete foodSelection;
  delete tPercentageScreen;
  delete sPercentageScreen;
  delete dpScreen;
  delete effortScreen;
  delete strengthScreen;
  delete hungryScreen;
  delete ageWeightScreen;
  delete digiNameScreen;
  delete digimonScreen;
  delete stateMachine;
  delete menuBar;
  delete screen;
  delete displayAdapter;
  delete img;
  delete digimon;
}

void VPetOriginalApp::begin(TFT_eSPI* display) {
  tft = display;
  if (!tft) return;

  // Original repo used a TFT_eSprite as a full-screen canvas.
  // Keep this system exactly: VPetLCD draws into img, then img pushes to TFT.
  img = new TFT_eSprite(tft);
  displayAdapter = new TFT_eSPI_DisplayAdapter(img, S3_SCREEN_W, S3_SCREEN_H);

  digimon = new Digimon(digiIndex);

  screen = new VPetLCD(displayAdapter, &spriteManager, 40, 16);
  menuBar = new VPetLCDMenuBar32p(8, 8, S3_SCREEN_W);

  const DigimonProperties* props = dataLoader.getDigimonProperties(digiIndex);
  digimon->setProperties(props);

  // Stage 1B: start from a clean known state.
  // The original repo did not fully initialize all runtime/save variables.
  // On ESP32-S3 this can show up as many poop sprites immediately after boot.
  // Save/load will be added later; for now we force a sane fresh-start state.
  digimon->setState(0);
  digimon->setAge(0);
  digimon->setWeight(props->minWeight);
  digimon->setFeedCounter(0);
  digimon->setCareMistakes(0);
  digimon->setTrainingCounter(0);
  digimon->setPoopTimer(0);
  digimon->setAgeTimer(0);
  digimon->setEvolutionTimer(0);
  digimon->setNumberOfPoops(0);
  digimon->setHunger(10);
  digimon->setStrength(0);
  digimon->setEffort(0);
  digimon->setDigimonPower(0);

  digimonScreen = new V20::DigimonWatchingScreen(&spriteManager, digimon->getDigimonIndex(), -8, 40, 0, 0);
  digimonScreen->forceClearPoop();
  digiNameScreen = new V20::DigimonNameScreen(&spriteManager, props->digiName, digimon->getDigimonIndex(), 24);
  ageWeightScreen = new V20::AgeWeightScreen(5, 21);
  hungryScreen = new V20::HeartsScreen("Hungry", 2, 4);
  // Stage 1D: the original repo used demo placeholder values here:
  // Strength=3, Effort=4, WIN S=100%, WIN T=93%.
  // For a real fresh game state, these should start from Digimon state / 0.
  strengthScreen = new V20::HeartsScreen("Strength", 0, 4);
  effortScreen = new V20::HeartsScreen("Effort", 0, 4);
  dpScreen = new V20::ProgressBarScreen("DP", 29, 40);
  sPercentageScreen = new V20::PercentageScreen("WIN", 'S', 0);
  tPercentageScreen = new V20::PercentageScreen("WIN", 'T', 0);
  foodSelection = new V20::SelectionScreen(true);
  fightSelection = new V20::SelectionScreen(true);
  clockScreen = new V20::ClockScreen(true);
  eatingAnimationScreen = new V20::EatingAnimationScreen(&spriteManager, digimon->getDigimonIndex());

  stateMachine = new ScreenStateMachine(numberOfScreens, numberOfSignals);
  addScreensToStateMachine();
  setupScreens();
  setupStateMachine();

  lastLoopStartedMs = millis();
  lastDelta = 0;
  ready = true;
}

void VPetOriginalApp::addScreensToStateMachine() {
  digimonScreenId = stateMachine->addScreen(digimonScreen);
  digiNameScreenId = stateMachine->addScreen(digiNameScreen);
  ageWeightScreenId = stateMachine->addScreen(ageWeightScreen);
  hungryScreenId = stateMachine->addScreen(hungryScreen);
  strengthScreenId = stateMachine->addScreen(strengthScreen);
  effortScreenId = stateMachine->addScreen(effortScreen);
  dpScreenId = stateMachine->addScreen(dpScreen);
  sPercentageScreenId = stateMachine->addScreen(sPercentageScreen);
  tPercentageScreenId = stateMachine->addScreen(tPercentageScreen);
  foodSelectionId = stateMachine->addScreen(foodSelection);
  fightSelectionId = stateMachine->addScreen(fightSelection);
  clockScreenId = stateMachine->addScreen(clockScreen);
  eatingAnimationScreenId = stateMachine->addScreen(eatingAnimationScreen);
}

void VPetOriginalApp::setupScreens() {
  // S3 320×170 landscape icon setup: 8 icons across the full top bar.
  // Slot meaning:
  // 0 Status, 1 Food, 2 Training, 3 Battle, 4 Clean, 5 Light, 6 Medicine, 7 Notification.
  menuBar->setIconOnIndex(0, 0);
  menuBar->setIconOnIndex(1, 1);
  menuBar->setIconOnIndex(2, 2);
  menuBar->setIconOnIndex(3, 3);
  menuBar->setIconOnIndex(4, 4);
  menuBar->setIconOnIndex(5, 5);
  menuBar->setIconOnIndex(6, 6);
  menuBar->setIconOnIndex(7, 7); // Notification icon

  // Slightly stronger active icon color for notification/selected icon.
  menuBar->setActiveIconColor(0x0001);

  screen->setMenuBar(menuBar);
  // Stage 1F2: Correct TTGO T-Display S3 landscape layout.
  // Screen: 320 × 170 px.
  // VPet LCD grid: 40 × 16 boxes.
  // Scale 8 gives 40×8 = 320 px wide and 16×8 = 128 px high.
  // Top icon bar uses ~40 px, then the LCD starts at y=42 and ends at y=169.
  screen->setLCDPos(0, 42);
  screen->setLcdScale(8);

  const int screensOffsetX = 4;
  ageWeightScreen->setPos(screensOffsetX, 0);
  effortScreen->setPos(screensOffsetX, 0);
  strengthScreen->setPos(screensOffsetX, 0);
  hungryScreen->setPos(screensOffsetX, 0);
  dpScreen->setPos(screensOffsetX, 0);
  sPercentageScreen->setPos(screensOffsetX, 0);
  tPercentageScreen->setPos(screensOffsetX, 0);
  clockScreen->setPos(screensOffsetX, 0);
  eatingAnimationScreen->setPos(screensOffsetX, 0);

  foodSelection->addOption("Meat", SYMBOL_MEAT);
  foodSelection->addOption("PILL", SYMBOL_PILL);
  foodSelection->addOption("LOVE", SYMBOL_HEART);
  foodSelection->addOption("SHIT", SYMBOL_POOP);

  fightSelection->setShowIcons(false);
  fightSelection->addOption("SINGLE");
  fightSelection->addOption("TAG");

  clockScreen->setHours(hours);
  clockScreen->setMinutes(minutes);
  clockScreen->setSeconds(seconds);
}

void VPetOriginalApp::setupStateMachine() {
  // Original eating animation end action.
  eatingAnimationScreen->setAnimationEndAction([this]() {
    stateMachine->setCurrentScreen(foodSelectionId);
  });

  // Original code tried 1..numberOfScreens+1. Keep valid screen IDs only to avoid bad bounds.
  for (int i = 1; i <= numberOfScreens; i++) {
    stateMachine->addTransition(i, digimonScreenId, backSignal);
  }

  // Scale menu transitions, unchanged.
  stateMachine->addTransition(digiNameScreenId, ageWeightScreenId, nextSignal);
  stateMachine->addTransition(ageWeightScreenId, hungryScreenId, nextSignal);
  stateMachine->addTransition(hungryScreenId, strengthScreenId, nextSignal);
  stateMachine->addTransition(strengthScreenId, effortScreenId, nextSignal);
  stateMachine->addTransition(effortScreenId, dpScreenId, nextSignal);
  stateMachine->addTransition(dpScreenId, sPercentageScreenId, nextSignal);
  stateMachine->addTransition(sPercentageScreenId, tPercentageScreenId, nextSignal);
  stateMachine->addTransition(tPercentageScreenId, digiNameScreenId, nextSignal);

  stateMachine->addTransition(digimonScreenId, clockScreenId, backSignal);
  stateMachine->addTransition(clockScreenId, digimonScreenId, backSignal);

  stateMachine->addTransition(digimonScreenId, digimonScreenId, nextSignal);
  stateMachine->addTransitionAction(digimonScreenId, nextSignal, [this]() {
    menuBar->nextSelection();
    // Icon 7 is Notification only. It is visible, but the user should not be able to select it.
    // Sequence becomes: none -> 0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> none.
    if (menuBar->getSelection() == 7) {
      menuBar->nextSelection();
    }
  });

  stateMachine->addTransition(digimonScreenId, digimonScreenId, confirmSignal);
  stateMachine->addTransitionAction(digimonScreenId, confirmSignal, [this]() {
    uint8_t maxdp = digimon->getProperties()->maxDigimonPower;
    switch (menuBar->getSelection()) {
      case 0:
        digiNameScreen->setDigimonSpriteIndex(digimon->getDigimonIndex());
        hungryScreen->setHearts(min<uint16_t>(4, 4 - 4 * digimon->getHunger() / 10));
        strengthScreen->setHearts(min<uint16_t>(4, digimon->getStrength()));
        effortScreen->setHearts(min<uint16_t>(4, digimon->getEffort()));
        if (maxdp > 0) {
          dpScreen->setFillPercentage((digimon->getDigimonPower() * 100) / maxdp);
        } else {
          dpScreen->setFillPercentage(0);
        }
        // Original repo had no battle record system yet, so both WIN pages stay 0%.
        sPercentageScreen->setPercentage(0);
        tPercentageScreen->setPercentage(0);
        ageWeightScreen->setAge(digimon->getAge());
        ageWeightScreen->setWeight(digimon->getWeight());
        stateMachine->setCurrentScreen(digiNameScreenId);
        break;
      case 1:
        foodSelection->setSelection(0);
        stateMachine->setCurrentScreen(foodSelectionId);
        break;
      case 3:
        fightSelection->setSelection(0);
        stateMachine->setCurrentScreen(fightSelectionId);
        break;
      case 4:
        // Stage 1C: make clean action deterministic during porting.
        // Original animation can be restored later after poop state is verified.
        digimon->setNumberOfPoops(0);
        digimonScreen->forceClearPoop();
        break;
      case 2:
        // Training icon exists in the original UI, but the original repo did not implement training gameplay yet.
        Serial.println("[VPET] Training icon selected: not implemented in original repo yet.");
        break;
      case 5:
        // Light icon exists in the original UI, but no sleep/light state machine is implemented yet.
        Serial.println("[VPET] Light icon selected: not implemented in original repo yet.");
        break;
      case 6:
        // Medicine icon exists in the original UI, but sickness/medicine gameplay is not wired yet.
        Serial.println("[VPET] Medicine icon selected: not implemented in original repo yet.");
        break;
      case 7:
        // Safety guard only. Icon 7 is Notification and should be skipped by menu navigation.
        // It remains visible/dark when the pet needs attention, but has no selectable function.
        break;
      default:
        break;
    }
  });

  stateMachine->addTransition(foodSelectionId, foodSelectionId, nextSignal);
  stateMachine->addTransitionAction(foodSelectionId, nextSignal, [this]() {
    foodSelection->nextSelection();
  });

  stateMachine->addTransition(foodSelectionId, foodSelectionId, confirmSignal);
  stateMachine->addTransitionAction(foodSelectionId, confirmSignal, [this]() {
    uint8_t selection = foodSelection->getSelection();
    switch (selection) {
      case 0:
        digimon->addWeight(1);
        digimon->reduceHunger(1);
        eatingAnimationScreen->setSprites(SYMBOL_MEAT, SYMBOL_HALF_MEAT, SYMBOL_EMPTY_MEAT);
        eatingAnimationScreen->startAnimation();
        stateMachine->setCurrentScreen(eatingAnimationScreenId);
        break;
      case 1:
        digimon->addWeight(2);
        digimon->addStrength(2);
        digimon->addDigimonPower(2);
        eatingAnimationScreen->setSprites(SYMBOL_PILL, SYMBOL_HALF_PILL, SYMBOL_EMPTY);
        eatingAnimationScreen->startAnimation();
        stateMachine->setCurrentScreen(eatingAnimationScreenId);
        break;
      case 2:
        eatingAnimationScreen->setSprites(SYMBOL_HEART, SYMBOL_HEARTEMPTY, SYMBOL_EMPTY);
        eatingAnimationScreen->startAnimation();
        stateMachine->setCurrentScreen(eatingAnimationScreenId);
        break;
      case 3:
        eatingAnimationScreen->setSprites(SYMBOL_POOP, SYMBOL_HALF_PILL, SYMBOL_EMPTY);
        eatingAnimationScreen->startAnimation();
        stateMachine->setCurrentScreen(eatingAnimationScreenId);
        break;
    }
  });

  stateMachine->addTransition(eatingAnimationScreenId, foodSelectionId, confirmSignal);
  stateMachine->addTransitionAction(eatingAnimationScreenId, confirmSignal, [this]() {
    eatingAnimationScreen->abortAnimation();
  });

  stateMachine->addTransition(fightSelectionId, fightSelectionId, nextSignal);
  stateMachine->addTransitionAction(fightSelectionId, nextSignal, [this]() {
    fightSelection->nextSelection();
  });
}

void VPetOriginalApp::update(uint32_t nowMs) {
  if (!ready) return;

  uint32_t currentDelta = nowMs - lastLoopStartedMs;
  lastLoopStartedMs = nowMs;
  lastDelta = currentDelta;

  digimon->loop(lastDelta);
  digimonScreen->setNumberOfPoop(digimon->getNumberOfPoops());

  // Notification icon logic for Stage 1F:
  // visible/dark only when the pet needs attention. No blinking.
  // Keep it simple for now: hungry or poop. Sick/sad can be added later.
  if (menuBar) {
    bool notificationOn = (digimon->getHunger() <= 1) || (digimon->getNumberOfPoops() > 0);
    menuBar->setIconActive(7, notificationOn);
  }

  digimonScreen->loop(lastDelta);
  clockScreen->loop(lastDelta);
  digiNameScreen->loop(lastDelta);

  if (stateMachine->getCurrentScreen() == eatingAnimationScreen) {
    eatingAnimationScreen->loop(lastDelta);
  }
}

void VPetOriginalApp::render() {
  if (!ready) return;
  screen->renderScreen(stateMachine->getCurrentScreen());
}

void VPetOriginalApp::pressNext() {
  if (ready) stateMachine->sendSignal(nextSignal);
}

void VPetOriginalApp::pressBack() {
  if (ready) stateMachine->sendSignal(backSignal);
}

void VPetOriginalApp::pressConfirm() {
  if (ready) stateMachine->sendSignal(confirmSignal);
}
