/*
 * SkulleyesMusicPlayer.cpp
 * A library for MusicShield 2.0
 *
 * Copyright (c) 2012 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Jack Shao (jacky.shaoxg@gmail.com)
 * Create Time: Mar 2014
 * Change Log :
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SD.h>
#include <SkulleyesMusicPlayer.h>
#include  <avr/pgmspace.h>

typedef void (*voidFunctionPtr)(void);
volatile static voidFunctionPtr digitCtrlFunc[ENABLE_NUM_DIGIT];
char _DigitPinLeval[ENABLE_NUM_DIGIT];
char _DigitPinNum[ENABLE_NUM_DIGIT];

volatile static voidFunctionPtr analogCtrlFunc[ENABLE_NUM_ANALOG];
char _AnalogPinNum[ENABLE_NUM_DIGIT];

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 10;
static byte readBuf[READ_BUF_LEN];
volatile static boolean playdone = false;

SkulleyesMusicPlayer player;
Sd2Card     card;
SdVolume    volume;
SdFile      root;
SdFile      cur_file;

playingstatetype playingState;
ctrlState_t      ctrlState;

volatile static byte ledflagOn = 1;
volatile static int  ledcount = 50;
volatile static byte timerloop = 0;
volatile static byte recording_state = 0;
volatile static byte fastforward = 0;

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void showString(PGM_P s)
{
  char c;
  while ((c = pgm_read_byte(s++)) != 0) Serial.print(c);
}

ISR(TIMER1_OVF_vect)          //Timer1 Service
{
  //fill vs1053
  while (digitalRead(VS_DREQ) == 1 && playingState == PS_PLAY && cur_file.isOpen() && !fastforward)
  {
    byte readLen = 0;
    readLen = cur_file.read(readBuf, READ_BUF_LEN);
    vs1053.writeData(readBuf, readLen);
    if (readLen < READ_BUF_LEN)
    {
      vs1053.writeRegister(SPI_MODE, 0, SM_OUTOFWAV);
      vs1053.sendZerosToVS10xx();
      //report play done event here...
      playingState = PS_POST_PLAY;
      break;
    }
  }
  //update
  if (++timerloop >= 20)
  {
    player._hardtime_update();
    timerloop = 0;
  }
}

//=============================================================
#define RESOLUTION 65536    // Timer1 is 16 bit
void SkulleyesMusicPlayer::_init_timer1(void)        //initialize Timer1 to 100us overflow
{
  TCCR1A = 0;                 // clear control register A
  TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer

  long cycles;
  long microseconds = 500;   //setup microseconds here
  unsigned char clockSelectBits;
  cycles = (F_CPU / 2000000) * microseconds;                                // the counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2
  if (cycles < RESOLUTION)              clockSelectBits = _BV(CS10);              // no prescale, full xtal
  else if ((cycles >>= 3) < RESOLUTION) clockSelectBits = _BV(CS11);              // prescale by /8
  else if ((cycles >>= 3) < RESOLUTION) clockSelectBits = _BV(CS11) | _BV(CS10);  // prescale by /64
  else if ((cycles >>= 2) < RESOLUTION) clockSelectBits = _BV(CS12);              // prescale by /256
  else if ((cycles >>= 2) < RESOLUTION) clockSelectBits = _BV(CS12) | _BV(CS10);  // prescale by /1024
  else        cycles = RESOLUTION - 1, clockSelectBits = _BV(CS12) | _BV(CS10);  // request was out of bounds, set as maximum

  ICR1 = cycles;
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TCCR1B |= clockSelectBits;                                          // reset clock select register, and starts the clock

  TIMSK1 = _BV(TOIE1);
  TCNT1 = 0;
  sei();                      //enable global interrupt
}

void SkulleyesMusicPlayer::begin(void)
{
  initIOForLED();
  vs1053.init();
  /* init sd card */
  //SD.begin(chipSelect);
  pinMode(chipSelect, OUTPUT);
  if (!card.init(SPI_HALF_SPEED, chipSelect))
  {
    showString(PSTR("initialization failed. Things to check:\r\n"));
    showString(PSTR("* is a card is inserted?\r\n"));
    showString(PSTR("* Is your wiring correct?\r\n"));
    showString(PSTR("* did you change the chipSelect pin to match your shield or module?\r\n"));
    return;
  }
  showString(PSTR("Card type: "));
  switch (card.type())
  {
  case SD_CARD_TYPE_SD1:
    showString(PSTR("SD1\r\n"));
    break;
  case SD_CARD_TYPE_SD2:
    showString(PSTR("SD2\r\n"));
    break;
  case SD_CARD_TYPE_SDHC:
    showString(PSTR("SDHC\r\n"));
    break;
  default:
    showString(PSTR("Unknown\r\n"));
  }
  if (!volume.init(card))
  {
    showString(PSTR("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card\r\n"));
    return;
  }
  showString(PSTR("Volume type is FAT"));
  Serial.println(volume.fatType(), DEC);
  root.openRoot(volume);

  playlistInit();
  setVolume(40);
  _playmode = PM_NORMAL_PLAY;
  playingState = PS_PRE_PLAY;
  _playmode = PM_NORMAL_PLAY;
  ctrlState = CS_EMPTY;
  /* init timer1 */
  _init_timer1();
}

void SkulleyesMusicPlayer::_hardtime_update(void)
{
  controlLED();
}

void SkulleyesMusicPlayer::play(void)
{
  _play();
}

void SkulleyesMusicPlayer::_play(void)
{
  song_t *songFile;
  switch (playingState)
  {
    //
  case PS_IDLE:
    if (ctrlState == CS_PLAYPAUSE)
    {
      playingState = PS_PRE_PLAY;
      ctrlState = CS_EMPTY;
    }
#if defined(__AVR_ATmega1280__)|| defined(__AVR_ATmega2560__)
    if (ctrlState == CS_PLAYPAUSE_LONG)
    {
      playingState = PS_PRE_RECORD;
      ctrlState = CS_EMPTY;
    }
#endif
    if (cur_file.isOpen()) cur_file.close();
    break;
    //
  case PS_PRE_PLAY:
    if (playlistIsEmpty())
    {
      showString(PSTR("Playlist is empty.\r\n"));
      playingState = PS_IDLE;
      break;
    }
    vs1053.softReset();

    if (cur_file.isOpen()) cur_file.close();
    songFile = spl.p_songFile[spl.currentSongNum];

    if (!cur_file.open(&root, songFile->index, O_READ))
    {
      Serial.print(songFile->name);
      showString(PSTR(" open failed.\r\n"));
      playingState = PS_POST_PLAY;
      break;
    }
    showString(PSTR("Playing song "));
    Serial.print(songFile->name);
    showString(PSTR(" ...\r\n"));
    playingState = PS_PLAY;
    break;
    //
  case PS_PLAY:
    if (ctrlState == CS_PLAYPAUSE)
    {
      ctrlState = CS_EMPTY;
      playingState = PS_PAUSE;
    }
    if (ctrlState == CS_PLAYPAUSE_LONG)
    {
      ctrlState = CS_EMPTY;
      playingState = PS_IDLE;
    }
    if (ctrlState == CS_DOWN)
    {
      ctrlState = CS_EMPTY;
      adjustVolume(0);
    }
    if (ctrlState == CS_UP)
    {
      ctrlState = CS_EMPTY;
      adjustVolume(1);
    }
    if (ctrlState == CS_NEXT)
    {
      ctrlState = CS_EMPTY;

      if (_playmode == PM_SHUFFLE_PLAY) spl.currentSongNum = random(0, spl.songTotalNum);
      else if (spl.currentSongNum == spl.songTotalNum - 1)   spl.currentSongNum = 0;
      else ++spl.currentSongNum;
      cur_file.close();
      playingState = PS_PRE_PLAY;
    }
    if (ctrlState == CS_PREV)
    {
      ctrlState = CS_EMPTY;
      if (_playmode == PM_SHUFFLE_PLAY) spl.currentSongNum = random(0, spl.songTotalNum);
      else if (spl.currentSongNum == 0)  spl.currentSongNum = spl.songTotalNum - 1;
      else --spl.currentSongNum;
      cur_file.close();
      playingState = PS_PRE_PLAY;
    }

    if (ctrlState == CS_NEXT_LONG)
    {
      ctrlState = CS_EMPTY;
      fastforward = 1;
      uint32_t pos = cur_file.curPosition();
      if (pos + FASTFORWARD_LEN < cur_file.fileSize())
      {
        cur_file.seekSet(pos + FASTFORWARD_LEN);
      }
      fastforward = 0;
    }

    if (ctrlState == CS_PREV_LONG)
    {
      ctrlState = CS_EMPTY;
      fastforward = 1;
      uint32_t pos = cur_file.curPosition();
      if (pos - FASTFORWARD_LEN > 0)
      {
        cur_file.seekSet(pos - FASTFORWARD_LEN);
      }
      fastforward = 0;
    }
    break;
    //
  case PS_PAUSE:
    if (ctrlState == CS_PLAYPAUSE)
    {
      ctrlState = CS_EMPTY;
      playingState = PS_PLAY;
    }
    break;
    //
  case PS_POST_PLAY:
    showString(PSTR(" [done].\r\n"));
    if (cur_file.isOpen()) cur_file.close();
    if (_playmode == PM_SHUFFLE_PLAY)
    {
      spl.currentSongNum = random(0, spl.songTotalNum);
      playingState = PS_PRE_PLAY;
    } else if (_playmode == PM_NORMAL_PLAY)
    {
      if (spl.currentSongNum == spl.songTotalNum - 1)
      {
        playingState = PS_IDLE;
      } else
      {
        spl.currentSongNum++;
        playingState = PS_PRE_PLAY;
      }
    } else if (_playmode == PM_REPEAT_LIST)
    {
      if (spl.currentSongNum == spl.songTotalNum - 1)   spl.currentSongNum = 0;
      else spl.currentSongNum++;
      playingState = PS_PRE_PLAY;
    } else if (_playmode == PM_REPEAT_ONE)
    {
      playingState = PS_PRE_PLAY;
    }
    break;

  default:
    break;
  }
}

boolean SkulleyesMusicPlayer::_addToPlaylist(uint16_t index, char *songName) //add a song to current playlist
{
  if (spl.songTotalNum >= (MAX_SONG_TOTAL_NUM - 1)) return false;

  if (_inPlayList(index))
  {
    Serial.print(songName);
    showString(PSTR(" already exists in playlist.\r\n"));
    return false;
  }

  SdFile f;
  if (!f.open(&root, index, O_READ))
  {
    Serial.print(songName);
    showString(PSTR(" cant be opened.\r\n"));
    return false;
  }
  f.close();

  song_t *sd = (song_t *)malloc(sizeof(song_t));
  if (sd)
  {
    strncpy(sd->name, songName, 13);
    sd->index = index;
    spl.p_songFile[spl.songTotalNum] = sd;
    spl.songTotalNum++;
    return true;
  }else
  {
    showString(PSTR("run out of ram.\r\n"));
  }
  return false;
}

boolean SkulleyesMusicPlayer::addToPlaylist(char *songName) //add a song to current playlist
{
  SdFile f;
  if (!f.open(&root, songName, O_READ))
  {
    Serial.print(songName);
    showString(PSTR(" is not found.\r\n"));
    return false;
  }
  f.close();
  uint32_t pos = root.curPosition();
  return _addToPlaylist((pos>>5)-1,songName);
  Serial.print("Add to play list: index");
  Serial.print((pos>>5)-1);
  Serial.print("  songName:");
  Serial.println(songName);
}

void SkulleyesMusicPlayer::adjustVolume(boolean UpOrDown, unsigned char NumSteps)
{
  if (UpOrDown == 0)
  {
    if (_volume < NumSteps) _volume = SILENT;
    else _volume -= NumSteps;
  } else
  {
    if (_volume > MAXVOL - NumSteps) _volume = MAXVOL;
    else _volume += NumSteps;
  }
  setVolume(_volume);
}

boolean SkulleyesMusicPlayer::playlistIsEmpty(void)
{
  if (spl.songTotalNum == 0) return true;
  else return false;
}
void SkulleyesMusicPlayer::playlistInit(void)
{
  unsigned char i;
  for (i = 0; i < MAX_SONG_TOTAL_NUM; i++)
  {
    spl.p_songFile[i] = 0;
  }
  spl.songTotalNum = 0;
  spl.currentSongNum = 0;
}
void SkulleyesMusicPlayer::controlLED(void)
{
  if (playingState == PS_PAUSE) GREEN_LED_ON();
  else if (playingState == PS_IDLE) GREEN_LED_OFF();
  else if (--ledcount == 0)
  {
    ledcount = 50;
    if (ledflagOn == 1)
    {
      GREEN_LED_ON();
      ledflagOn = 0;
    } else
    {
      GREEN_LED_OFF();
      ledflagOn = 1;
    }
  }
}
boolean SkulleyesMusicPlayer::playlistIsFull(void)
{
  if (spl.songTotalNum < MAX_SONG_TOTAL_NUM) return false;
  else return true;
}

boolean SkulleyesMusicPlayer::_inPlayList(uint16_t index)
{
  for (unsigned char i = 0; i < MAX_SONG_TOTAL_NUM; i++)
  {
    if (spl.p_songFile[i] == 0) continue;
    if (spl.p_songFile[i]->index == index)
    {
      return true;
    }
  }
  return false;
}

// added

boolean SkulleyesMusicPlayer::isIdle(void) 
{
	return playingState == PS_IDLE || playingState == PS_PAUSE;
}

String SkulleyesMusicPlayer::getPlayingState(void)
{
	return String(playingState);
}

void SkulleyesMusicPlayer::playOne(char *songFile)
{
  playlistInit();
  
  SdFile f;
  if (!f.open(&root, songFile, O_READ))
  {
    Serial.print(songFile);
    showString(PSTR(" does not exists.\r\n"));
    return;
  }
  f.close();
  if (!_inPlayList(root.dirIndex( )))
  {
    addToPlaylist(songFile);
    
    // added for skulleyes
    _playmode = PM_NORMAL_PLAY;
  	playingState = PS_PRE_PLAY;
  }
}