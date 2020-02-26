#ifndef __FANO_CONFIG__
#define __FANO_CONFIG__

#include <termios.h>


struct editorConfig {
  int cx, cy;
  int screenRows;
  int screenCols;
  struct termios orig_termios;
};

struct editorConfig E;

enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

#endif
