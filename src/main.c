

#include "core.h"
#include "editor.h"
#include "terminal.h"


int main(int argc, char *argv[]) {
  Terminal_EnableRawMode();
  Editor_Init();
  if (argc >= 2) {
    Editor_Open(argv[1]);
  }

  Screen_SetStatusMessage("help: Ctrl-s = save | Ctrl-F = find | Ctrl-q = quit");

  while (1) {
    Screen_RefreshScreen();
    Editor_ProcessKeyPress();
  }

  return 0;
}
