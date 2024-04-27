/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "lcd.h"
#include "gui.h"
#include "mariosfx.h"
#include <math.h>

struct WavHeader
{
	/* RIFF */
	char     riffChunkID[4];		// "RIFF"
	uint32_t riffChunkSize;			// (Size of whole file - 8) bytes, -8 for -(riffChunkID size + riffChunkSize size)
	char     riffFormat[4];			// "WAVE" should be in there for WAV file

	/* FORMAT */
	char     fmtChunkID[4];			// "FMT " *SPACE MUST BE INCLUDED IN THE END*
	uint32_t fmtChunkSize;			// (Size of FMT chunk - 8) bytes, -8 for (fmtChunkID size + fmtChunkSize size), should be 16
	uint16_t audioFmt;				// for PCM audio, 1, which is for WAV file
	uint16_t numChannels;			// number of channels of the audio
	uint32_t sampleRate;			// number of samples per a second, sampling frequency in Hz
	uint32_t byteRate;				// number of bytes for play per a second, (bitsPerSample * numChannels * sampleRate) bytes
	uint16_t blockAlign;			// size of samples per one set of channel play
	uint16_t bitsPerSample;			// sample size in bits

	/* DATA */
	char     dataChunkID[4];		// chunk ID tells computer the data chunk begins from here, "DATA"
	uint32_t dataChunkSize;			// size of actual audio wave data
};
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BUFFER_SIZE 		512

#define LCD_ERR_PAGE 		300
#define LCD_ERR_COLUMN 		0

#define PCM_AUDIO_FMT 		1

#define SYS_CLOCK 			72000000

/* SYSTEM ERROR CODES */
#define MOUNT_FAIL 			1
#define OPENDIR_FAIL 		2
#define NO_WAV_FOUND 		3
#define FILE_OPEN_FAIL 		4
#define HEADER_READ_FAIL 	5
#define RIFF_NOT_FOUND 		6
#define WRONG_AUDIOFMT 		7
#define DATA_NOT_FOUND 		8

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac_ch1;

SD_HandleTypeDef hsd;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim6;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

/* KEYPAD */
GPIO_InitTypeDef 	GPIO_InitStructPrivate = {0};
uint8_t 			keyPressed = 0;
uint32_t 			currentTick;
uint32_t 			previousTick;


/* DMA */
uint8_t 			flag_dma_done;
static uint16_t 	dmaBuffer[2][BUFFER_SIZE];
static uint8_t 		dmaBufferIndexToUse = 0;


/* FILE I/O */
static uint8_t 		fileBuffer[BUFFER_SIZE];
UINT 				bytesRead;
uint32_t 			bytesRemained = 0;
uint32_t 			bytesRemainedMax;
FIL 				audioFile;
FIL 				sfxFile;
FATFS 				fatfs = {0};
FRESULT 			res;
DIR					dir;
FILINFO				fno;
char* 				filename;
uint32_t 			blockSize;


/* MAIN AUDIO CONTROL */
// Play & Pause //
uint8_t 			isMusicPlaying;

// Tempo Control //
uint8_t				tempoLevel = 5;
uint8_t				tempoLevelMax = 10;
float				tempoMultipliers[11] = {0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f};
uint32_t 			originalSampleRate;

// Time Control //
uint8_t 			isMoveForward = 0;
uint8_t 			isMoveBackward = 0;
int16_t				timeShift = 0;
uint16_t			recentPin = 0;
uint32_t			audioLength;
uint32_t			audioCurrentTime;

// Filter Control //
uint8_t				isLPF = 0;
uint8_t				isHPF = 0;
uint8_t				filterFrequencyLevel = 7;
uint8_t				formerFreq = 7;
const uint8_t		filterFrequencyLevelMax = 15;
uint32_t			filterFrequencies[16] = {25, 50, 75, 100,
											 250, 500, 750, 1000,
											 2500, 5000, 7500, 10000,
											 12500, 15000, 17500, 20000};
double				alpha;
double				wc;
double				tau;

// SFX Control //
uint8_t 	isSFX = 0; // 0: OFF, 1: coin, 2: fireball, 3: kick, 4: stomp
uint8_t 	sfxBuffer[50000];
uint32_t 	sfxCounter = 0;
uint32_t 	sfxCounterMax;
uint8_t*	sfxPointer;

// Pitch Shift // jj
float Rd_P;
float Shift;
float MAX_SHIFT = 2.0f;
float MIN_SHIFT = 0.5;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_DMA_Init(void);
static void MX_FSMC_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
void SetSampleRate(uint32_t freq);
void ThrowSystemError(uint8_t errorCode);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SDIO_SD_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  MX_FATFS_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  /* Peripheral Initialization */
  LCD_INIT();
  HAL_TIM_Base_Start(&htim6);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_ADCEx_Calibration_Start(&hadc1);



  /* SD CARD Mount */
  res = f_mount(&fatfs, "", 1);
  if (res != FR_OK)
	  ThrowSystemError(MOUNT_FAIL);



  /* OPEN DIRECTORY */
  res = f_opendir(&dir, "");
  if (res != FR_OK)
	  ThrowSystemError(OPENDIR_FAIL);



  /* FIND WAV FILENAME */
  while(1)
  {
	  res = f_readdir(&dir, &fno);
	  if (res != FR_OK || fno.fname[0] == 0)
		  ThrowSystemError(NO_WAV_FOUND);

	  filename = fno.fname;
	  if ((strstr(filename, ".WAV") != 0) && (strcmp(filename, "SFX1.WAV") != 0))
		  break;
  }



  /* OPEN WAV FILE */
  res = f_open(&audioFile, filename, FA_READ);
  if (res != FR_OK)
	  ThrowSystemError(FILE_OPEN_FAIL);



  /* READ WAV HEADER */
  struct WavHeader header;
  res = f_read(&audioFile, &header, sizeof(struct WavHeader), &bytesRead);
  if (res != FR_OK)
	  ThrowSystemError(HEADER_READ_FAIL);



  /* CHECK WAV HEADER VALIDITY */
  if (strncmp(header.riffChunkID, "RIFF", 4) != 0) 	// 1. Check whether the header begins with "RIFF"
	  ThrowSystemError(RIFF_NOT_FOUND);

  if (header.audioFmt != PCM_AUDIO_FMT) 			// 2. Check whether the audio format is PCM
	  ThrowSystemError(WRONG_AUDIOFMT);

  if (strncmp(header.dataChunkID, "data", 4) != 0) 	// 3. check whether the data section begins with "data"
	  ThrowSystemError(DATA_NOT_FOUND);



  /* CALCULATE AUDIO LENGTH */
  audioLength = header.dataChunkSize / header.byteRate;
  audioCurrentTime = 0;

  /* Initialize Pitch Variables */
    Rd_P = 0.0;
    Shift = 1.0;

  /* INITIALIZE GUI */
  int32_t GUI_updateTick = HAL_GetTick();
  int32_t GUI_previousTick = HAL_GetTick();
  int32_t GUI_updatePeriod = 200;
  GUI_Init();
  GUI_SetPlaytime(audioCurrentTime, audioLength);
  GUI_SetFileName(filename);
  GUI_SetPlayStatus(MUSIC_PLAYING);
  GUI_SetTempo(tempoLevel, tempoLevelMax);
  GUI_SetPitch(Shift, MAX_SHIFT);


  /* SET AUDIO PLAYING FREQUENCY */
  SetSampleRate(header.sampleRate);
  originalSampleRate = header.sampleRate;



  /* CALCULATE CONSTANTS FOR FILTERS */
  wc = filterFrequencies[filterFrequencyLevel] * 2.0 * 3.141592; 	// 1. wc is angular cutting frequency of the filter
  tau = 1.0 / wc;													// 2. tau is time constant
  alpha = tau / (tau + (1.0 / originalSampleRate));					// 3. alpha is (tau / (tau + sampling time))
  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	 	 	// (+) Filter Idea From: https://coding-robot.tistory.com/15


  /* INITIALIZE FILTER BUFFER */
  int32_t lpfBuffer = 0; 		// lpfBuffer contains result before one time.
  	  	  	  	  	  	  	  	// result[t] = alpha * result[t-1] + (1-alpha) * data[t]
  int32_t hpfBuffer[2] = {0,0};	// [0]: Result before one time [1]: Data before one time
  	  	  	  	  	  	  	  	// result[t] = alpha * (result[t-1] + data[t] - data[t-1])



  /* PLAY MUSIC */
  const uint8_t numChannels = (uint8_t)(header.numChannels);
  const uint8_t bytesPerSample = (uint8_t)(header.bitsPerSample / 8);

  // 0. Initialize variables
  flag_dma_done = 1; 					// DMA sets this flag to 1 if DMA usage is done
  dmaBufferIndexToUse = 0; 				// DMA buffer index for writing fileBuffer data. The other index DMA buffer is used for DMA transmission
  bytesRemained = header.dataChunkSize; // Remained size of data to play (in bytes)
  bytesRemainedMax = bytesRemained; 	// Total size of data section, used for preventing rewind below the point data begins
  isMusicPlaying = 1; 					// Music is now playing

  while(bytesRemained > 0)
  {
	  // 1. Calculate the size of block to load (in bytes)
	  blockSize = (bytesRemained < BUFFER_SIZE) ? bytesRemained : BUFFER_SIZE;

	  // 2. Load data to file buffer
	  res = f_read(&audioFile, fileBuffer, blockSize, &bytesRead);
	  if ((res != FR_OK) || (bytesRead == 0))
	  {
		  // YOU FAILED TO READ? TRY UNTIL IT SUCCESS!
		  while(res != FR_OK)
		  {
		  if (bytesRemained < BUFFER_SIZE*2)
			break;
		  DWORD audioFilePtr = f_tell(&audioFile);
		  f_close(&audioFile);
		  f_open(&audioFile, filename, FA_READ);
		  f_lseek(&audioFile, audioFilePtr);
		  res = f_read(&audioFile, fileBuffer, blockSize, &bytesRead);
		  }
	  }

	  // 3. Audio processing preparation
	  uint16_t numSamples = bytesRead / bytesPerSample / numChannels; 	// Total number of samples to process
	  int16_t* from = (int16_t*)fileBuffer; 							// from points to file buffer
	  uint16_t* to = dmaBuffer[dmaBufferIndexToUse]; 					// to points to dma buffer

	  // 4. Send audio data from file buffer to dma buffer
	  if (bytesPerSample == 1) // Case audio is 8-Bit
	  {
		  uint8_t* from8Bit = (uint8_t*)from; 	// from8Bit is a pointer to file buffer which reads 8-bit unsigned integer if *from8Bit is called.
		  for (int i = 0; i < numSamples; ++i)
		  {
			  uint16_t temp;
			  temp = *from8Bit; 		// (1) Get 8-bit unsigned data (The data of 8-bit WAV is unsigned)
			  temp = temp << 4; 		// (2) Change the data to 12-bit unsigned value by left arithmetic shift of 4
			  from8Bit += numChannels; 	// (3) Just the get the value of the first channel; skip the stereo channel data

			  // Pitch shift
			  Rd_P += Shift;
			  if (roundf(Rd_P) >= BUFFER_SIZE) Rd_P = 0.0f;


			  int RdPtr_Int = roundf(Rd_P);

			  int16_t Rd0 = ((*(fileBuffer + RdPtr_Int))<<4) - 2047;

			  temp = (Rd0) + 2047;




			  // Filter Processing //
			  int16_t tempSigned = (int16_t)temp; 									// Convert 12-bit data above in signed form, by giving -2047 offset
			  tempSigned -= 2047;


			  // Overwrite data by LPF value if LPF is on
			  if (isLPF)
			  {
				  double tempLPF = lpfBuffer * alpha + (1 - alpha) * tempSigned; 		// LPF calculation
				  lpfBuffer = (int32_t)tempLPF; 										// LPF buffer allocation

				  tempLPF += 2047;
				  temp = (uint16_t)tempLPF;
			  }
			  // Overwrite data by HPF value if HPF is on
			  else if (isHPF)
			  {
				  double tempHPF = alpha * (hpfBuffer[0] + tempSigned - hpfBuffer[1]);	// HPF calculation
				  hpfBuffer[0] = (int32_t)tempHPF; 										// HPF buffer allocation
				  hpfBuffer[1] = tempSigned;
				  tempHPF += 2047;
				  temp = (uint16_t)tempHPF;
			  }
			  // if sfx is on
			  if (isSFX)
			  {
				  if (sfxCounter > sfxCounterMax)
				  {
					  isSFX = 0;
					  sfxCounter = 0;
					  sfxPointer = 0;
				  }
				  else
				  {

					  int32_t sfxData = sfxPointer[sfxCounter];
					  sfxData = sfxData << 8;
					  sfxData = (tempSigned + sfxData);
					  sfxData += 32767;
					  sfxData = sfxData >> 4;
					  sfxData = sfxData & 0xfff;
					  sfxCounter++;

					  temp = (uint16_t)sfxData;

				  }
			  }

			  *to = temp; 				// (4) Send the modified 12-bit data (in (1) - (3)) to DMA buffer
			  to++;						// (5) 'to' now points to next element of DMA buffer
		  }
	  }
	  else // Case audio is 16-Bit
	  {
		  for (int i = 0; i < numSamples; ++i)
		  {
			 int32_t temp = *(from); 	// (1) Get 16 bit *signed* data (Range: -32767 ~ 32768) (The data of non 8-bit WAV is signed)
			 temp += 32767; 			// (2) Change data to 16 bit *unsigned* value by giving offset of +32767
			 temp = temp >> 4; 			// (3) Change data to *12-bit* unsigned value by removing 4 least significant bits
			 temp = temp & 0xfff; 		// (4) Just for insurance, set all the bits except for first 12-bit to be 0
			 from += numChannels; 		// (5) Just get the value of first channel; skip the stereo channel data


			// Filter System on 16-Bit is now on the progress //
			int32_t tempSigned = (int32_t)temp;
			tempSigned -= 2047;
			double tempLPF = lpfBuffer * alpha + (1 - alpha) * tempSigned;
			lpfBuffer = (int32_t)tempLPF;
			 if (isLPF)
			 {
				 tempLPF += 2047;
				 temp = (uint16_t)tempLPF;
			 }

			 *to = (uint16_t)temp; 		// (6) Send the modified 12-bit data (in (1) - (5)) to DMA buffer
			 to++;						// (7) 'to' now points to next element of DMA buffer
		  }
	  }
	  // 5. Wait until DMA, which is started in the previous loop, is done (When DMA is done, the DMA interrupt function changes flag to 0)
	  while(flag_dma_done == 0){};

	  // 6. If previous DMA is done, stop the DMA function for a new start
	  HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	  flag_dma_done = 0;


	  // 7. If user paused music, wait until user play the music (isMusicPlaying is controlled by interrupt function)
	  while(!isMusicPlaying){};

	  // 8. Start DMA for the DMA buffer where new audio data is loaded in this loop
	  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) dmaBuffer[dmaBufferIndexToUse], numSamples, DAC_ALIGN_12B_R);

	  // 9. Flip the dmaBufferIndexToUse (1 to 0 OR 0 to 1)
	  //    DMA Buffer section being used for DMA started above will not be used for loading data in the next loop.
	  dmaBufferIndexToUse = !dmaBufferIndexToUse;

	  // 10. As block size of data is read, reduce the bytesRemained
	  bytesRemained = bytesRemained - blockSize;

	  // 11. If user decided to move forward or backward, shift the file pointer
	  // isMoveForward and isMoveBackward is toggled by interrupt function
	  if (isMoveForward)
	  {
		  isMoveForward = 0;
		  bytesRemained -= 1024000;
	      f_lseek(&audioFile, f_tell(&audioFile) + 1024000); // 1024000 is just a notably big value to check time shift.
	  }
	  if (isMoveBackward)
	  {
		  isMoveBackward = 0;
		  bytesRemained += 1024000;
		  if (bytesRemained > bytesRemainedMax) // Prevent time backward shift below the point the music data begins
		  {
			  bytesRemained = bytesRemainedMax;
			  f_lseek(&audioFile, 1);
		  }
		  else
			  f_lseek(&audioFile, f_tell(&audioFile) - 1024000);
	  }
	  // Rotary Encoder Time Shift
	  if (timeShift != 0) {
		  bytesRemained -= (timeShift << 13);
		  if (bytesRemained > bytesRemainedMax) {
			  bytesRemained = bytesRemainedMax;
			  f_lseek(&audioFile, 1);
		  }
		  else
			  f_lseek(&audioFile, f_tell(&audioFile) + (timeShift << 13));
		  if (timeShift > 0)
			  timeShift--;
		  else
			  timeShift++;
	  }


	  // 12. Update GUI periodically
	  GUI_updateTick = HAL_GetTick();
	  if (GUI_updateTick - GUI_previousTick > GUI_updatePeriod) // One update per 0.2 second
	  {
		  // Put any non-interrupt GUI functions inside here //
		  audioCurrentTime = (header.dataChunkSize - bytesRemained) / header.byteRate;
		  GUI_SetPlaytime(audioCurrentTime, audioLength);
		  GUI_previousTick = GUI_updateTick;
		  HAL_ADC_Start(&hadc1);
		  HAL_ADC_PollForConversion(&hadc1, 100);
		  uint32_t frequencyCheck = HAL_ADC_GetValue(&hadc1);
		  filterFrequencyLevel = frequencyCheck >> 8;
		  if (formerFreq != filterFrequencyLevel) {
			  wc = filterFrequencies[filterFrequencyLevel] * 2.0 * 3.141592;
		  	  tau = 1.0 / wc;
		  	  alpha = tau / (tau + (1.0/originalSampleRate));
		  	  formerFreq = filterFrequencyLevel;
		  }
		  if (isLPF)
			  GUI_SetFilterFreq(NONE, filterFrequencies[filterFrequencyLevel]);
		  else if (isHPF)
			  GUI_SetFilterFreq(filterFrequencies[filterFrequencyLevel], NONE);
	  }


	  // 13. Check for loop's end condition, when the music is all played
	  if (bytesRemained < BUFFER_SIZE)
		  break;
  }

  /* WAIT FOR LAST DMA */
  while(flag_dma_done == 0){};
  HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);

  /* JUST FOR DEBUG */
  LCD_DrawString(0, 250, "DONE!");



  // FINISH FILESYSTEM
  f_close(&audioFile);
  f_closedir(&dir);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 16;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 11999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ITR1;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, KEYPAD_IN1_Pin|KEYPAD_IN2_Pin|KEYPAD_IN3_Pin|KEYPAD_IN4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : KEYPAD_IN1_Pin KEYPAD_IN2_Pin KEYPAD_IN3_Pin KEYPAD_IN4_Pin */
  GPIO_InitStruct.Pin = KEYPAD_IN1_Pin|KEYPAD_IN2_Pin|KEYPAD_IN3_Pin|KEYPAD_IN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : K1_Pin */
  GPIO_InitStruct.Pin = K1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(K1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KEYPAD_OUT3_Pin KEYPAD_OUT4_Pin ROTEN_A_Pin KEYPAD_OUT1_Pin
                           KEYPAD_OUT2_Pin */
  GPIO_InitStruct.Pin = KEYPAD_OUT3_Pin|KEYPAD_OUT4_Pin|ROTEN_A_Pin|KEYPAD_OUT1_Pin
                          |KEYPAD_OUT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ROTEN_B_Pin */
  GPIO_InitStruct.Pin = ROTEN_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ROTEN_B_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Disconnect NADV
  */

  __HAL_AFIO_FSMCNADV_DISCONNECTED();

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */

/* SET SAMPLE RATE */
void SetSampleRate(uint32_t freq)
{
	uint16_t period = (SYS_CLOCK / freq) - 1;
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 0;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = period;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim6);
}

/* JUST FUNCTION TO CHECK ERROR CODE */
void ThrowSystemError(uint8_t errorCode)
{
	char errorMessage[23];
	sprintf(errorMessage, "SYSTEM ERROR(CODE: %d)", errorCode);
	LCD_INIT();
	LCD_DrawString(0, 300, errorMessage);

	if (errorCode > 4)
		f_close(&audioFile);

	if (errorCode > 2)
		f_closedir(&dir);

	while(1){};
}

/* FUNCTION CALLED WHEN DMA IS FINISHED */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
	flag_dma_done = 1;
}


/* EXTERNAL CONTROL BY INTERRUPT */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// IS ROTARY ENCODER INPUT
	if (GPIO_Pin == GPIO_PIN_12) {
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET)
		{
			if (timeShift <= 0)
				timeShift -= 1;
		}
		else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET)
		{
			if (timeShift >= 0)
				timeShift += 1;
		}
		return;
	}
	// IS KEYPAD INPUT
	/* KEYPAD CONNECTION:
	 * 1, 2, 3, 4 -> KEYPAD_IN1, IN2, IN3, IN4
	 * 5, 6, 7, 8 -> KEYPAD_OUT1, OUT2, OUT3, OUT4
	 */
	currentTick = HAL_GetTick();
	if (currentTick - previousTick < 200)
		return;

    GPIO_InitStructPrivate.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructPrivate);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    if(GPIO_Pin == GPIO_PIN_8 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8))
    {
    	/* KEYPAD K1 */
    	// Play/Pause Button
      keyPressed = 1;
      LCD_DrawChar(100, 250, 'A');
      isMusicPlaying = !isMusicPlaying;
      GUI_SetPlayStatus(isMusicPlaying);
    }
    else if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9))
    {
      keyPressed = 2;
      LCD_DrawChar(100, 250, 'B');
    }
    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10))
    {
    	/* KEYPAD K3 */
    	// Tempo Up Button
      LCD_DrawChar(100, 250, 'C');
      if (tempoLevel == tempoLevelMax)
    	  return;

      tempoLevel++;
      GUI_SetTempo(tempoLevel, tempoLevelMax);
      SetSampleRate(tempoMultipliers[tempoLevel] * originalSampleRate);
    }
    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11))
    {
    	/* KEYPAD K4 */
    	// Tempo Down Button
      LCD_DrawChar(100, 250, 'D');
      if (tempoLevel == 0)
    	  return;

      tempoLevel--;
      GUI_SetTempo(tempoLevel, tempoLevelMax);
      SetSampleRate(tempoMultipliers[tempoLevel] * originalSampleRate);
    }

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    if(GPIO_Pin == GPIO_PIN_8 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8))
    {
      keyPressed = 5;
      LCD_DrawChar(100, 250, 'E');
      isMoveForward = 1;
    }
    else if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9))
    {
      keyPressed = 6;
      LCD_DrawChar(100, 250, 'F');
      isMoveBackward = 1;
    }
    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10))
    {
    	/* KEYPAD K7 */
    	// High Pass Filter Toggle Button
      keyPressed = 7;
      LCD_DrawChar(100, 250, 'G');
      isHPF = !isHPF;
      if (isHPF)
      {
    	  if (isLPF) isLPF = 0;
    	  GUI_SetFilterStatus(HPF);
    	  GUI_SetFilterFreq(NONE, filterFrequencies[filterFrequencyLevel]);
      }
      else
      {
    	  GUI_SetFilterStatus(-1);
    	  GUI_SetFilterFreq(NONE, NONE);
      }
    }
    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11))
    {
    	/* KEYPAD K8 */
    	// Low Pass Filter Toggle Button
      keyPressed = 8;
      LCD_DrawChar(100, 250, 'H');
      isLPF = !isLPF;
      if (isLPF)
      {
    	  if (isHPF) isHPF = 0;
    	  GUI_SetFilterStatus(LPF);
      	  GUI_SetFilterFreq(filterFrequencies[filterFrequencyLevel], NONE);
      }
      else
      {
    	  GUI_SetFilterStatus(-1);
    	  GUI_SetFilterFreq(NONE, NONE);
      }
    }

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    if(GPIO_Pin == GPIO_PIN_8 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8))
    {
      keyPressed = 9;
      LCD_DrawChar(100, 250, 'I');
      if (isSFX)
    	  return;
      isSFX = 3;
      sfxCounter = 0;
      sfxCounterMax = sfxLength[isSFX-1];
      sfxPointer = kickSFX;
    }
    else if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9))
    {
      keyPressed = 10;
      LCD_DrawChar(100, 250, 'J');
      if (isSFX)
    	  return;
      isSFX = 4;
      sfxCounter = 0;
      sfxCounterMax = sfxLength[isSFX-1];
      sfxPointer = stompSFX;
    }
    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10))
    {
      keyPressed = 11; //K11 Pressed Pitch Down
      LCD_DrawChar(100, 250, 'K');

      if(Shift <= MIN_SHIFT){
    	  return;
      }


      Shift -= 0.1;

      GUI_SetPitch(Shift*100, MAX_SHIFT*100);
    }
    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11))
    {
    	/* KEYPAD K12 */
    	// Filter Frequency Up Button
      keyPressed = 12;
      LCD_DrawChar(100, 250, 'L');
      if (!isLPF && !isHPF)
    	  return;
      if (filterFrequencyLevel)
      {
    	  filterFrequencyLevel--;
		  wc = filterFrequencies[filterFrequencyLevel] * 2.0 * 3.141592;
		  tau = 1.0 / wc;
		  alpha = tau / (tau + (1.0/originalSampleRate));
		  if (isLPF)
			  GUI_SetFilterFreq(filterFrequencies[filterFrequencyLevel], NONE);
		  else
			  GUI_SetFilterFreq(NONE, filterFrequencies[filterFrequencyLevel]);
      }
    }

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
    if(GPIO_Pin == GPIO_PIN_8 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8))
    {
      keyPressed = 13;
      LCD_DrawChar(100, 250, 'M');
      if (isSFX)
    	  return;
      isSFX = 1;
      sfxCounter = 0;
      sfxCounterMax = sfxLength[isSFX-1];
      sfxPointer = coinSFX;
    }
    else if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9))
    {
      keyPressed = 14;
      LCD_DrawChar(100, 250, 'N');
      if (isSFX)
    	  return;
      isSFX = 2;
      sfxCounter = 0;
      sfxCounterMax = sfxLength[isSFX-1];
      sfxPointer = fireballSFX;
    }
    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10))
    {
    	/* KEYPAD K15 */
    	// Pitch shift UP
      keyPressed = 15;
      LCD_DrawChar(100, 250, 'O');
      if(Shift >= MAX_SHIFT){
          	  return;
      }

      Shift += 0.1;


      GUI_SetPitch(Shift*100, MAX_SHIFT*100);







    }
    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11))
    {
    	/* KEYPAD K16 */
    	// Filter Frequency Down Button
      keyPressed = 16;
      LCD_DrawChar(100, 250, 'P');
      if (!isLPF && !isHPF)
    	  return;
      if (filterFrequencyLevel < filterFrequencyLevelMax)
      {
    	  filterFrequencyLevel++;
		  wc = filterFrequencies[filterFrequencyLevel] * 2.0 * 3.141592;
		  tau = 1.0 / wc;
		  alpha = tau / (tau + (1.0/originalSampleRate));
		  if (isLPF)
			  GUI_SetFilterFreq(filterFrequencies[filterFrequencyLevel], NONE);
		  else
			  GUI_SetFilterFreq(NONE, filterFrequencies[filterFrequencyLevel]);
      }
    }

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);

    GPIO_InitStructPrivate.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStructPrivate.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructPrivate);
    previousTick = currentTick;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
