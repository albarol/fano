
#include "screen.h"


void Screen_RefreshScreen() {
  Screen_Scroll();

  struct buffer pBuffer = BUFFER_INIT;

  Buffer_Append(&pBuffer, EDITOR_RM, 6);
  Buffer_Append(&pBuffer, "\x1b[H", 3);

  Editor_DrawRows(&pBuffer);
  Editor_DrawStatusBar(&pBuffer);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowOff) + 1, (E.cx - E.colOff) + 1);
  Buffer_Append(&pBuffer, buf, strlen(buf));

  Buffer_Append(&pBuffer, EDITOR_SM, 6);

  write(STDOUT_FILENO, pBuffer.value, pBuffer.len);

  Buffer_Free(&pBuffer);
}


int Screen_GetWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return Screen_GetCursorPosition(rows, cols);
  } else {
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
  }
}

int Screen_GetCursorPosition(int *rows, int *cols) {
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

void Screen_MoveCursor(int key) {
  editorRow *row = (E.cy >= E.numRows) ? NULL : &E.rows[E.cy];

  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      } else if (E.cy > 0) {
        E.cy--;
        E.cx = E.rows[E.cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E.cx < row->size) {
        E.cx++;
      } else if (row && E.cx == row->size) {
        E.cy++;
        E.cx = 0;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if (E.cy != E.numRows) {
        E.cy++;
      }
      break;
  }

  row = (E.cy >= E.numRows) ? NULL : &E.rows[E.cy];
  int rowLen = row ? row->size : 0;
  if (E.cx > rowLen) {
    E.cx = rowLen;
  }
}

void Screen_Scroll() {
  if (E.cy < E.rowOff) {
    E.rowOff = E.cy;
  }
  if (E.cy >= E.rowOff + E.screenRows) {
    E.rowOff = E.cy - E.screenRows + 1;
  }
  if (E.cx < E.colOff) {
    E.colOff = E.cx;
  }
  if (E.cx >= E.colOff + E.screenCols) {
    E.colOff = E.cx - E.screenCols + 1;
  }
}
