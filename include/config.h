
#include <termios.h>


struct editorConfig {
  struct termios orig_termios;
};

struct editorConfig E;
