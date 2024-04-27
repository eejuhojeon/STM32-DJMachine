#include "lcd.h"
#include "gui.h"
#include "stdio.h"
#include "string.h"

void GUI_Init()
{
	GUI_SetPlaytime(-1, -1);
	GUI_SetFileName(NO_FILENAME);
	GUI_SetPlayStatus(NO_MUSIC);
	GUI_SetVolumeLevel(NONE, NONE);
	GUI_SetFilterStatus(NO_FILTER);
	GUI_SetFilterFreq(NONE, NONE);
	GUI_SetPitch(NONE, NONE);
	GUI_SetTempo(NONE, NONE);
	GUI_SetTitle();
}

void GUI_SetPlaytime(int time, int maxTime)
{
	if ((time < 0) || (maxTime < 0))
	{
		LCD_DrawString(PLAYTIME_COLUMN, PLAYTIME_PAGE, "Time: -- : -- / -- : --");
		return;
	}
	int time_minutes, time_seconds, maxTime_minutes, maxTime_seconds;
	time_minutes = time / SECONDS_IN_MINUTE;
	time_seconds = time % SECONDS_IN_MINUTE;
	maxTime_minutes = maxTime / SECONDS_IN_MINUTE;
	maxTime_seconds = maxTime % SECONDS_IN_MINUTE;

	char strTime[8];
	if (time_minutes > 9)
		if (time_seconds > 9)
			sprintf(strTime, "%d : %d", time_minutes, time_seconds);
		else
			sprintf(strTime, "%d : 0%d", time_minutes, time_seconds);
	else
		if (time_seconds > 9)
			sprintf(strTime, "0%d : %d", time_minutes, time_seconds);
		else
			sprintf(strTime, "0%d : 0%d", time_minutes, time_seconds);

	char strMaxTime[8];
	if (maxTime_minutes > 9)
		if (maxTime_seconds > 9)
			sprintf(strMaxTime, "%d : %d", maxTime_minutes, maxTime_seconds);
		else
			sprintf(strMaxTime, "%d : 0%d", maxTime_minutes, maxTime_seconds);
	else
		if (maxTime_seconds > 9)
			sprintf(strMaxTime, "0%d : %d", maxTime_minutes, maxTime_seconds);
		else
			sprintf(strMaxTime, "0%d : 0%d", maxTime_minutes, maxTime_seconds);

	LCD_DrawString(PLAYTIME_COLUMN, PLAYTIME_PAGE, "Time: ");
	LCD_DrawString(PLAYTIME_COLUMN + (CHAR_WIDTH * (6)), PLAYTIME_PAGE, strTime); // 6 for 6 chars in "Time: "
	LCD_DrawString(PLAYTIME_COLUMN + (CHAR_WIDTH * (6 + 7)), PLAYTIME_PAGE, " / "); // 7 for 7 chars in 'strTime'
	LCD_DrawString(PLAYTIME_COLUMN + (CHAR_WIDTH * (6 + 7 + 3)), PLAYTIME_PAGE, strMaxTime); // 3 for 3 chars in " / "
}

void GUI_SetFileName(const char* filename)
{
	LCD_DrawString(0, FILENAME_PAGE + CHAR_HEIGHT, EMPTY_STR_LINE);

	LCD_DrawString(FILENAME_COLUMN, FILENAME_PAGE, "Filename: ");
	LCD_DrawString(FILENAME_COLUMN, FILENAME_PAGE + CHAR_HEIGHT, filename); // File name in the next line
}

void GUI_SetPlayStatus(int status)
{

	uint16_t statColor;
	if (status < 0)
		statColor = RED;
	else if (status)
		statColor = GREEN;
	else
		statColor = YELLOW;

	LCD_DrawString(PLAY_STAT_COLUMN, PLAY_STAT_PAGE, "PLAY/PAUSE");
	// 10 for 10 chars in "Play/Pause", 1 for empty space
	LCD_OpenWindow(PLAY_STAT_COLUMN + (CHAR_WIDTH * (10 + 1)), PLAY_STAT_PAGE, CHAR_HEIGHT, CHAR_HEIGHT);
	LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, statColor);
}

void GUI_SetVolumeLevel(int volume, int maxVolume)
{
	if ((volume < 0) || (maxVolume < 0))
	{
		LCD_DrawString(PLAY_STAT_COLUMN + (CHAR_WIDTH * (10 + 1 + 4)), PLAY_STAT_PAGE, "Volume LV");
		LCD_DrawString(PLAY_STAT_COLUMN + (CHAR_WIDTH * (10 + 1 + 4)), PLAY_STAT_PAGE + CHAR_HEIGHT, "N/A%");
		return;
	}

	double d_volumeRatio = (double)volume / maxVolume * 100;
	int volumeRatio = (int)d_volumeRatio;

	char str_volumeRatio[5];
	if (volumeRatio > 99)
		sprintf(str_volumeRatio, "100%%");
	else if (volumeRatio > 9)
		sprintf(str_volumeRatio, "0%d%%", volumeRatio);
	else
		sprintf(str_volumeRatio, "00%d%%", volumeRatio);

	LCD_DrawString(PLAY_STAT_COLUMN + (CHAR_WIDTH * (10 + 1 + 4)), PLAY_STAT_PAGE, "Volume LV");
	LCD_DrawString(PLAY_STAT_COLUMN + (CHAR_WIDTH * (10 + 1 + 4)), PLAY_STAT_PAGE + CHAR_HEIGHT, str_volumeRatio);
}

void GUI_SetFilterStatus(int status)
{

	LCD_DrawString(FILTER_STAT_COLUMN, FILTER_STAT_PAGE + CHAR_HEIGHT, "Filter");
	LCD_DrawString(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1)), FILTER_STAT_PAGE, "LPF");
	LCD_DrawString(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 4)), FILTER_STAT_PAGE, "BPF");
	LCD_DrawString(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 8)), FILTER_STAT_PAGE, "HPF");

	switch (status)
	{
	case NO_FILTER:
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 4)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 8)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		break;
	case LPF:
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, GREEN);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 4)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 8)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		break;
	case BPF:
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 4)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, GREEN);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 8)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		break;
	case HPF:
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 4)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, BLACK);
		LCD_OpenWindow(FILTER_STAT_COLUMN + (CHAR_WIDTH * (6 + 1 + 8)), FILTER_STAT_PAGE + CHAR_HEIGHT, CHAR_HEIGHT, CHAR_HEIGHT);
		LCD_FillColor(CHAR_HEIGHT * CHAR_HEIGHT, GREEN);
		break;
	}
}

void GUI_SetFilterFreq(int lower, int upper)
{

	char strLower[9];
	if (lower > 9999)
		sprintf(strLower, "%d Hz", lower);
	else if (lower > 999)
		sprintf(strLower, "0%d Hz", lower);
	else if (lower > 99)
		sprintf(strLower, "00%d Hz", lower);
	else if (lower > 9)
		sprintf(strLower, "000%d Hz", lower);
	else if (lower > -1)
		sprintf(strLower, "0000%d Hz", lower);
	else
		sprintf(strLower, "----- Hz");

	char strUpper[9];
	if (upper > 9999)
		sprintf(strUpper, "%d Hz", upper);
	else if (upper > 999)
		sprintf(strUpper, "0%d Hz", upper);
	else if (upper > 99)
		sprintf(strUpper, "00%d Hz", upper);
	else if (upper > 9)
		sprintf(strUpper, "000%d Hz", upper);
	else if (upper > -1)
		sprintf(strUpper, "0000%d Hz", upper);
	else
		sprintf(strUpper, "----- Hz");



	LCD_DrawString(FILTER_FREQ_COLUMN, FILTER_FREQ_PAGE, "Lower Bound: ");
	LCD_DrawString(FILTER_FREQ_COLUMN + (CHAR_WIDTH * 13), FILTER_FREQ_PAGE, strLower);
	LCD_DrawString(FILTER_FREQ_COLUMN, FILTER_FREQ_PAGE + CHAR_HEIGHT, "Upper Bound: ");
	LCD_DrawString(FILTER_FREQ_COLUMN + (CHAR_WIDTH * 13), FILTER_FREQ_PAGE + CHAR_HEIGHT, strUpper);
}

void GUI_SetTempo(int tempo, int maxTempo)
{
	int tempoBarLen;

	if ((tempo < 0) || (maxTempo < 0))
	{
		tempoBarLen = 0;
		LCD_DrawString(TEMPO_COLUMN + (CHAR_WIDTH * (5 + 1)), TEMPO_PAGE + CHAR_HEIGHT, "N/A%");
	}
	else
	{
		double tempoRatio = (double)tempo / maxTempo;
		double d_tempoBarLen = tempoRatio * TEMPO_BAR_WIDTH;
		tempoBarLen = (int)d_tempoBarLen;

		char str_tempoRatio[5];
		int i_tempoRatio = tempoRatio * 100.0;
		if (i_tempoRatio > 99)
			sprintf(str_tempoRatio, "100%%");
		else if (i_tempoRatio > 9)
			sprintf(str_tempoRatio, "0%d%%", i_tempoRatio);
		else
			sprintf(str_tempoRatio, "00%d%%", i_tempoRatio);

		LCD_DrawString(TEMPO_COLUMN + (CHAR_WIDTH * (5 + 1)), TEMPO_PAGE + CHAR_HEIGHT, str_tempoRatio);
	}

	LCD_DrawString(TEMPO_COLUMN, TEMPO_PAGE, "Tempo");
	LCD_OpenWindow(TEMPO_COLUMN + (CHAR_WIDTH * (5 + 1)), TEMPO_PAGE, TEMPO_BAR_WIDTH, CHAR_HEIGHT);
	LCD_FillColor(TEMPO_BAR_WIDTH * CHAR_HEIGHT, BLACK);
	LCD_OpenWindow(TEMPO_COLUMN + (CHAR_WIDTH * (5 + 1)), TEMPO_PAGE, tempoBarLen, CHAR_HEIGHT);
	LCD_FillColor(tempoBarLen * CHAR_HEIGHT, YELLOW);
}

void GUI_SetPitch(int pitch, int maxPitch)
{
	int pitchBarLen;

	if ((pitch < 0) || (maxPitch < 0))
	{
		pitchBarLen = 0;
		LCD_DrawString(PITCH_COLUMN + (CHAR_WIDTH * (5 + 1)), PITCH_PAGE + CHAR_HEIGHT, "N/A%");
	}
	else
	{
		double pitchRatio = (double)pitch / maxPitch;
		double d_pitchBarLen = pitchRatio * PITCH_BAR_WIDTH;
		pitchBarLen = (int)d_pitchBarLen;

		char str_pitchRatio[5];
		int i_pitchRatio = pitchRatio * 100.0;
		if (i_pitchRatio > 99)
			sprintf(str_pitchRatio, "100%%");
		else if (i_pitchRatio > 9)
			sprintf(str_pitchRatio, "0%d%%", i_pitchRatio);
		else
			sprintf(str_pitchRatio, "00%d%%", i_pitchRatio);

		LCD_DrawString(PITCH_COLUMN + (CHAR_WIDTH * (5 + 1)), PITCH_PAGE + CHAR_HEIGHT, str_pitchRatio);
	}

	LCD_DrawString(PITCH_COLUMN, PITCH_PAGE, "Pitch");
	LCD_OpenWindow(PITCH_COLUMN + (CHAR_WIDTH * (5 + 1)), PITCH_PAGE, PITCH_BAR_WIDTH, CHAR_HEIGHT);
	LCD_FillColor(PITCH_BAR_WIDTH * CHAR_HEIGHT, BLACK);
	LCD_OpenWindow(PITCH_COLUMN + (CHAR_WIDTH * (5 + 1)), PITCH_PAGE, pitchBarLen, CHAR_HEIGHT);
	LCD_FillColor(pitchBarLen * CHAR_HEIGHT, YELLOW);
}

void GUI_SetTitle()
{
	LCD_DrawString(CHAR_WIDTH * 6,4,"[CONTROL DASHBOARD]");
}









