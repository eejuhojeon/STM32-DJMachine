#ifndef __GUI_H
#define __GUI_H

#define PLAYTIME_COLUMN 	24
#define PLAYTIME_PAGE 		300

#define FILENAME_COLUMN 	24
#define FILENAME_PAGE 		252
#define NO_FILENAME 		"NOT PLAYING NOW"

#define PLAY_STAT_COLUMN	24
#define PLAY_STAT_PAGE		220

#define MUSIC_PLAYING 		1
#define MUSIC_PAUSED 		0
#define NO_MUSIC 			-1

#define FILTER_STAT_COLUMN	24
#define FILTER_STAT_PAGE	172

#define NO_FILTER			-1
#define LPF 				0
#define BPF 				1
#define HPF 				2

#define FILTER_FREQ_COLUMN	24
#define FILTER_FREQ_PAGE	124

#define NONE				-1

#define EMPTY_STR_LINE 		"                              " // 30 empty spaces 8 x 30 = 240, which is the width of LCD

#define SECONDS_IN_MINUTE 	60

#define CHAR_WIDTH 			8
#define CHAR_HEIGHT 		16

#define TEMPO_COLUMN		24
#define TEMPO_PAGE			76
#define TEMPO_BAR_WIDTH		144

#define PITCH_COLUMN		24
#define PITCH_PAGE			28
#define PITCH_BAR_WIDTH		144

#define DISPLAY_WIDTH		240
#define DISPLAY_HEIGHT		320

void GUI_Init();
void GUI_SetPlaytime(int time, int maxTime);
void GUI_SetFileName(const char* filename);
void GUI_SetPlayStatus(int status);
void GUI_SetVolumeLevel(int volume, int maxVolume);
void GUI_SetFilterStatus(int status);
void GUI_SetFilterFreq(int lower, int upper);
void GUI_SetTempo(int tempo, int maxTempo);
void GUI_SetPitch(int pitch, int maxPitch);
void GUI_SetTitle();

#endif
