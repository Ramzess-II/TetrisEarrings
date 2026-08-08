#ifndef INC_EFFECTS_H_
#define INC_EFFECTS_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------- подключим файлы ------------------------------------//
#include "ledMatrix.h"

//----------------------- дефайним значения ----------------------------------//
#define heat_map (state.f.heat)
#define m_grid   (state.m.grid)
#define m_heads  (state.m.heads)

#define st_bright (state.st.brightness)
#define st_state  (state.st.state)

#define balls_data (state.p.balls)

#define rain_drops (state.r.drops)
#define rain_splashes (state.r.splashes)
//----------------------- объявим функции ------------------------------------//
void ModeTransition(void);
void Heart_Tick(void);
void DrawSmiley(void);
void Fire_Tick(void);
void Matrix_Init(void);
void Matrix_Tick(void);
void Stars_Init(void);
void Stars_Tick(void);
void Rainbow_Tick(void);
void Balls_Init(void);
void Balls_Tick(void);
void Rain_Init(void);
void Rain_Tick(void);
//----------------------- объявим структуры ----------------------------------//




#ifdef __cplusplus
}
#endif



#endif /* INC_EFFECTS_H_ */
