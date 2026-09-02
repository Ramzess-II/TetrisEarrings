#include "effects.h"
//----------------------- переменные из других файлов --------------------------------//

//----------------------- переменные из этого файла ----------------------------------//
int heart_brightness = 5;   // Переменные для Сердца
int heart_fade_step = 2;

//------------------------------ функции ---------------------------------------------//
void DrawSmiley(void) {                                                 // Рисуем тестовый смайлик
    ClearScreen();

    // Рисуем глаза (Green)
    // Координаты X (от 0 до 5), Y (от 0 до 9, где 0 - это самый низ)

    // Левый глаз
    SetPixel(1, 7, 0, 20, 0);
    SetPixel(1, 6, 0, 20, 0);


    // Правый глаз
    SetPixel(4, 7, 0, 20, 0);
    SetPixel(4, 6, 0, 20, 0);

    // Рисуем улыбку (красным цветом)
    SetPixel(0, 4, 20, 0, 0); // Левый край улыбки
    SetPixel(1, 3, 20, 0, 0);
    SetPixel(2, 2, 20, 0, 0); // Центр улыбки
    SetPixel(3, 2, 20, 0, 0); // Центр улыбки
    SetPixel(4, 3, 20, 0, 0);
    SetPixel(5, 4, 20, 0, 0); // Правый край улыбки
}

// Функция для отрисовки сердца с заданной яркостью (от 0 до 255)
void DrawHeart(uint8_t brightness) {
    ClearScreen(); // Очищаем буфер

    // Задаем цвет (чисто красный).
    // Если хочешь розовый, можно добавить немного синего: b = brightness / 3;
    uint8_t r = brightness;
    uint8_t g = 0;
    uint8_t b = 0;

    // Рисуем верхушки (дуги) сердца
    SetPixel(1, 7, r, g, b);
    SetPixel(4, 7, r, g, b);

    // Верхняя широкая часть
    SetPixel(0, 6, r, g, b);
    SetPixel(1, 6, r, g, b);
    SetPixel(2, 6, r, g, b);
    SetPixel(3, 6, r, g, b);
    SetPixel(4, 6, r, g, b);
    SetPixel(5, 6, r, g, b);

    // Средняя широкая часть
    SetPixel(0, 5, r, g, b);
    SetPixel(1, 5, r, g, b);
    SetPixel(2, 5, r, g, b);
    SetPixel(3, 5, r, g, b);
    SetPixel(4, 5, r, g, b);
    SetPixel(5, 5, r, g, b);

    // Сужение к низу
    SetPixel(1, 4, r, g, b);
    SetPixel(2, 4, r, g, b);
    SetPixel(3, 4, r, g, b);
    SetPixel(4, 4, r, g, b);

    // Самый низ (острие)
    SetPixel(2, 3, r, g, b);
    SetPixel(3, 3, r, g, b);
}

// --- ТИК СЕРДЦА ---
void Heart_Tick(void) {
    DrawHeart(heart_brightness);
    UpdateMatrix();

    heart_brightness += heart_fade_step;
    if (heart_brightness >= 60) {
        heart_fade_step = -2;
    } else if (heart_brightness <= 5) {
        heart_fade_step = 2;
    }
}

// --- АНИМАЦИЯ ПЕРЕХОДА (ШТОРКА) ---
void ModeTransition(void) {
    // Плавно стираем экран сверху вниз
    for (int y = MAX_Y - 1; y >= 0; y--) {
        for (int x = 0; x < MAX_X; x++) {
            SetPixel(x, y, 0, 0, 0);
        }
        UpdateMatrix();
        HAL_Delay(40); // Задержка для плавности шторки
    }
    HAL_Delay(300); // Пауза в полной темноте перед новым режимом
}

// --- АНИМАЦИЯ ВЫКЛЮЧЕНИЯ ("СХЛОПЫВАНИЕ", КАК У СТАРЫХ ЭЛТ-ТЕЛЕВИЗОРОВ) ---
// Экран стягивается по вертикали в узкую полосу, затем по горизонтали - в яркую
// точку по центру, которая вспыхивает и гаснет. Вызывается один раз перед тем,
// как окончательно погасить матрицу и снять питание.
void ShutdownAnim(void) {
    const uint8_t r = 18, g = 26, b = 18; // Холодноватое бело-зеленое свечение, как у люминофора ЭЛТ

    // 1. Схлопываем экран по вертикали к двум центральным строкам (4 и 5)
    for (int8_t h = MAX_Y / 2 - 1; h >= 0; h--) {
        ClearScreen();
        for (int8_t y = 0; y < MAX_Y; y++) {
            int8_t dist = (y <= 4) ? (4 - y) : (y - 5);
            if (dist <= h) {
                for (int8_t x = 0; x < MAX_X; x++) SetPixel(x, y, r, g, b);
            }
        }
        UpdateMatrix();
        HAL_Delay(35);
    }

    // 2. Стягиваем получившуюся полосу по горизонтали к центру (колонки 2 и 3)
    for (int8_t w = MAX_X / 2 - 1; w >= 0; w--) {
        ClearScreen();
        for (int8_t x = 0; x < MAX_X; x++) {
            int8_t dist = (x <= 2) ? (2 - x) : (x - 3);
            if (dist <= w) {
                SetPixel(x, 4, r, g, b);
                SetPixel(x, 5, r, g, b);
            }
        }
        UpdateMatrix();
        HAL_Delay(35);
    }

    // 3. Короткая яркая вспышка получившейся точки...
    SetPixel(2, 4, 45, 65, 45); SetPixel(3, 4, 45, 65, 45);
    SetPixel(2, 5, 45, 65, 45); SetPixel(3, 5, 45, 65, 45);
    UpdateMatrix();
    HAL_Delay(60);

    // 4. ...и плавное угасание точки в полную темноту
    for (uint8_t fade = 5; fade > 0; fade--) {
        uint8_t fr = (r * fade) / 5, fg = (g * fade) / 5, fb = (b * fade) / 5;
        SetPixel(2, 4, fr, fg, fb); SetPixel(3, 4, fr, fg, fb);
        SetPixel(2, 5, fr, fg, fb); SetPixel(3, 5, fr, fg, fb);
        UpdateMatrix();
        HAL_Delay(40);
    }

    ClearScreen();
    UpdateMatrix();
}

// ================= РЕЖИМ: ОГОНЬ (Динамичный, с отрывом пламени) =================
void Fire_Tick(void) {
    // 1. Остывание и движение вверх (читаем сверху вниз)
    for (int y = MAX_Y - 1; y >= 1; y--) {
        for (int x = 0; x < MAX_X; x++) {
            int from_x = x;

            // Виляем влево/вправо только с вероятностью 25%
            if (rand() % 4 == 0) {
                from_x = x + (rand() % 3) - 1;
                if (from_x < 0) from_x = 0;
                if (from_x >= MAX_X) from_x = MAX_X - 1;
            }

            int heat = heat_map[y - 1][from_x];

            // Прогрессивное остывание
            int cooling = rand() % 4;
            cooling += y;

            // Ледяной потолок
            if (y >= MAX_Y - 2) {
                cooling += 20;
            }

            if (heat > cooling) {
                heat_map[y][x] = heat - cooling;
            } else {
                heat_map[y][x] = 0;
            }
        }
    }

    // 2. АГРЕССИВНОЕ остывание дна (чтобы пламя отрывалось)
    for (int x = 0; x < MAX_X; x++) {
        // Остужаем на случайную величину от 10 до 20 за кадр (раньше было всего 5)
        int bottom_cooling = 10 + (rand() % 10);
        if (heat_map[0][x] > bottom_cooling) {
            heat_map[0][x] -= bottom_cooling;
        } else {
            heat_map[0][x] = 0;
        }
    }

    // 3. Рваная генерация искр
    // Даем 40% шанс на то, что кадр будет "пустым".
    // В эти моменты пламя как раз будет отрываться от земли и улетать вверх!
    if (rand() % 100 < 60) {
        int sparks_count = 1 + (rand() % 2); // 1 или 2 узких очага
        for (int i = 0; i < sparks_count; i++) {
            int spark_x = rand() % MAX_X;
            // Делаем искру чуть ярче (60-89), чтобы она успела пролететь высоко, прежде чем потухнет
            heat_map[0][spark_x] = 60 + (rand() % 30);
        }
    }

    // 4. Рендер в цвета
    ClearScreen();
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            uint8_t h = heat_map[y][x];
            if (h > 0) {
                uint8_t r = h;
                // Желтизна только в самых горячих точках у основания (h > 40)
                uint8_t g = (h > 40) ? (h - 40) : 0;
                uint8_t b = 0;

                SetPixel(x, y, r, g, b);
            }
        }
    }
    UpdateMatrix();
}

// ================= РЕЖИМ: МАТРИЦА =================
void Matrix_Init(void) {
    for(int x = 0; x < MAX_X; x++) {
        m_heads[x] = -1; // -1 означает, что в колонке пока нет падающей капли
        for(int y = 0; y < MAX_Y; y++) m_grid[y][x] = 0;
    }
}

// ================= РЕЖИМ: МАТРИЦА (Только зеленый) =================
void Matrix_Tick(void) {
    // 1. Плавное затухание старых следов
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            if (m_grid[y][x] >= 6) m_grid[y][x] -= 6; // Скорость исчезновения хвоста
            else m_grid[y][x] = 0;
        }
    }

    // 2. Движение капель вниз
    for (int x = 0; x < MAX_X; x++) {
        if (m_heads[x] >= 0) {
            m_grid[m_heads[x]][x] = 60; // Ярко-зеленая голова
            m_heads[x]--;               // Капля падает ниже
        } else {
            // Шанс появления новой капли сверху
            if (rand() % 12 == 0) {
                m_heads[x] = MAX_Y - 1;
            }
        }
    }

    // 3. Рендер в ЧИСТО зеленый цвет
    ClearScreen();
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            uint8_t val = m_grid[y][x];
            if (val > 0) {
                // Никаких примесей красного или синего, только чистая яркость зеленого
                SetPixel(x, y, 0, val, 0);
            }
        }
    }
    UpdateMatrix();
}

// ================= РЕЖИМ: ЗВЕЗДНОЕ НЕБО =================
void Stars_Init(void) {
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            st_bright[y][x] = 0;
            st_state[y][x] = 0;
        }
    }
}

void Stars_Tick(void) {
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {

            // Если клетка пустая (нет звезды)
            if (st_state[y][x] == 0) {
                // Шанс появления уменьшен почти в 10 раз (0.2%), чтобы звезд было мало
                if (rand() % 1000 < 4) {
                    // Выбираем оттенок: 1 (Красный), 2 (Синий), 3 (Фиолетовый)
                    st_state[y][x] = 1 + (rand() % 3);

                    // РЕЗКАЯ ВСПЫШКА: сразу задаем высокую случайную яркость от 50 до 80
                    st_bright[y][x] = 50 + (rand() % 30);
                }
            }
            else {
                // Если звезда уже есть - она только ТУХНЕТ
                if (st_bright[y][x] > 4) {
                    st_bright[y][x] -= 3; // Скорость затухания (чем больше цифра, тем быстрее гаснет)
                } else {
                    st_bright[y][x] = 0;
                    st_state[y][x] = 0;   // Звезда полностью погасла, клетка свободна
                }
            }
        }
    }

    // Отрисовка
    ClearScreen();
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            if (st_state[y][x] != 0) {
                uint8_t b = st_bright[y][x];
                uint8_t s = st_state[y][x];
                uint8_t red = 0, green = 0, blue = 0;

                // Назначаем цвета
                if (s == 1) {
                    red = b; blue = b / 6;       // Красный
                } else if (s == 2) {
                    red = b / 6; blue = b;       // Синий
                } else if (s == 3) {
                    red = b / 2; blue = b;       // Фиолетовый
                }

                SetPixel(x, y, red, green, blue);
            }
        }
    }
    UpdateMatrix();
}

// ================= Вспомогательная функция для Радуги =================
// Переводит позицию (0-255) в цвет RGB.
void ColorWheel(uint8_t pos, uint8_t *r, uint8_t *g, uint8_t *b) {
    pos = 255 - pos;
    if (pos < 85) {
        *r = 255 - pos * 3;
        *g = 0;
        *b = pos * 3;
    } else if (pos < 170) {
        pos -= 85;
        *r = 0;
        *g = pos * 3;
        *b = 255 - pos * 3;
    } else {
        pos -= 170;
        *r = pos * 3;
        *g = 255 - pos * 3;
        *b = 0;
    }
    // Приглушаем яркость в 8 раз, чтобы серьги не выжгли глаза (получаем ~12% яркости)
    *r = *r / 8;
    *g = *g / 8;
    *b = *b / 8;
}

// ================= РЕЖИМ: РАДУЖНАЯ ВОЛНА =================
static uint8_t rainbow_offset = 0; // Сдвиг волны (глобальная переменная, памяти почти не ест)

void Rainbow_Tick(void) {
    rainbow_offset += 3; // Скорость движения волны

    ClearScreen();
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            uint8_t r, g, b;

            // Математика диагонали: умножаем координаты на "ширину" полосы
            uint8_t pixel_hue = (x * 20) + (y * 20) + rainbow_offset;

            ColorWheel(pixel_hue, &r, &g, &b);
            SetPixel(x, y, r, g, b);
        }
    }
    UpdateMatrix();
}

// Функция для случайного цвета при ударе
void ChangeBallColor(uint8_t idx) {
    uint8_t c = rand() % 6;
    if (c == 0) { balls_data[idx].r = 40; balls_data[idx].g = 0;  balls_data[idx].b = 0;  }
    else if (c == 1) { balls_data[idx].r = 0;  balls_data[idx].g = 40; balls_data[idx].b = 0;  }
    else if (c == 2) { balls_data[idx].r = 0;  balls_data[idx].g = 0;  balls_data[idx].b = 40; }
    else if (c == 3) { balls_data[idx].r = 30; balls_data[idx].g = 30; balls_data[idx].b = 0;  }
    else if (c == 4) { balls_data[idx].r = 30; balls_data[idx].g = 0;  balls_data[idx].b = 30; }
    else if (c == 5) { balls_data[idx].r = 0;  balls_data[idx].g = 30; balls_data[idx].b = 30; }
}

// ================= ФУНКЦИЯ ПЛАВНОЙ ОТРИСОВКИ МЯЧИКА (Anti-Aliasing) =================
void DrawSmoothBall(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
    // "Пол"-деление и остаток вместо обычных / и % — те округляют к нулю и для
    // отрицательных x/y дают отрицательный остаток, из-за чего rx/ry (uint8_t)
    // заворачиваются в огромные значения и веса w00..w11 вылетают за 0..100.
    int16_t px16 = x / 100;
    int16_t rx16 = x % 100;
    if (rx16 < 0) { rx16 += 100; px16--; }

    int16_t py16 = y / 100;
    int16_t ry16 = y % 100;
    if (ry16 < 0) { ry16 += 100; py16--; }

    int8_t px = (int8_t)px16;     // Основной пиксель X
    int8_t py = (int8_t)py16;     // Основной пиксель Y
    uint8_t rx = (uint8_t)rx16;   // Смещение от 0 до 99
    uint8_t ry = (uint8_t)ry16;   // Смещение от 0 до 99

    // Вычисляем процент свечения (от 0 до 100) для 4 соседних светодиодов
    uint8_t w00 = ((100 - rx) * (100 - ry)) / 100; // Левый верхний
    uint8_t w10 = (rx * (100 - ry)) / 100;         // Правый верхний
    uint8_t w01 = ((100 - rx) * ry) / 100;         // Левый нижний
    uint8_t w11 = (rx * ry) / 100;                 // Правый нижний

    // Отрисовываем 4 пикселя, пропорционально распределяя яркость цвета (с защитой от границ в SetPixel)
    if (w00 > 0) SetPixel(px,     py,     (r * w00)/100, (g * w00)/100, (b * w00)/100);
    if (w10 > 0) SetPixel(px + 1, py,     (r * w10)/100, (g * w10)/100, (b * w10)/100);
    if (w01 > 0) SetPixel(px,     py + 1, (r * w01)/100, (g * w01)/100, (b * w01)/100);
    if (w11 > 0) SetPixel(px + 1, py + 1, (r * w11)/100, (g * w11)/100, (b * w11)/100);
}

// ================= РЕЖИМ: ПИНГ-ПОНГ =================
void Balls_Init(void) {
    balls_data[0].x = 100; balls_data[0].y = 100;
    balls_data[0].dx = 12; balls_data[0].dy = 18;
    ChangeBallColor(0);

    balls_data[1].x = 400; balls_data[1].y = 200;
    balls_data[1].dx = -16; balls_data[1].dy = 12;
    ChangeBallColor(1);

    balls_data[2].x = 200; balls_data[2].y = 700;
    balls_data[2].dx = 14; balls_data[2].dy = -20;
    ChangeBallColor(2);
}

void Balls_Tick(void) {
    int16_t max_x = (MAX_X - 1) * 100;
    int16_t max_y = (MAX_Y - 1) * 100;

    // 1. Двигаем мячики и отбиваем от стен
    for (int i = 0; i < 3; i++) {
        balls_data[i].x += balls_data[i].dx;
        balls_data[i].y += balls_data[i].dy;

        uint8_t bounced = 0;

        if (balls_data[i].x <= 0) {
            balls_data[i].x = 0;
            balls_data[i].dx = -balls_data[i].dx;
            bounced = 1;
        } else if (balls_data[i].x >= max_x) {
            balls_data[i].x = max_x;
            balls_data[i].dx = -balls_data[i].dx;
            bounced = 1;
        }

        if (balls_data[i].y <= 0) {
            balls_data[i].y = 0;
            balls_data[i].dy = -balls_data[i].dy;
            bounced = 1;
        } else if (balls_data[i].y >= max_y) {
            balls_data[i].y = max_y;
            balls_data[i].dy = -balls_data[i].dy;
            bounced = 1;
        }

        if (bounced) ChangeBallColor(i);
    }

    // 2. Проверяем столкновения друг с другом
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 3; j++) {
            int16_t dist_x = (balls_data[i].x > balls_data[j].x) ? (balls_data[i].x - balls_data[j].x) : (balls_data[j].x - balls_data[i].x);
            int16_t dist_y = (balls_data[i].y > balls_data[j].y) ? (balls_data[i].y - balls_data[j].y) : (balls_data[j].y - balls_data[i].y);

            if (dist_x < 100 && dist_y < 100) {
                int16_t temp_dx = balls_data[i].dx;
                int16_t temp_dy = balls_data[i].dy;
                balls_data[i].dx = balls_data[j].dx;
                balls_data[i].dy = balls_data[j].dy;
                balls_data[j].dx = temp_dx;
                balls_data[j].dy = temp_dy;

                balls_data[i].x += balls_data[i].dx * 5;
                balls_data[i].y += balls_data[i].dy * 5;
                balls_data[j].x += balls_data[j].dx * 5;
                balls_data[j].y += balls_data[j].dy * 5;

                // Раздвижка после столкновения может вытолкнуть мячик за поле
                // (например, если он ударился о шарик сразу после отскока от стены) —
                // возвращаем его в границы, иначе DrawSmoothBall получит "залётные" координаты.
                if (balls_data[i].x < 0) balls_data[i].x = 0;
                else if (balls_data[i].x > max_x) balls_data[i].x = max_x;
                if (balls_data[i].y < 0) balls_data[i].y = 0;
                else if (balls_data[i].y > max_y) balls_data[i].y = max_y;

                if (balls_data[j].x < 0) balls_data[j].x = 0;
                else if (balls_data[j].x > max_x) balls_data[j].x = max_x;
                if (balls_data[j].y < 0) balls_data[j].y = 0;
                else if (balls_data[j].y > max_y) balls_data[j].y = max_y;

                ChangeBallColor(i);
                ChangeBallColor(j);
            }
        }
    }

    // 3. Плавная отрисовка
    ClearScreen();
    for (int i = 0; i < 3; i++) {
        // Вместо одного грубого пикселя рисуем сглаженный субпиксельный мячик!
        DrawSmoothBall(balls_data[i].x, balls_data[i].y, balls_data[i].r, balls_data[i].g, balls_data[i].b);
    }
    UpdateMatrix();
}

// ================= РЕЖИМ: ДОЖДЬ НА СТЕКЛЕ =================
void Rain_Init(void) {
    for(int i = 0; i < 4; i++) rain_drops[i].active = 0;
    for(int i = 0; i < 8; i++) rain_splashes[i].active = 0;
}

void Rain_Tick(void) {
    // 1. Создание новых капель (теперь редко!)
    if (rand() % 100 < 4) { // Шанс снижен с 15% до 4%
        for (int i = 0; i < 4; i++) {
            if (!rain_drops[i].active) {
                rain_drops[i].x = rand() % MAX_X;
                rain_drops[i].y = (MAX_Y - 1) * 100;
                rain_drops[i].speed = 18 + (rand() % 12); // Было 15 + rand()%10, ускорили падение примерно на 20%
                rain_drops[i].active = 1;
                break;
            }
        }
    }

    // 2. Падение капель
    for (int i = 0; i < 4; i++) {
        if (rain_drops[i].active) {
            rain_drops[i].y -= rain_drops[i].speed;

            // Если капля ударилась о дно
            if (rain_drops[i].y <= 0) {
                rain_drops[i].active = 0;

                // Создаем 2 брызги
                int spawned = 0;
                for (int j = 0; j < 8 && spawned < 2; j++) {
                    if (!rain_splashes[j].active) {
                        rain_splashes[j].active = 1;
                        rain_splashes[j].x = rain_drops[i].x * 100;
                        rain_splashes[j].y = 0;

                        // Разлет в стороны
                        rain_splashes[j].dx = (spawned == 0) ? (-12 - (rand() % 10)) : (12 + (rand() % 10));
                        // Брызги подлетают не так высоко
                        rain_splashes[j].dy = 15 + (rand() % 15);
                        // Яркость брызги (быстрее тухнет)
                        rain_splashes[j].life = 40 + (rand() % 20);
                        spawned++;
                    }
                }
            }
        }
    }

    // 3. Полет брызг (с физикой)
    for (int i = 0; i < 8; i++) {
        if (rain_splashes[i].active) {
            rain_splashes[i].x += rain_splashes[i].dx;
            rain_splashes[i].y += rain_splashes[i].dy;

            rain_splashes[i].dy -= 5; // Гравитация стала сильнее (брызги быстрее падают)

            // Брызга гаснет быстрее
            if (rain_splashes[i].life > 5) {
                rain_splashes[i].life -= 5;
            } else {
                rain_splashes[i].active = 0;
            }

            if (rain_splashes[i].x <= 0 || rain_splashes[i].x >= (MAX_X - 1) * 100) {
                rain_splashes[i].dx = -rain_splashes[i].dx;
            }

            if (rain_splashes[i].y <= 0) {
                rain_splashes[i].y = 0;
                rain_splashes[i].dy = 0;
                rain_splashes[i].dx = rain_splashes[i].dx / 2;
            }
        }
    }

    // 4. Отрисовка
    ClearScreen();

    // Сначала рисуем брызги
    for (int i = 0; i < 8; i++) {
        if (rain_splashes[i].active) {
            uint8_t l = rain_splashes[i].life;
            // Цвет брызги: добавляем красный и зеленый, чтобы брызги казались БЕЛЫМИ, а не синими
            DrawSmoothBall(rain_splashes[i].x, rain_splashes[i].y, l/3, l/2, l);
        }
    }

    // Поверх рисуем сами капли дождя
    for (int i = 0; i < 4; i++) {
        if (rain_drops[i].active) {
            int8_t px = rain_drops[i].x;
            int8_t py = rain_drops[i].y / 100;

            SetPixel(px, py, 0, 15, 45); // Голова капли (Глубокий сине-голубой)
            if (py + 1 < MAX_Y) SetPixel(px, py + 1, 0, 5, 20); // Хвост
            if (py + 2 < MAX_Y) SetPixel(px, py + 2, 0, 1, 5);  // Конец хвоста
        }
    }

    UpdateMatrix();
}
//------------------------------ примечания --------------------------------------------//
