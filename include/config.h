#ifndef __FANO_CONFIG__
#define __FANO_CONFIG__

#include <termios.h>



typedef struct editorRow {
  int size;
  char *chars;
} editorRow;

struct editorConfig {
  int cx, cy;
  int screenRows;
  int screenCols;
  int numRows;
  editorRow* rows;
  struct termios orig_termios;
};

struct editorConfig E;

enum editorKey {
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
