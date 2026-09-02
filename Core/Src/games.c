#include "games.h"
//----------------------- переменные из других файлов --------------------------------//

//----------------------- переменные из этого файла ----------------------------------//
int current_brightness = 5; // Начальная яркость
int fade_step = 2;          // Шаг изменения яркости (скорость пульсации)
// Цвета для 7 классических фигур (плюс 0 - пустая клетка). Формат {R, G, B}
const uint8_t colors[8][3] = {
  {0, 0, 0},       // 0: Пусто
  {0, 20, 20},     // 1: I - Голубой
  {0, 0, 20},      // 2: J - Синий
  {20, 10, 0},     // 3: L - Оранжевый
  {20, 20, 0},     // 4: O - Желтый
  {0, 20, 0},      // 5: S - Зеленый
  {15, 0, 15},     // 6: T - Фиолетовый
  {20, 0, 0}       // 7: Z - Красный
};

// Массив фигур: 7 фигур по 4 состояния поворота
const uint16_t tetrominoes[7][4] = {
  {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
  {0x44C0, 0x8E00, 0x6440, 0x0E20}, // J
  {0x4460, 0x0E80, 0xC440, 0x2E00}, // L
  {0xCC00, 0xCC00, 0xCC00, 0xCC00}, // O
  {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
  {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
  {0x0C60, 0x4C80, 0xC600, 0x2640}  // Z
};

// Счетчик тиков для независимой анимации
uint8_t game_tick_counter = 0;

//------------------------------ функции ---------------------------------------------//
void DrawGame(void) {                               // Обновленная функция отрисовки (исправлено направление оси Y)
  ClearScreen();
  for (int y = 0; y < MAX_Y; y++) {                 // 1. Отрисовываем "стакан" (уже упавшие фигуры)
	  for (int x = 0; x < MAX_X; x++) {
		  if (gameBoard[y][x] != 0) {
			  uint8_t c_id = gameBoard[y][x];
			  SetPixel(x, y, colors[c_id][0], colors[c_id][1], colors[c_id][2]);
		  }
	  }
  }
  // 2. Отрисовываем текущую падающую фигуру
  uint16_t piece = tetrominoes[current_id][current_rotation];
  uint8_t c_id = current_id + 1;
  for (int px = 0; px < 4; px++) {
	  for (int py = 0; py < 4; py++) {
		  if (piece & (1 << (15 - (py * 4 + px)))) {
			  int draw_x = current_x + px;
			  int draw_y = current_y - py;         // Y направлен вверх, фигура вычитается вниз

			  if (draw_x >= 0 && draw_x < MAX_X && draw_y >= 0 && draw_y < MAX_Y) {
				  SetPixel(draw_x, draw_y, colors[c_id][0], colors[c_id][1], colors[c_id][2]);
			  }
		  }
	  }
  }
}
// Проверка столкновений
uint8_t CheckCollision(int8_t test_x, int8_t test_y, int8_t test_rot) {
  uint16_t piece = tetrominoes[current_id][test_rot];
  for (int px = 0; px < 4; px++) {
	  for (int py = 0; py < 4; py++) {
		  if (piece & (1 << (15 - (py * 4 + px)))) {
			  int board_x = test_x + px;
			  int board_y = test_y - py;
			  if (board_x < 0 || board_x >= MAX_X || board_y < 0) {       // Столкновение со стенами и полом
				  return 1;
			  }
			  if (board_y < MAX_Y && gameBoard[board_y][board_x] != 0) {  // Столкновение с другими блоками (если мы в пределах поля)
				  return 1;
			  }
		  }
	  }
  }
  return 0;                                                               // Путь свободен
}

void LockPiece(void) {                                                    // Фиксация фигуры в стакане
  uint16_t piece = tetrominoes[current_id][current_rotation];
  for (int px = 0; px < 4; px++) {
	  for (int py = 0; py < 4; py++) {
		  if (piece & (1 << (15 - (py * 4 + px)))) {
			  int board_x = current_x + px;
			  int board_y = current_y - py;
			  if (board_y >= 0 && board_y < MAX_Y && board_x >= 0 && board_x < MAX_X) {
				  gameBoard[board_y][board_x] = current_id + 1;
			  }
		  }
	  }
  }
}

void ClearLines(void) {                                                   // Удаление заполненных линий
  for (int y = 0; y < MAX_Y; y++) {
	  uint8_t full = 1;
	  for (int x = 0; x < MAX_X; x++) {
		  if (gameBoard[y][x] == 0) {
			  full = 0;
			  break;
		  }
	  }
	  if (full) {                                                         // Сдвигаем всё, что выше, на одну строку вниз
		  for (int shift_y = y; shift_y < MAX_Y - 1; shift_y++) {
			  for (int x = 0; x < MAX_X; x++) {
				  gameBoard[shift_y][x] = gameBoard[shift_y + 1][x];
			  }
		  }
		  for (int x = 0; x < MAX_X; x++) {                               // Очищаем самую верхнюю строку
			  gameBoard[MAX_Y - 1][x] = 0;
		  }
		  y--;                                                            // Проверяем эту же строку еще раз (так как в нее упали новые блоки)
	  }
  }
}

// Анимация проигрыша
void GameOverAnim(void) {
  // 1. Прячем текущую (новую) фигуру далеко за пределы экрана,
  // чтобы она не перекрывала красную анимацию своим цветом
  current_y = 100;
  // 2. Плавно заливаем экран красным снизу вверх
  for (int y = 0; y < MAX_Y; y++) {
	  for (int x = 0; x < MAX_X; x++) {
		  gameBoard[y][x] = 7;
	  }
	  DrawGame();
	  UpdateMatrix();
	  HAL_Delay(50); // Чуть ускорили анимацию
  }

  // 3. Небольшая пауза на полностью красном экране
  HAL_Delay(500);

  // 4. Очищаем стакан для новой игры
  for (int y = 0; y < MAX_Y; y++) {
	  for (int x = 0; x < MAX_X; x++) {
		  gameBoard[y][x] = 0;
	  }
  }
}

// Бот-"ИИ": перебирает все повороты и колонки для текущей фигуры,
// на лету укладывает её в копию стакана и оценивает результат простой
// эвристикой (высота стакана, дыры под фигурой, перепад высот колонок,
// собранные линии) - в духе классических Tetris-ботов (Dellacherie/El-Tetris),
// но на целых числах, без float, чтобы не раздувать прошивку на Cortex-M0.
static void ChooseBestMove(void) {
  int16_t best_score = -32000;
  int8_t best_rot = 0;
  int8_t best_x = current_x;

  for (int8_t rot = 0; rot < 4; rot++) {
    for (int8_t x = -3; x < MAX_X + 3; x++) {
      if (CheckCollision(x, MAX_Y, rot)) continue; // Фигура тут не помещается по ширине

      // Находим, на какую строку фигура реально упадет
      int8_t y = MAX_Y;
      while (!CheckCollision(x, y - 1, rot)) y--;

      // Пробуем уложить фигуру в копию стакана
      uint8_t temp[MAX_Y][MAX_X];
      for (int8_t ty = 0; ty < MAX_Y; ty++) {
        for (int8_t tx = 0; tx < MAX_X; tx++) temp[ty][tx] = gameBoard[ty][tx];
      }

      uint16_t piece = tetrominoes[current_id][rot];
      for (int8_t px = 0; px < 4; px++) {
        for (int8_t py = 0; py < 4; py++) {
          if (piece & (1 << (15 - (py * 4 + px)))) {
            int8_t bx = x + px;
            int8_t by = y - py;
            if (bx >= 0 && bx < MAX_X && by >= 0 && by < MAX_Y) temp[by][bx] = 1;
          }
        }
      }

      // Высота каждой колонки и дыры под фигурами
      int8_t heights[MAX_X];
      int16_t holes = 0;
      for (int8_t tx = 0; tx < MAX_X; tx++) {
        int8_t top = -1;
        for (int8_t ty = MAX_Y - 1; ty >= 0; ty--) {
          if (temp[ty][tx] != 0) { top = ty; break; }
        }
        heights[tx] = top + 1;
        for (int8_t ty = 0; ty < top; ty++) {
          if (temp[ty][tx] == 0) holes++;
        }
      }

      int16_t agg_height = 0, bumpiness = 0;
      for (int8_t tx = 0; tx < MAX_X; tx++) {
        agg_height += heights[tx];
        if (tx > 0) {
          int8_t d = heights[tx] - heights[tx - 1];
          bumpiness += (d < 0) ? -d : d;
        }
      }

      int16_t lines = 0;
      for (int8_t ty = 0; ty < MAX_Y; ty++) {
        uint8_t full = 1;
        for (int8_t tx = 0; tx < MAX_X; tx++) {
          if (temp[ty][tx] == 0) { full = 0; break; }
        }
        if (full) lines++;
      }

      int16_t score = (lines * 8) - (agg_height * 4) - (holes * 6) - (bumpiness * 2);

      if (score > best_score) {
        best_score = score;
        best_rot = rot;
        best_x = x;
      }
    }
  }

  target_rot = best_rot;
  target_x = best_x;
}

void SpawnPiece(void) {
  // 1. Генерируем параметры для новой фигуры
  current_id = rand() % 7;
  current_rotation = 0;
  current_x = 2;
  current_y = MAX_Y;

  ChooseBestMove(); // Вместо случайной цели - осознанный выбор поворота и колонки

  // 2. Проверяем, есть ли место на поле
  if (CheckCollision(current_x, MAX_Y - 1, current_rotation)) {
	  // Места нет - запускаем красивый финал (фигура прячется на Y=100)
	  GameOverAnim();

	  // 3. ИСПРАВЛЕНИЕ: После очистки стакана генерируем фигуру заново,
	  // чтобы она появилась прямо над экраном, а не падала с высоты 100!
	  current_id = rand() % 7;
	  current_rotation = 0;
	  current_x = 2;
	  current_y = MAX_Y;
	  ChooseBestMove();
  }
}

// Пошагово ведет фигуру к цели, выбранной в ChooseBestMove (поворот, затем сдвиг)
void BotMove(void) {
  // Сначала пытаемся повернуть фигуру, если цель еще не достигнута
  if (current_rotation != target_rot) {
	  int8_t next_rot = (current_rotation + 1) % 4;
	  if (!CheckCollision(current_x, current_y, next_rot)) {
		  current_rotation = next_rot;
	  } else {
		  target_rot = current_rotation; // Уперлись — отменяем план поворота
	  }
  }
  // Если повернули как надо, начинаем двигаться по горизонтали
  else if (current_x < target_x) {
	  if (!CheckCollision(current_x + 1, current_y, current_rotation)) {
		  current_x++;
	  } else {
		  target_x = current_x; // Уперлись в стену или блок — остаемся тут
	  }
  }
  else if (current_x > target_x) {
	  if (!CheckCollision(current_x - 1, current_y, current_rotation)) {
		  current_x--;
	  } else {
		  target_x = current_x;
	  }
  }
}
// Главный игровой тик
void GameTick(void) {
  game_tick_counter++;

  // Бот делает горизонтальный шаг или поворот каждые 3 тика
  if (game_tick_counter % 3 == 0) {
	  BotMove();
  }

  // Гравитация тянет фигуру вниз каждые 5 тиков (падает медленнее, чем двигается вбок)
  if (game_tick_counter >= 5) {
	  game_tick_counter = 0; // Сбрасываем счетчик

	  if (!CheckCollision(current_x, current_y - 1, current_rotation)) {
		  current_y--;
	  } else {
		  LockPiece();
		  ClearLines();
		  SpawnPiece();
	  }
  }

  DrawGame();
  UpdateMatrix();
}

// Спавн еды в случайном свободном месте
void Snake_SpawnFood(void) {
  uint8_t valid = 0;
  while (!valid) {
	  food.x = rand() % MAX_X;
	  food.y = rand() % MAX_Y;

	  valid = 1;
	  // Проверяем, чтобы еда не появилась внутри змейки
	  for (int i = 0; i < snake_len; i++) {
		  if (snake[i].x == food.x && snake[i].y == food.y) {
			  valid = 0;
			  break;
		  }
	  }
  }
}

// Сброс и инициализация игры Змейка
void Snake_Init(void) {
  snake_len = 3;
  // Начальное положение змейки по центру
  snake[0].x = 3; snake[0].y = 5; // Голова
  snake[1].x = 2; snake[1].y = 5;
  snake[2].x = 1; snake[2].y = 5; // Хвост

  dir_x = 1; // Движемся вправо
  dir_y = 0;

  Snake_SpawnFood();
}

// Проверка: безопасна ли клетка для шага?
uint8_t Snake_IsSafe(int8_t x, int8_t y) {
  // Выход за границы поля
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y) return 0;

  // Врезание в себя (не проверяем самый последний сегмент хвоста, так как он уедет)
  for (int i = 0; i < snake_len - 1; i++) {
	  if (snake[i].x == x && snake[i].y == y) return 0;
  }
  return 1;
}

// Логика выбора направления для Авто-Бота
void Snake_BotLogic(void) {
  Point head = snake[0];

  // Возможные направления: Вверх, Вниз, Влево, Вправо
  int8_t dx[4] = {0, 0, -1, 1};
  int8_t dy[4] = {1, -1, 0, 0};

  int best_dir = -1;
  int min_dist = 999;

  // 1. Пытаемся найти безопасный шаг, который приблизит нас к еде
  for (int i = 0; i < 4; i++) {
	  // Нельзя разворачиваться на 180 градусов
	  if (dx[i] == -dir_x && dy[i] == -dir_y) continue;

	  int8_t next_x = head.x + dx[i];
	  int8_t next_y = head.y + dy[i];

	  if (Snake_IsSafe(next_x, next_y)) {
		  // Расстояние до еды (Манхэттенское)
		  int dist = abs(food.x - next_x) + abs(food.y - next_y);
		  if (dist < min_dist) {
			  min_dist = dist;
			  best_dir = i;
		  }
	  }
  }

  // 2. Если путь к еде заблокирован, выбираем ЛЮБОЙ безопасный поворот
  if (best_dir == -1) {
	  for (int i = 0; i < 4; i++) {
		  if (dx[i] == -dir_x && dy[i] == -dir_y) continue;
		  if (Snake_IsSafe(head.x + dx[i], head.y + dy[i])) {
			  best_dir = i;
			  break;
		  }
	  }
  }

  // Применяем выбранное направление
  if (best_dir != -1) {
	  dir_x = dx[best_dir];
	  dir_y = dy[best_dir];
  }
}

// Анимация врезания (Game Over)
void Snake_GameOverAnim(void) {
  // Вспышка красным
  for (int i = 0; i < 3; i++) {
	  ClearScreen();
	  UpdateMatrix();
	  HAL_Delay(100);

	  // Заливаем всё красным
	  for (int y = 0; y < MAX_Y; y++) {
		  for (int x = 0; x < MAX_X; x++) {
			  SetPixel(x, y, 30, 0, 0);
		  }
	  }
	  UpdateMatrix();
	  HAL_Delay(100);
  }

  Snake_Init(); // Перезапускаем игру
}

// Главный тик Змейки
void Snake_Tick(void) {
  Snake_BotLogic(); // Бот делает выбор

  int8_t new_x = snake[0].x + dir_x;
  int8_t new_y = snake[0].y + dir_y;

  // Если ходить больше некуда — проигрыш
  if (!Snake_IsSafe(new_x, new_y)) {
	  Snake_GameOverAnim();
	  return;
  }

  // Проверка съедания еды
  uint8_t ate_food = (new_x == food.x && new_y == food.y);

  if (ate_food) {
	  if (snake_len < SNAKE_MAX_LEN) snake_len++;
	  Snake_SpawnFood();
  }

  // Двигаем тело (каждый сегмент занимает место предыдущего)
  for (int i = snake_len - 1; i > 0; i--) {
	  snake[i] = snake[i - 1];
  }

  // Двигаем голову
  snake[0].x = new_x;
  snake[0].y = new_y;

  // ОТРИСОВКА
  ClearScreen();

  // 1. Рисуем приманку (Синяя)
  SetPixel(food.x, food.y, 0, 0, 40);

  // 2. Рисуем тело змейки (Зеленое)
  for (int i = 1; i < snake_len; i++) {
	  SetPixel(snake[i].x, snake[i].y, 0, 30, 0);
  }

  // 3. Рисуем голову змейки (Красная)
  SetPixel(snake[0].x, snake[0].y, 40, 0, 0);

  UpdateMatrix();
}

// --- ИНИЦИАЛИЗАЦИЯ ТЕТРИСА ---
void Tetris_Init(void) {
  for(int y = 0; y < MAX_Y; y++) {
	  for(int x = 0; x < MAX_X; x++) {
		  gameBoard[y][x] = 0;
	  }
  }
  SpawnPiece();
}

//------------------------------ примечания --------------------------------------------//


