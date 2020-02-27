
#include "editor.h"

void Editor_Init() {
  E.cx = 0;
  E.cy = 0;
  E.colOff = 0;
  E.rowOff = 0;
  E.numRows = 0;
  E.rows = NULL;
  E.filename = NULL;

  if (Screen_GetWindowSize(&E.screenRows, &E.screenCols) == -1) die("GetWindowSize");
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

void Editor_FreeRow(editorRow *row) {
  free(row->render);
  free(row->chars);
}

void Editor_AppendCharAtRow(editorRow *row, int at, int c) {
  if (at < 0 || at > row->size) at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  Editor_UpdateRow(row);
}


void Editor_RemoveCharAtRow(editorRow *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  Editor_UpdateRow(row);
}

void Editor_InsertChar(int c) {
  if (E.cy == E.numRows) {
    Editor_AppendRow("", 0);
  }
  Editor_AppendCharAtRow(&E.rows[E.cy], E.cx, c);
  E.cx++;
}

void Editor_RemoveChar() {
  if (E.cy == E.numRows) return;

  editorRow *row = &E.rows[E.cy];
  if (E.cx > 0) {
    Editor_RemoveCharAtRow(row, E.cx - 1);
    E.cx--;
  }
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
    case '\r':
      // TODO
      break;

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

    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:
      if (c == DEL_KEY) Screen_MoveCursor(ARROW_RIGHT);
      Editor_RemoveChar();
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
          Screen_MoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break;

    case ARROW_LEFT:
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
      Screen_MoveCursor(c);
      break;

    case CTRL_KEY('l'):
    case '\x1b':
      break;

    default:
      Editor_InsertChar(c);
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
