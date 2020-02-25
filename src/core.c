
#include "core.h"

void die(const char *s) {
  refreshScreen();
  perror(s);
  exit(1);
}

void refreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}
