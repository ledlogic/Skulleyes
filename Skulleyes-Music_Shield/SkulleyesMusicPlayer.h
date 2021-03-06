/*
 * SkulleyesMusicPlayer.h
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

#ifndef SkulleyesMusicPlayer_H
#define SkulleyesMusicPlayer_H

#include <SD.h>
#include "pins_config.h"
#include "vs10xx.h"
#include  <avr/pgmspace.h>

#define READ_BUF_LEN  32
#define FASTFORWARD_LEN  32768
#define VOLUME_UP   0
#define VOLUME_DOWN 1
#define CHANNEL_LEFT  0
#define CHANNEL_RIGHT 1

#define MAXVOL 0xfe
#define SILENT 0x0

#define MAX_SONG_TOTAL_NUM 40

#define min(a,b) (((a)<(b))?(a):(b))

/******************************************************************************/
#define ENABLE_NUM_DIGIT 8      //for pin[7..0]
#define ENABLE_NUM_ANALOG 2     //for A4/A5

/** Playing states definations. */
volatile typedef enum {
  PS_IDLE = 0,     /**< Player idle                                        */
  PS_PLAY,         /**< Start to player                                    */
  PS_PAUSE,        /**< Pause play                                         */
  PS_PRE_PLAY,     /**< Pre-play state                                     */
  PS_POST_PLAY,    /**< Post-play state                                    */
  PS_PRE_RECORD,   /**< Pre-recording state                                */
  PS_RECORDING,    /**< Recording states                                   */
  PS_POST_RECORD   /**< Post-recording state                               */
} playingstatetype;
extern  playingstatetype playingState;

/** Control states definations. */
volatile typedef enum
{
  CS_EMPTY = 0,      /**< Have no control                                     */
  CS_PLAYPAUSE,      /**< Play/pause button pressed                           */
  CS_PLAYPAUSE_LONG, /**< Play/pause button long pressed                      */
  CS_UP,             /**< Up button pressed                                   */
  CS_UP_LONG,        /**< Up button long pressed                              */
  CS_DOWN,           /**< Down button pressed                                 */
  CS_DOWN_LONG,      /**< Down button long pressed                            */
  CS_NEXT,           /**< Right button pressed                                */
  CS_NEXT_LONG,      /**< Right button long pressed                           */
  CS_PREV,           /**< Left button pressed                                 */
  CS_PREV_LONG,      /**< Left button long pressed                            */
} ctrlState_t;
extern ctrlState_t ctrlState;

/** Play modes definations. */
typedef enum
{
  PM_NORMAL_PLAY = 0,   /**< Request to play according to the order of the list     */
  PM_SHUFFLE_PLAY = 1,  /**< Request to shuffle play according to the list          */
  PM_REPEAT_LIST = 2,   /**< Request to repeat play according to the list           */
  PM_REPEAT_ONE = 3     /**< Request to repeat play a song according to in the list */
} playMode_t;

/****************structure for songs playlist*****************/
typedef struct songDesc
{
  char name[13];
  uint16_t index;
}song_t;
typedef struct songsPlaylist
{
  song_t *p_songFile[MAX_SONG_TOTAL_NUM];
  unsigned char songTotalNum;   //total number of songs in the playlist
  unsigned char currentSongNum;
}spl_t;

/*****************class for the music player*******************/
class SkulleyesMusicPlayer
{
public:
  void    begin(void);

  void    playOne(char *songName);
  boolean addToPlaylist(char *songName);
  void    attachDigitOperation(int pinNum, void (*userFunc)(void), int mode);
  void    attachAnalogOperation(int pinNum, void (*userFunc)(void));

  void    setVolume(unsigned char volume) { vs1053.setVolume(volume, volume); _volume = volume;}
  void    adjustVolume(boolean UpOrDown, unsigned char NumSteps = 6);
  void    setPlayMode(playMode_t playmode) { _playmode = playmode;}

  void    play(void);
  void    _hardtime_update(void);
  boolean isIdle(void);
  String  getPlayingState(void);
  
private:
  unsigned char _volume;
  unsigned int  _currentSongIndex;
  playMode_t    _playmode;
  boolean       playOrPause;
  boolean       Flag_toPlay;
  spl_t         spl;
  boolean       _playingList;

  void    AvailableProcessorTime(void);
  boolean playlistIsEmpty(void);
  void    playlistInit(void);
  boolean _addToPlaylist(uint16_t index, char *songName);
  void    _playSong(char *songName);
  void    controlLED(void);
  boolean playlistIsFull(void);
  boolean _inPlayList(uint16_t);
  void    _play();
  void    _init_timer1();
};

extern SkulleyesMusicPlayer player;
extern SdFile cur_file;

#endif
