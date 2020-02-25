
#include "editor.h"

void Editor_Init() {
  if (Editor_GetWindowSize(&E.screenRows, &E.screenCols) == -1) die("GetWindowSize");
}

void Editor_DrawRows() {
  int y;
  for (y = 0; y < E.screenRows; y++) {
    write(STDOUT_FILENO, "~", 1);

    if (y < E.screenRows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
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


int Editor_GetWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return Editor_GetCursorPosition(rows, cols);
  } else {
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
  }
}

int Editor_GetCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) -1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}
