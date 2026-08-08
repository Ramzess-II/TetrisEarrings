#include "ledMatrix.h"
//----------------------- переменные из других файлов --------------------------------//
extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_ch2;
//----------------------- переменные из этого файла ----------------------------------//
uint16_t pwmData[WS2812_BUFFER_SIZE];          // Массив для DMA таймера (наши значения ШИМ)
//------------------------------ функции ---------------------------------------------//

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_2);
    }
}

uint16_t GetLEDIndex(uint8_t x, uint8_t y) {
    if (y % 2 == 0) {                           // Четные ряды (0, 2, 4... снизу) идут справа налево
        return (y * MAX_X) + (MAX_X - 1 - x);
    }
    else {                                      // Нечетные ряды (1, 3, 5... снизу) идут слева направо
        return (y * MAX_X) + x;
    }
}


void SetPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {        // Теперь мы пишем биты напрямую в буфер DMA!
    if(x >= MAX_X || y >= MAX_Y) return;
    uint16_t led_idx = GetLEDIndex(x, y);
    uint32_t color = (g << 16) | (r << 8) | b;
    for (int bit = 23; bit >= 0; bit--) {                                     // Заполняем 24 бита для текущего светодиода
        if (color & (1 << bit)) {
            pwmData[led_idx * 24 + (23 - bit)] = 38;                          // Логическая "1" (~64% ШИМ)
        } else {
            pwmData[led_idx * 24 + (23 - bit)] = 19;                          // Логический "0" (~32% ШИМ)
        }
    }
}


void UpdateMatrix(void) {                                                     // Отправка готового буфера на матрицу
    for (uint16_t i = NUM_LEDS * 24; i < WS2812_BUFFER_SIZE; i++) {           // Гарантируем, что в конце массива остались нули для сигнала RESET
        pwmData[i] = 0;
    }
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t*)pwmData, WS2812_BUFFER_SIZE);
}

void ClearScreen(void) {                                                     // Функция для очистки экрана (гасим все светодиоды)
    for (uint8_t y = 0; y < MAX_Y; y++) {
        for (uint8_t x = 0; x < MAX_X; x++) {
            SetPixel(x, y, 0, 0, 0);
        }
    }
}

//------------------------------ примечания --------------------------------------------//

