
#include "screen.h"

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

void Screen_RefreshScreen() {
  Screen_Scroll();

  struct buffer pBuffer = BUFFER_INIT;

  Buffer_Append(&pBuffer, EDITOR_RM, 6);
  Buffer_Append(&pBuffer, "\x1b[H", 3);

  Editor_DrawRows(&pBuffer);
  Screen_DrawStatusBar(&pBuffer);
  Screen_DrawMessageBar(&pBuffer);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowOff) + 1, (E.cx - E.colOff) + 1);
  Buffer_Append(&pBuffer, buf, strlen(buf));

  Buffer_Append(&pBuffer, EDITOR_SM, 6);

  write(STDOUT_FILENO, pBuffer.value, pBuffer.len);

  Buffer_Free(&pBuffer);
}

void Screen_DrawStatusBar(struct buffer *pBuffer) {
  Buffer_Append(pBuffer, EDITOR_SGR, 4);

  char status[80], rStatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
      E.filename ? E.filename : "[No Name]", E.numRows,
      E.dirty ? "(modified)" : "");

  int rLen = snprintf(rStatus, sizeof(rStatus), "%d/%d",
      E.cy + 1, E.numRows);

  if (len > E.screenCols) len = E.screenCols;
  Buffer_Append(pBuffer, status, len);

  while (len < E.screenCols) {
    if (E.screenCols - len == rLen) {
      Buffer_Append(pBuffer, rStatus, rLen);
      break;
    } else {
      Buffer_Append(pBuffer, " ", 1);
      len++;
    }
  }
  Buffer_Append(pBuffer, "\x1b[m", 3);
  Buffer_Append(pBuffer, "\r\n", 2);
}

void Screen_DrawMessageBar(struct buffer *pBuffer) {
   Buffer_Append(pBuffer, "\x1b[K", 3);
   int msgLen = strlen(E.statusMsg);
   if (msgLen > E.screenCols) msgLen = E.screenCols;
   if (msgLen && time(NULL) - E.statusMsgTime < 3)
     Buffer_Append(pBuffer, E.statusMsg, msgLen);
}

void Screen_SetStatusMessage(const char* fmt, ...) {
	va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusMsg, sizeof(E.statusMsg), fmt, ap);
    va_end(ap);
    E.statusMsgTime = time(NULL);
}
