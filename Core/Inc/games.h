#ifndef INC_GAMES_H_
#define INC_GAMES_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------- подключим файлы ------------------------------------//
#include "ledMatrix.h"

//----------------------- дефайним значения ----------------------------------//
#define SNAKE_MAX_LEN 60 // Максимальная длина змейки (на всё поле)

// --- МАКРОСЫ ДЛЯ ТЕТРИСА (код будет думать, что переменные старые) ---
#define gameBoard         (state.t.board)
#define current_id        (state.t.id)
#define current_rotation  (state.t.rot)
#define current_x         (state.t.x)
#define current_y         (state.t.y)
#define target_x          (state.t.tx)
#define target_rot        (state.t.trot)

// --- МАКРОСЫ ДЛЯ ЗМЕЙКИ ---
#define snake             (state.s.body)
#define snake_len         (state.s.len)
#define food              (state.s.foods)
#define dir_x             (state.s.dx)
#define dir_y             (state.s.dy)

//----------------------- объявим функции ------------------------------------//
void Tetris_Init(void);
void Snake_Tick(void);
void DrawGame(void);
uint8_t CheckCollision(int8_t test_x, int8_t test_y, int8_t test_rot);
void LockPiece(void);
void ClearLines(void);
void GameOverAnim(void);
void SpawnPiece(void);
void BotMove(void);
void GameTick(void);
void Snake_SpawnFood(void);
void Snake_Init(void);
uint8_t Snake_IsSafe(int8_t x, int8_t y);
void Snake_BotLogic(void);
void Snake_GameOverAnim(void);

//----------------------- объявим структуры ----------------------------------//




#ifdef __cplusplus
}
#endif

#endif /* INC_GAMES_H_ */
