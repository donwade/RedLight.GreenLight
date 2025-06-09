//#define WAV_FILE_NAME "/resources_speak_sd.wav"
//#define WAV_FILE_NAME "/400.wav"
#define WAV_FILE_NAME "/440-1c-44k-1s.wav"

TaskHandle_t speak_file(char *waveFilename = WAV_FILE_NAME);
void setup_voice();



