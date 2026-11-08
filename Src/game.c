#include "game.h"
#include "main.h"
#include <stdint.h>

#define COLORS 4

static TIM_HandleTypeDef *buzzerTimer;

static uint8_t sequence[MAX_SEQUENCE];
static uint32_t randomState;

static uint8_t level = 1;
static uint8_t highScore = 0;
static uint32_t mistakes = 0;
static uint32_t lastReactionMs = 0;

static const uint16_t frequencies[COLORS] = {
    415, 330, 220, 196
};

static GPIO_TypeDef *ledPorts[COLORS] = {
    LED_GREEN_GPIO_Port,
    LED_YELLOW_GPIO_Port,
    LED_RED_GPIO_Port,
    LED_BLUE_GPIO_Port
};

static const uint16_t ledPins[COLORS] = {
    LED_GREEN_Pin,
    LED_YELLOW_Pin,
    LED_RED_Pin,
    LED_BLUE_Pin
};

static GPIO_TypeDef *buttonPorts[COLORS] = {
    BTN_GREEN_GPIO_Port,
    BTN_YELLOW_GPIO_Port,
    BTN_RED_GPIO_Port,
    BTN_BLUE_GPIO_Port
};

static const uint16_t buttonPins[COLORS] = {
    BTN_GREEN_Pin,
    BTN_YELLOW_Pin,
    BTN_RED_Pin,
    BTN_BLUE_Pin
};

static uint32_t nextRandom(void)
{
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;

    return randomState;
}

static void allLedsOff(void)
{
    for (int i = 0; i < COLORS; i++) {
        HAL_GPIO_WritePin(
            ledPorts[i], ledPins[i], GPIO_PIN_RESET
        );
    }
}

static void stopTone(void)
{
    __HAL_TIM_SET_COMPARE(
        buzzerTimer, TIM_CHANNEL_3, 0
    );
}

static void startTone(uint16_t frequency)
{
    // Requires TIM2 counter clock = 1 MHz.
    uint32_t period = 1000000UL / frequency;

    __HAL_TIM_SET_AUTORELOAD(
        buzzerTimer, period - 1
    );

    __HAL_TIM_SET_COUNTER(buzzerTimer, 0);

    __HAL_TIM_SET_COMPARE(
        buzzerTimer, TIM_CHANNEL_3, period / 2
    );
}

static void playColor(uint8_t color, uint32_t ms)
{
    HAL_GPIO_WritePin(
        ledPorts[color], ledPins[color], GPIO_PIN_SET
    );

    startTone(frequencies[color]);

    HAL_Delay(ms);

    stopTone();

    HAL_GPIO_WritePin(
        ledPorts[color], ledPins[color], GPIO_PIN_RESET
    );
}

static int readButton(void)
{
    for (int i = 0; i < COLORS; i++) {
        if (HAL_GPIO_ReadPin(
            buttonPorts[i], buttonPins[i]
        ) == GPIO_PIN_RESET) {

            HAL_Delay(25); // Debounce

            if (HAL_GPIO_ReadPin(
                buttonPorts[i], buttonPins[i]
            ) == GPIO_PIN_RESET) {
                return i;
            }
        }
    }

    return -1;
}

static void waitForRelease(int button)
{
    while (HAL_GPIO_ReadPin(
        buttonPorts[button], buttonPins[button]
    ) == GPIO_PIN_RESET) {
        HAL_Delay(1);
    }

    HAL_Delay(25);
}

static void generateSequence(void)
{
    for (int i = 0; i < MAX_SEQUENCE; i++) {
        sequence[i] = nextRandom() % COLORS;
    }
}

static void failureSound(void)
{
    allLedsOff();

    for (int i = 0; i < COLORS; i++) {
        HAL_GPIO_WritePin(
            ledPorts[i], ledPins[i], GPIO_PIN_SET
        );
    }

    startTone(150);
    HAL_Delay(600);
    stopTone();

    allLedsOff();
    HAL_Delay(500);
}

static void victorySound(void)
{
    for (int i = 0; i < COLORS; i++) {
        playColor(i, 180);
        HAL_Delay(70);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < COLORS; j++) {
            HAL_GPIO_WritePin(
                ledPorts[j], ledPins[j], GPIO_PIN_SET
            );
        }

        HAL_Delay(200);
        allLedsOff();
        HAL_Delay(200);
    }
}

static void playSequence(void)
{
    uint32_t duration = 500;

    if (level > 5) {
        duration = 400;
    }

    if (level > 10) {
        duration = 300;
    }

    if (level > 20) {
        duration = 220;
    }

    HAL_Delay(500);

    for (int i = 0; i < level; i++) {
        playColor(sequence[i], duration);
        HAL_Delay(180);
    }
}

static uint8_t checkSequence(void)
{
    for (int i = 0; i < level; i++) {
        int button = -1;

        uint32_t startTime = HAL_GetTick();

        while (button < 0) {
            button = readButton();
        }

        lastReactionMs = HAL_GetTick() - startTime;

        // Keep sound and light on until release.
        HAL_GPIO_WritePin(
            ledPorts[button],
            ledPins[button],
            GPIO_PIN_SET
        );

        startTone(frequencies[button]);

        waitForRelease(button);

        stopTone();

        HAL_GPIO_WritePin(
            ledPorts[button],
            ledPins[button],
            GPIO_PIN_RESET
        );

        if (button != sequence[i]) {
            return 0;
        }
    }

    return 1;
}

void Game_Init(TIM_HandleTypeDef *timer)
{
    buzzerTimer = timer;

    HAL_TIM_PWM_Start(
        buzzerTimer, TIM_CHANNEL_3
    );

    stopTone();
    allLedsOff();

    // Initial seed; improve with hardware entropy later.
    randomState = HAL_GetTick() ^ 0xA37F91C5UL;

    if (randomState == 0) {
        randomState = 1;
    }
}

void Game_Run(void)
{
    generateSequence();
    level = 1;

    while (1) {
        playSequence();

        if (checkSequence()) {
            if (level > highScore) {
                highScore = level;
            }

            if (level == MAX_SEQUENCE) {
                victorySound();
                return;
            }

            level++;
            HAL_Delay(700);
        } else {
            mistakes++;
            failureSound();
            return;
        }
    }
}
