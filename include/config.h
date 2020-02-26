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

#endif
