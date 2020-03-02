#ifndef __FANO_CONFIG__
#define __FANO_CONFIG__

#include <termios.h>
#include <time.h>



typedef struct editorRow {
  int size;
  int renderSize;
  char *chars;
  char *render;
} editorRow;

struct editorConfig {
  int rx, cx, cy;
  int rowOff;
  int colOff;
  int screenRows;
  int screenCols;
  int numRows;
  editorRow* rows;
  int dirty;
  char* filename;
  char statusMsg[80];
  time_t statusMsgTime;
  struct termios orig_termios;
};

struct editorConfig E;

enum editorKey {
  BACKSPACE = 127,
  TAB = 9,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

#endif
