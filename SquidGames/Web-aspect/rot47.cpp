
/*********
 -*- Made by VoxelPixel
 -*- For YouTube Tutorial
 -*- https://github.com/VoxelPixel
 -*- Support me on Patreon: https://www.patreon.com/voxelpixel
*********/

#include <string.h>

void cipherEncryption(char *source){

    int key = 47;
	
    char *encrypText = source;

    for (int i = 0; i < strlen(source); i++)
	{
        int temp = source[i] + key;
        if(source[i] == 32){
            *encrypText = ' ';
        } else if (temp > 126){
            temp -= 94;
            *encrypText = temp;
        } else {
            *encrypText = temp;
        }
        encrypText++;
    }
}

void cipherDecryption(char *message){

    int key = 47;
    char *decrypText = message;

    for (int i = 0; i < strlen(message); i++){
        int temp = message[i] - key;
        if(message[i] == 32){
            *decrypText = ' ';
        } else if (temp < 32){
            temp += 94;
            *decrypText = temp;
        } else {
            *decrypText = temp;
        }
		decrypText++;
    }
}

