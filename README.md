# STM32-DJMachine
STM32 based DJ machine that can play music with various real-time sound effects.


Watch the demo video for every function available.
[![DEMO VIDEO](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_HERE/0.jpg)](https://www.youtube.com/watch?v=ZRtjr05uK14)

You may check a pdf file under /pdf for hardware connections.

Credits to HAN, Sungje and PARK, Gunwoo who were the team members at the time.

List of Functions:
Play an audio file.
o WAV files are read from micro-SD cards by SDIO interface and FATFS from micro-SD Module
o Audio data are modified by Digital Audio Processing on the STM32 processor.
o Internal DAC reads processed audio data from memory by using Direct Memory Access (DMA)
o DAC is controlled by internal timers, and output signal amplified by LM386 amplifier
o Amplified audio signal is sent to external speaker to play the audio.

Real-time digital signal processing on STM32 processor
o Audio tempo adjustment from x0.5 ~ x1.5
o Low pass filter / High pass filter application with adjustable cutoff frequency (64 – 16384Hz)
o Real-time audio mixing of four inherent sound effects.
o Play & Pause, audio time control, selecting audio file to play.

Device control by external physical inputs
o Audio sound volume control by adjusting variable register connected to LM386 amplifier.
o Audio tempo control by potentiometer using ADC.
o Filter application, sound effects, play/pause, audio file selection by GPIO interface
o Audio time control can be done by buttons communicating with STM32 by GPIO interface.

Audio-play Dashboard graphic user interface by built-in LCD and FSMC interface
o Current play status including play/pause status, tempo level, filter status, filter cutoff frequency, sound effect play status, file selection, audio play time are shown on the LCD.