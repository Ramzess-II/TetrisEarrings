#ifndef INC_LEDMATRIX_H_
#define INC_LEDMATRIX_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------- подключим файлы ------------------------------------//
#include "stm32f0xx_hal.h"
#include "saportAndData.h"

//----------------------- дефайним значения ----------------------------------//
#define MAX_X 6
#define MAX_Y 10
#define NUM_LEDS (MAX_X * MAX_Y)
#define RESET_PULSES 150
#define WS2812_BUFFER_SIZE (NUM_LEDS * 24 + RESET_PULSES)

//----------------------- объявим функции ------------------------------------//
void ClearScreen(void);
void UpdateMatrix(void);
void SetPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
uint16_t GetLEDIndex(uint8_t x, uint8_t y);
//----------------------- объявим структуры ----------------------------------//


//------------------------------ примечания ------------------------------------------//


#ifdef __cplusplus
}
#endif


#endif /* INC_LEDMATRIX_H_ */
