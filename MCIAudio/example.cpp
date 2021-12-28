#define MCI_AUDIO
#include "../include/MCIAudio.h"
#include <iostream>
using namespace MCIAudio;

int main()
{
    Audio audio;
    audio.Open(L"../audios/BOBACRI - Sleep Away.mp3");
    audio.Play();
    audio.Pause();
    audio.Resume();
    std::cout << "playing.\n";
    system("pause");
    return 0;
}