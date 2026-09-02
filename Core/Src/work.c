#include "work.h"
//----------------------- переменные из других файлов --------------------------------//
// Подключаем АЦП, который инициализирован в main.c
extern ADC_HandleTypeDef hadc;

//----------------------- переменные из этого файла ----------------------------------//
// Переменные для переключения режимов
uint8_t current_mode = 0;   // 0 - Тетрис, 1 - Змейка, 2 - Сердце
uint32_t mode_timer = 0;    // Таймер смены режимов (45 сек)
uint32_t action_timer = 0;  // Таймер кадров (FPS)

// Переменные для умной кнопки и АЦП
uint32_t btn_press_time = 0;
uint8_t btn_prev_state = 0;
uint8_t auto_mode_switch = 1; // 1 - автопереключение работает, 0 - остановлено
uint32_t adc_timer = 0;

//------------------------------ функции ---------------------------------------------//
void doWork (void){
	Balls_Init();                    // Стартуем с первого режима (Пинг-Понга)
    mode_timer = HAL_GetTick();
    action_timer = HAL_GetTick();

    // Ожидание дребезга и включения
    if (HAL_GPIO_ReadPin(ReadKey_GPIO_Port, ReadKey_Pin)){
        HAL_Delay(200);
    }
    if (HAL_GPIO_ReadPin(ReadKey_GPIO_Port, ReadKey_Pin)) {
        HAL_GPIO_WritePin(PowerOn_GPIO_Port, PowerOn_Pin, GPIO_PIN_SET);
        DrawSmiley();
        UpdateMatrix();
    }
    // Ждем, пока пользователь отпустит кнопку при включении
    while (HAL_GPIO_ReadPin(ReadKey_GPIO_Port, ReadKey_Pin));
    HAL_Delay(200);
}

void Work (void){
    uint32_t current_time = HAL_GetTick();

    // ================= 1. ЛОГИКА УМНОЙ КНОПКИ =================
    uint8_t btn_state = HAL_GPIO_ReadPin(ReadKey_GPIO_Port, ReadKey_Pin);

    // Кнопка только что нажата
    if (btn_state == GPIO_PIN_SET && btn_prev_state == GPIO_PIN_RESET) {
        btn_press_time = current_time;
    }
    // Кнопка удерживается
    else if (btn_state == GPIO_PIN_SET && btn_prev_state == GPIO_PIN_SET) {
        if (current_time - btn_press_time > 1000) { // Длинное нажатие (1 секунда) - ВЫКЛЮЧЕНИЕ
            ShutdownAnim();
            HAL_GPIO_WritePin(PowerOn_GPIO_Port, PowerOn_Pin, GPIO_PIN_RESET);
            while(1); // Ждем обесточивания платы
        }
    }
    // Кнопка отпущена
    else if (btn_state == GPIO_PIN_RESET && btn_prev_state == GPIO_PIN_SET) {
        if (current_time - btn_press_time > 50 && current_time - btn_press_time <= 1000) {
            // Короткое нажатие (от 50 до 1000 мс) - СМЕНА РЕЖИМА
            ModeTransition();
            current_mode++;
            if (current_mode > MODE_STEP) current_mode = 0; // <-- Теперь переключаем до 8!
            if (current_mode == 0) Balls_Init();    // Инициализация Пинг-Понга
    		if (current_mode == 1) Tetris_Init();
    		if (current_mode == 2) Snake_Init();
    		if (current_mode == 5) Matrix_Init();   // Огню и Сердцу инициализация не нужна
    		if (current_mode == 6) Stars_Init();    // Инициализация Звезд
    		if (current_mode == 8) Rain_Init();     // <-- Инициализация Дождя
            auto_mode_switch = 0; // Пользователь сам переключил режим, отключаем авто-смену!
        }
    }
    btn_prev_state = btn_state; // Запоминаем состояние кнопки для следующего цикла


    // ================= 2. ЛОГИКА АЦП (Опрос раз в 500 мс) =================
    if (current_time - adc_timer >= 500) {
        adc_timer = current_time;

        HAL_ADC_Start(&hadc); // Запускаем преобразование
        // Ждем завершения преобразования (максимум 5 мс)
        if (HAL_ADC_PollForConversion(&hadc, 5) == HAL_OK) {
            uint16_t adc_val = HAL_ADC_GetValue(&hadc); // Читаем результат
            if (adc_val < 2700) {
                // Напряжение упало — спасаем аккумулятор, выключаемся!
                ShutdownAnim();
                HAL_GPIO_WritePin(PowerOn_GPIO_Port, PowerOn_Pin, GPIO_PIN_RESET);
                while(1);
            }
        }
    }


    // ================= 3. АВТОМАТИЧЕСКАЯ СМЕНА РЕЖИМОВ =================
    // Сработает только если авто-смена включена (auto_mode_switch == 1)
    if (auto_mode_switch && (current_time - mode_timer >= 45000)) {
        ModeTransition();
        current_mode++;
        if (current_mode > MODE_STEP) current_mode = 0; // Теперь у нас 8 режимов

        if (current_mode == 0) Balls_Init();    // Инициализация Пинг-Понга
		if (current_mode == 1) Tetris_Init();
		if (current_mode == 2) Snake_Init();
		if (current_mode == 5) Matrix_Init();   // Огню и Сердцу инициализация не нужна
		if (current_mode == 6) Stars_Init();    // Инициализация Звезд
		if (current_mode == 8) Rain_Init();     // <-- Инициализация Дождя
        mode_timer = HAL_GetTick();
    }


    // ================= 4. ОТРИСОВКА ИГР =================
	if (current_mode == 0) {                              // РЕЖИМ 0: Пинг-Понг
	    if (current_time - action_timer >= 30) {          // 30 мс (высокий FPS для плавности)
	        Balls_Tick();
	        action_timer = current_time;
	    }
	}
	else if (current_mode == 1) {                         // 1: Тетрис
		if (current_time - action_timer >= 50) {
			GameTick();
			action_timer = current_time;
		}
	}
	else if (current_mode == 2) {                         // 2: Змейка
		if (current_time - action_timer >= 220) {
			Snake_Tick();
			action_timer = current_time;
		}
	}
	else if (current_mode == 3) {                         // 3: Сердце
		if (current_time - action_timer >= 30) {
			Heart_Tick();
			action_timer = current_time;
		}
	}
	else if (current_mode == 4) {                         // 4: Огонь
		if (current_time - action_timer >= 60) {          // 60 мс - оптимальная скорость горения
			Fire_Tick();
			action_timer = current_time;
		}
	}
	else if (current_mode == 5) {                         // 5: Матрица
		if (current_time - action_timer >= 80) {          // 80 мс - чтобы хвосты успевали таять
			Matrix_Tick();
			action_timer = current_time;
		}
	}
	else if (current_mode == 6) {                         // РЕЖИМ 6: Звездное небо
	    if (current_time - action_timer >= 40) {          // 40 мс - оптимальная скорость мерцания
	        Stars_Tick();
	        action_timer = current_time;
	    }
	}
	else if (current_mode == 7) {                         // РЕЖИМ 7: Радуга
		if (current_time - action_timer >= 30) {      // 30 мс - плавный перелив
			Rainbow_Tick();
			action_timer = current_time;
		}
	}
	else if (current_mode == 8) {                         // РЕЖИМ 8: Дождь
	    if (current_time - action_timer >= 30) {          // 30 мс для плавной физики
	        Rain_Tick();
	        action_timer = current_time;
	    }
	}
}
//------------------------------ примечания --------------------------------------------//


