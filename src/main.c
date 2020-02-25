
//#include <ctype.h>
//#include <errno.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>

#include "core.h"
#include "editor.h"
#include "terminal.h"


int main() {
  Terminal_EnableRawMode();

  while (1) {
    Editor_RefreshScreen();
    Editor_ProcessKeyPress();
  }

  return 0;
}
