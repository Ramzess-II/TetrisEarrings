#ifndef INC_SAPORTANDDATA_H_
#define INC_SAPORTANDDATA_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------- подключим файлы ------------------------------------//
#include "stm32f0xx_hal.h"
#include "stdint.h"
#include <stdlib.h> // Для rand()

//----------------------- дефайним значения ----------------------------------//
#define MAX_X 6
#define MAX_Y 10
#define SNAKE_MAX_LEN 60

#define Led1_Pin GPIO_PIN_3
#define Led1_GPIO_Port GPIOA
#define Led2_Pin GPIO_PIN_4
#define Led2_GPIO_Port GPIOA
#define PowerOn_Pin GPIO_PIN_6
#define PowerOn_GPIO_Port GPIOA
#define ReadKey_Pin GPIO_PIN_7
#define ReadKey_GPIO_Port GPIOA

//----------------------- объявим структуры ----------------------------------//
typedef struct {
    int8_t x;
    int8_t y;
} Point;

typedef struct {
    int16_t x;   // Позиция X (в сотых долях)
    int16_t y;   // Позиция Y (в сотых долях)
    int16_t dx;  // Скорость X
    int16_t dy;  // Скорость Y
    uint8_t r, g, b; // Цвет мячика
} Ball;

// 2. Создаем общий буфер памяти для ВСЕХ режимов
typedef union {
    // Память для Тетриса
    struct {
        uint8_t board[MAX_Y][MAX_X];
        int8_t id;
        int8_t rot;
        int8_t x;
        int8_t y;
        int8_t tx;
        int8_t trot;
    } t;

    // Память для Змейки
    struct {
        Point body[SNAKE_MAX_LEN];
        uint8_t len;
        Point foods;
        int8_t dx;
        int8_t dy;
    } s;

    // Память для Огня
    struct {
        uint8_t heat[MAX_Y][MAX_X];
    } f;

    // Память для Матрицы
    struct {
        uint8_t grid[MAX_Y][MAX_X];
        int8_t heads[MAX_X];
    } m;
    struct {
		uint8_t brightness[MAX_Y][MAX_X];
		uint8_t state[MAX_Y][MAX_X]; // 0-выкл, 1,3,5-разгорается, 2,4,6-тухнет
	} st;
	struct {                         // Память для Пинг-Понга (занимает всего 33 байта!)
		Ball balls[3];
	} p;
	struct {                // Память для Дождя (занимает 104 байта)
		struct {
			int8_t x;       // Колонка (0-5)
			int16_t y;      // Высота в субпикселях
			int16_t speed;  // Скорость падения
			uint8_t active; // Существует ли капля
		} drops[4];         // Максимум 4 падающие капли одновременно

		struct {
			int16_t x;      // Позиция X (субпиксели)
			int16_t y;      // Позиция Y (субпиксели)
			int16_t dx;     // Вектор полета по X
			int16_t dy;     // Вектор полета по Y (будет уменьшаться гравитацией)
			uint8_t life;   // Яркость / Жизнь брызги
			uint8_t active; // Существует ли брызга
		} splashes[8];      // Максимум 8 брызг
	} r;
} AppState;

extern AppState state;
//----------------------- объявим функции ------------------------------------//

//------------------------------ примечания ------------------------------------------//


#ifdef __cplusplus
}
#endif

#endif /* INC_SAPORTANDDATA_H_ */
