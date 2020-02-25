
#include "editor.h"

void Editor_DrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void Editor_ProcessKeyPress() {
  char c = Editor_ReadKey();

  switch (c) {
    case CTRL_KEY('q'):
	  refreshScreen();
      exit(0);
      break;
  }
}

char Editor_ReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
      if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

void Editor_RefreshScreen() {
  refreshScreen();
  Editor_DrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}
