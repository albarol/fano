

#include "core.h"
#include "editor.h"
#include "terminal.h"


int main(int argc, char *argv[]) {
  Terminal_EnableRawMode();
  Editor_Init();
  if (argc >= 2) {
    Editor_Open(argv[1]);
  }

  while (1) {
    Editor_RefreshScreen();
    Editor_ProcessKeyPress();
  }

  return 0;
}
