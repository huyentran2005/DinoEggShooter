#ifndef USER_AUDIO_ASSETS_H_
#define USER_AUDIO_ASSETS_H_

#include <stdint.h>
#include <math.h>

#define SAMPLING_RATE 16000
#define MAX_SAMPLES   16000

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


extern const unsigned char gameover_sound[20800];
extern const unsigned char break_sound_v1[16968];
extern const unsigned char break_sound_v2[6684];
extern const unsigned char break_sound_v3[5094];
extern int16_t breakBuffer[2000];
extern int16_t startBuffer[8000];
extern int16_t gameOverBuffer[16000];
extern int16_t shootBuffer[1600];

extern volatile uint8_t audioBusy;
void CreateStartGameSound(void);
void CreateGameOverSound(void);
void CreateBubbleShootSound(void);

#endif /* USER_AUDIO_ASSETS_H_ */
