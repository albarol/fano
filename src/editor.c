
#include "editor.h"

void Editor_Init() {
  E.cx = 0;
  E.cy = 0;
  E.numRows = 0;
  E.rows = NULL;

  if (Editor_GetWindowSize(&E.screenRows, &E.screenCols) == -1) die("GetWindowSize");
}

void Editor_Open(char* filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
      linelen--;
    Editor_AppendRow(line, linelen);
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
    E.numRows++;
}

void Editor_DrawRows(struct buffer *pBuffer) {
  int y;
  for (y = 0; y < E.screenRows; y++) {
    if (y >= E.numRows) {
        if (E.numRows == 0) Buffer_Append(pBuffer, "~", 1);
    } else {
      int len = E.rows[y].size;
      if (len > E.screenCols) len = E.screenCols;
      Buffer_Append(pBuffer, E.rows[y].chars, len);
    }

    Buffer_Append(pBuffer, EDITOR_EL, 3);
    if (y < E.screenRows - 1) {
      Buffer_Append(pBuffer, "\r\n", 2);
    }
  }
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
      E.cx = E.screenCols - 1;
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
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
  struct buffer pBuffer = BUFFER_INIT;

  Buffer_Append(&pBuffer, EDITOR_RM, 6);
  Buffer_Append(&pBuffer, "\x1b[H", 3);

  Editor_DrawRows(&pBuffer);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
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
  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      }
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screenCols - 1) {
        E.cx++;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if (E.cy != E.screenRows - 1) {
        E.cy++;
      }
      break;
  }
}
