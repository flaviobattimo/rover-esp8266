#include "pitches.h"

#define MELODY_MODE_NONE 0
#define MELODY_MODE_MUSIC (1<<0)
#define MELODY_MODE_LASER (1<<1)
#define MELODY_MODE_LED (1<<2)
#define MELODY_MODE_STEER_WHEELS (1<<3)
#define MELODY_MODE_ALL (0xFF)
#define MELODY_MODE_DEFAULT MELODY_MODE_ALL

typedef struct Melody_t{
  uint8_t mode;
  int* data;
  int tempo;
  Melody_t(uint8_t mode,int* data,int tempo){
    this->mode=mode;
    this->data=data;
    this->tempo=tempo;
  }
  Melody_t(){
    mode=MELODY_MODE_NONE;
    tempo=0;
    data=0;
  }
} Melody;


// Jingle Bells
int jingle_bells_melody[] = {
  NOTE_E5,8, NOTE_E5,8, NOTE_E5,4,
  NOTE_E5,8, NOTE_E5,8, NOTE_E5,4,
  NOTE_E5,8, NOTE_G5,8, NOTE_C5,8, NOTE_D5,8,
  NOTE_E5,2,
  NOTE_F5,8, NOTE_F5,8, NOTE_F5,8, NOTE_F5,8,
  NOTE_F5,8, NOTE_E5,8, NOTE_E5,8, NOTE_E5,16, NOTE_E5, 16,
  NOTE_E5,8, NOTE_D5,8, NOTE_D5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_G5,4,0
};


// We wish you a merry Christmas

int wish_melody[] = {
  NOTE_B3,4, 
  NOTE_F4,4, NOTE_F4,8, NOTE_G4,8, NOTE_F4,8, NOTE_E4,8,
  NOTE_D4,4, NOTE_D4,4, NOTE_D4,4,
  NOTE_G4,4, NOTE_G4,8, NOTE_A4,8, NOTE_G4,8, NOTE_F4,8,
  NOTE_E4,4, NOTE_E4,4, NOTE_E4,4,
  NOTE_A4,4, NOTE_A4,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
  NOTE_F4,4, NOTE_D4,4, NOTE_B3,8, NOTE_B3,8,
  NOTE_D4,4, NOTE_G4,4, NOTE_E4,4,
  NOTE_F4,2,0
};


// Santa Claus is coming to town

int santa_melody[] = {
  NOTE_G4,8,
  NOTE_E4,8, NOTE_F4,8, NOTE_G4,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_A4,8, NOTE_B4,8, NOTE_C5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_E4,8, NOTE_F4,8, NOTE_G4,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_A4,8, NOTE_G4,8, NOTE_F4,4, NOTE_F4,2,
  NOTE_E4,4, NOTE_G4,4, NOTE_C4,4, NOTE_E4,4,
  NOTE_D4,4, NOTE_F4,2, NOTE_B3,4,
  NOTE_C4,1,0
};

int brahms_lullaby_melody[] = {

  // Wiegenlied (Brahms' Lullaby)
  // Score available at https://www.flutetunes.com/tunes.php?id=54

  NOTE_G4, 4, NOTE_G4, 4, //1
  NOTE_AS4, -4, NOTE_G4, 8, NOTE_G4, 4,
  NOTE_AS4, 4, REST, 4, NOTE_G4, 8, NOTE_AS4, 8,
  NOTE_DS5, 4, NOTE_D5, -4, NOTE_C5, 8,
  NOTE_C5, 4, NOTE_AS4, 4, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_GS4, 4, NOTE_F4, 4, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_GS4, 4, REST, 4, NOTE_F4, 8, NOTE_GS4, 8,
  NOTE_D5, 8, NOTE_C5, 8, NOTE_AS4, 4, NOTE_D5, 4,

  NOTE_DS5, 4, REST, 4, NOTE_DS4, 8, NOTE_DS4, 8, //8
  NOTE_DS5, 2, NOTE_C5, 8, NOTE_GS4, 8,
  NOTE_AS4, 2, NOTE_G4, 8, NOTE_DS4, 8,
  NOTE_GS4, 4, NOTE_AS4, 4, NOTE_C5, 4,
  NOTE_AS4, 2, NOTE_DS4, 8, NOTE_DS4, 8,
  NOTE_DS5, 2, NOTE_C5, 8, NOTE_GS4, 8,
  NOTE_AS4, 2, NOTE_G4, 8, NOTE_DS4, 8,
  NOTE_AS4, 4, NOTE_G4, 4, NOTE_DS4, 4,
  NOTE_DS4, 2

};

Melody MELODIES[]={
  Melody{MELODY_MODE_DEFAULT,jingle_bells_melody,2000},
  Melody{MELODY_MODE_DEFAULT,wish_melody,1800},
  Melody{MELODY_MODE_DEFAULT,santa_melody,2000},
  Melody{MELODY_MODE_MUSIC,brahms_lullaby_melody,2000},
};

#define MELODIES_COUNT (sizeof(MELODIES) / sizeof(Melody))

