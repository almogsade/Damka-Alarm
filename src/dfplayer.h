#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <stdint.h>

// --- COMMAND DEFINITIONS ---
#define DF_CMD_NEXT       0x01
#define DF_CMD_PREV       0x02
#define DF_CMD_PLAY_TRACK 0x03 // Parameters: 0x00, TrackNum
#define DF_CMD_VOL_UP     0x04
#define DF_CMD_VOL_DOWN   0x05
#define DF_CMD_SET_VOL    0x06 // Parameters: 0x00, Volume (0-30)
#define DF_CMD_EQ         0x07 // Parameters: 0x00, EQ_Type
#define DF_CMD_RESET      0x0C
#define DF_CMD_PLAY       0x0D
#define DF_CMD_PAUSE      0x0E
#define DF_CMD_FOLDER     0x0F // Parameters: FolderNum, TrackNum

// --- API PROTOTYPES ---
void DF_Init(void);
void DF_PlayTrack(uint16_t trackNum);
void DF_SetVolume(uint8_t volume);
void DF_Pause(void);
void DF_Resume(void);
void DF_Reset(void);

#endif