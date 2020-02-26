
#include "editor.h"

void Editor_Init() {
  E.cx = 0;
  E.cy = 0;
  E.colOff = 0;
  E.rowOff = 0;
  E.numRows = 0;
  E.rows = NULL;
  E.filename = NULL;

  if (Editor_GetWindowSize(&E.screenRows, &E.screenCols) == -1) die("GetWindowSize");
  E.screenRows -= 1;
}

void Editor_Open(char* filename) {
  free(E.filename);
  E.filename = strdup(filename);

  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t lineCap = 0;
  ssize_t lineLen;

  while ((lineLen = getline(&line, &lineCap, fp)) != -1) {
    while (lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r'))
      lineLen--;
    Editor_AppendRow(line, lineLen);
  }
  free(line);
  fclose(fp);
}

void Editor_AppendRow(char *s, size_t len) {
    E.rows = realloc(E.rows, sizeof(editorRow) * (E.numRows + 1));

    int at = E.numRows;
    E.rows[at].size = len;
    E.rows[at].chars = malloc(len + 1);
    memcpy(E.rows[at].chars, s, len);
    E.rows[at].chars[len] = '\0';

    E.rows[at].renderSize = 0;
    E.rows[at].render = NULL;

    Editor_UpdateRow(&E.rows[at]);

    E.numRows++;
}

void Editor_UpdateRow(editorRow *row) {
  int tabs = 0;
  int j;
  for (j = 0; j < row->size; j++)
    if (row->chars[j] == '\t') tabs++;

  free(row->render);
  row->render = malloc(row->size + tabs*(FANO_TAB_STOP - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % FANO_TAB_STOP != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->renderSize = idx;
}

void Editor_DrawRows(struct buffer *pBuffer) {
  int y;
  for (y = 0; y < E.screenRows; y++) {
    int fileRow = y + E.rowOff;
    if (fileRow >= E.numRows) {
        if (E.numRows == 0) Buffer_Append(pBuffer, "~", 1);
    } else {
      int len = E.rows[fileRow].renderSize - E.colOff;
      if (len < 0) len = 0;
      if (len > E.screenCols) len = E.screenCols;
      Buffer_Append(pBuffer, &E.rows[fileRow].render[E.colOff], len);
    }

    Buffer_Append(pBuffer, EDITOR_EL, 3);
    Buffer_Append(pBuffer, "\r\n", 2);
  }
}

void Editor_DrawStatusBar(struct buffer *pBuffer) {
  Buffer_Append(pBuffer, EDITOR_SGR, 4);

  char status[80], rStatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines",
      E.filename ? E.filename : "[No Name]", E.numRows);

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
  Buffer_Append(pBuffer, "\x1b[ma", 3);
}

void Editor_ProcessKeyPress() {
  int c = Editor_ReadKey();

  switch (c) {
    case CTRL_KEY('q'):
	  refreshScreen();
      exit(0);
      break;

    case HOME_KEY:
      E.cx = 0;
      break;

    case END_KEY:
      if (E.cy < E.numRows)
        E.cx = E.rows[E.cy].size;
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        if (c == PAGE_UP) {
          E.cy = E.rowOff;
        } else if (c == PAGE_DOWN) {
          E.cy = E.rowOff + E.screenRows + 1;
          if (E.cy > E.numRows) E.cy = E.numRows;
        }

        int times = E.screenRows;
        while (times--)
          Editor_MoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break;

    case ARROW_LEFT:
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
      Editor_MoveCursor(c);
      break;
  }
}

int Editor_ReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
      if (nread == -1 && errno != EAGAIN) die("read");
  }

  if (c == EDITOR_SCAPE) {
    char seq[3];

    if (read(STDOUT_FILENO, &seq[0], 1) != 1) return EDITOR_SCAPE;
    if (read(STDOUT_FILENO, &seq[1], 1) != 1) return EDITOR_SCAPE;

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDOUT_FILENO, &seq[2], 1) != 1) return EDITOR_SCAPE;
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
        switch (seq[1]) {
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
    }
    return EDITOR_SCAPE;
  }

  return c;
}

void Editor_RefreshScreen() {
  Editor_Scroll();

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

void Editor_MoveCursor(int key) {
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

void Editor_Scroll() {
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
