/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 *
 * Play wav file from SD card
 *
 ****************************************************************
 * Prepare a file named "resources_speak_sd.wav" in the root of your SD card
 * Wav file format : 44100Hz 16bit mono
 *
 * Download it from https://github.com/m5stack/M5Core2/wiki
 * or you can make it.
 *   Example of conversion using ffmpeg
 *   ffmpeg -i input.wav -vn -ac 1 -ar 44100 -acodec pcm_s16le -f wav resources_speak_sd.wav
 ****************************************************************
*/
#include <M5Core2.h>
#include "wav2spkr.h"

extern "C" {
#include "watchdogs.h"
}

#include "Speaker.h"
Speaker mySpeaker;

static File  	hFile;
static uint8_t 	spkBuffer[1024 * 8];

static bool bValidWavFile;
static bool play_loop=true;
static bool bIsStreaming = false;


struct __attribute__((packed)) wav_header_t {
    char RIFF[4];
    uint32_t chunk_size;
    char WAVEfmt[8];
    uint32_t fmt_chunk_size;
    uint16_t audiofmt;
    uint16_t channel;
    uint32_t sample_rate;
    uint32_t byte_per_sec;
    uint16_t block_size;
    uint16_t bit_per_sample;
};
struct __attribute__((packed)) sub_chunk_t {
    char identifier[4];
    uint32_t chunk_size;
};
//-------------------------------------------------------------
static void dumper (char *comment, void *from, uint32_t len)
{ 
	uint8_t *src = (uint8_t *) from;

	Serial.printf("[%s] ", comment);
	for (int i = 0; i < len; i++) Serial.printf("%02X ", src[i]);
	Serial.printf("    ");
	for (int i = 0; i < len; i++) Serial.printf("%c", src[i]);
	Serial.printf("\n");
}
//-------------------------------------------------------------
// Validate file type
// Seek to the beginning of the audio data

bool prepareFile(void) 
{
	uint32_t bread;
    wav_header_t wheader;
	
    hFile.seek(0);

    // Read header
    bread  = hFile.read ((uint8_t*)&wheader, sizeof(wheader));
	Serial.printf("bread = %d\n", bread);
	
	if (bread != sizeof(wheader)) 
	{
        return false;
    }

    // Check format
	//wheader.sample_rate != 44100 || 
	
    // 4410 16bit mono linear PCM
    /*
    if (memcmp(wheader.RIFF, "RIFF", 4) )
    {
	//wheader.audiofmt != 1 ||
	//wheader.bit_per_sample != 16 ||
	//wheader.channel != 1
    */
    
	dumper("RIFF", wheader.RIFF, sizeof(wheader.RIFF));
	dumper("WAVEfmt", wheader.WAVEfmt, sizeof(wheader.WAVEfmt));
	
	Serial.printf("wtf = %d\n", memcmp(wheader.RIFF, "RIFF", 4)); 

	
	ABORT_ON_FAIL(memcmp(wheader.RIFF,"RIFF",4), 0);
	ABORT_ON_FAIL(memcmp(wheader.WAVEfmt, "WAVEfmt", 7), 0);
 		
	if (memcmp(wheader.RIFF, "RIFF", 4) ||
		memcmp(wheader.WAVEfmt, "WAVEfmt", 7) 
		)
		{
	    	Serial.printf("\n*** %s Illegal format\n", __FUNCTION__);
	    	return false;
    	}

    // Find data chunk
    sub_chunk_t aChunk;
	
    while (true) {
        if (hFile.read ((uint8_t*)&aChunk, sizeof(aChunk)) != sizeof(aChunk)) {
			Serial.println("*** failed to find data chunk!!!\n");
            return false;
        }
		
        if (memcmp(aChunk.identifier, "data", 4) == 0) {
            break;
        }
		
        hFile.seek(hFile.position() + aChunk.chunk_size);
    }
    return true;
}

//-------------------------------------------------------------

// Seek to the beginning of the audio data
unsigned int get_rate(void)
{
    hFile.seek(0);

    // Read header
    wav_header_t wheader{};
    if (hFile.read((uint8_t*)&wheader, sizeof(wheader)) != sizeof(wheader)) {
        return false;
    }

#if 1
    printf( "\nRIFF           : %.4s\n" , wheader.RIFF          );
    printf( "chunk_size     : %d\n"   , wheader.chunk_size    );
    printf( "WAVEfmt        : %.8s\n" , wheader.WAVEfmt       );
    printf( "fmt_chunk_size : %d\n"   , wheader.fmt_chunk_size);
    printf( "audiofmt       : %d\n"   , wheader.audiofmt      );
    printf( "channel        : %d\n"   , wheader.channel       );
    printf( "sample_rate    : %d\n"   , wheader.sample_rate   );
    printf( "byte_per_sec   : %d\n"   , wheader.byte_per_sec  );
    printf( "block_size     : %d\n"   , wheader.block_size    );
    printf( "bit_per_sample : %d\n\n"   , wheader.bit_per_sample);
#endif
	return wheader.sample_rate;
}

//-------------------------------------------------------------

void  streamVoice(void *passIn) 
{
	uint32_t blk_ctr = 0;
	watchdog_prefix();
	while (true)
	{
	
		watchdog_kick();
		
	    // Read more bytes from file and play
	    auto numBytesRead = hFile.read(spkBuffer, sizeof(spkBuffer));

	    if (numBytesRead <= 0)
	    {
	    	//hFile.close();
			Serial.printf("TODO: WAV end %d blocks xfer' %d\n", blk_ctr);
			bIsStreaming = false;

			while(true)
			{
				watchdog_kick();
				delay(1000); // somebody kill me!!!
			}
	    }
		
	    // I2S write is blocking until the end of write
		blk_ctr++;
		mySpeaker.PlaySound(spkBuffer, numBytesRead);
	    //M5.Spk.PlaySound(spkBuffer, numBytesRead);
	}
	
	// watchdog_postfix(); no. only originator can call this.
}

//-------------------------------------------------------------

TaskHandle_t speak_file(char *waveFilename)
{

    hFile = SD.open(waveFilename);

    if (!hFile)
	{
		printf("file %s does not exist\n", waveFilename);
		return NULL;
    }
	printf("reading file %s +++ \n", waveFilename);

	unsigned int playRate = get_rate();
    mySpeaker.InitI2SSpeakOrMic(MODE_SPK, playRate);
	
	
    bValidWavFile = prepareFile();
    if (!bValidWavFile) {
        printf("*** Not good Illegal wav format\n");
		return NULL;
    }

	// file handle postioned to first data chunk. Play it.
	
	bIsStreaming = true;
	TaskHandle_t hSpkthread = watchdog_task(streamVoice,"wav2voice", 1024*3, NULL, 5);

	return hSpkthread;
	/*
	auto pos = M5.Touch.getPressPoint();
	if (pos.x >= 0 && pos.x < M5.Lcd.width() && pos.y >= 0 &&
		pos.y < M5.Lcd.height()) {
		Serial.println("Repeat");
		prepareFile(hFile);
	}
	*/		  
}


void setup_voice() 
{
    //         LCDEnable, SDEnable, SerialEnable, I2CEnable, mbus_mode_t,SpeakerEnable
    //M5.begin(true,      true,     true,         true,      mbus_mode_t::kMBusModeOutput, true);

	if (!SD.begin(4)) {
	
	  Serial.println("*** STOP missing SD card!");
	
	  while (1);
	
	}

	mySpeaker.begin();
    mySpeaker.InitI2SSpeakOrMic(MODE_SPK, 8000);

	//M5.Speaker.begin(); //Initialize the speaker
    //M5.Speaker.tone(661, 3000);    //Set the speaker to tone at 661Hz for 1000ms

	//dac0 = machine.DAC(25)
	//dac0.write(0)

	Serial.println("SD card started");
}


