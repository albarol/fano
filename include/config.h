#ifndef __FANO_CONFIG__
#define __FANO_CONFIG__

#include <termios.h>



typedef struct editorRow {
  int size;
  int renderSize;
  char *chars;
  char *render;
} editorRow;

struct editorConfig {
  int cx, cy;
  int rowOff;
  int colOff;
  int screenRows;
  int screenCols;
  int numRows;
  editorRow* rows;
  char* filename;
  struct termios orig_termios;
};

struct editorConfig E;

enum editorKey {
  BACKSPACE = 127,
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
