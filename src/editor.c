
#include "editor.h"

void Editor_Init() {
  E.cx = 0;
  E.cy = 0;
  E.colOff = 0;
  E.rowOff = 0;
  E.numRows = 0;
  E.rows = NULL;
  E.dirty = 0;
  E.filename = NULL;
  E.statusMsg[0] = '\0';
  E.statusMsgTime = 0;
  E.lastSearch = NULL;

  if (Screen_GetWindowSize(&E.screenRows, &E.screenCols) == -1) die("GetWindowSize");
  E.screenRows -= 2;
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
    Editor_InsertRow(E.numRows, line, lineLen);
  }
  free(line);
  fclose(fp);
  E.dirty = 0;
}

void Editor_Save() {
  if (E.filename == NULL) {
    E.filename = Editor_Prompt("Save as: %s (ESC to cancel)");
    if (E.filename == NULL) {
      Screen_SetStatusMessage("Save aborted.");
      return;
    }
  }

  int len;
  char *buffer = Editor_GetRowsAsString(&len);

  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buffer, len) != -1) {
        close(fd);
        free(buffer);
        E.dirty = 0;
        Screen_SetStatusMessage("%d bytes written to disk", len);
        return;
      }
    }
    close(fd);
  }
  free(buffer);
  Screen_SetStatusMessage("I/O error: %s", strerror(errno));
}

void Editor_Find() {
  char* query = Editor_Prompt("Search: %s (ESC to cancel).");
  if (query == NULL) return;

  if (E.lastSearch) free(E.lastSearch);

  size_t bufSize = 128;
  E.lastSearch = malloc(bufSize);
  strcpy(E.lastSearch, query);

  int i;
  for (i = 0; i < E.numRows; i++) {
    editorRow* row = &E.rows[i];
    char *match = strstr(row->render, query);

    if (match) {
      E.cy = i;
	  E.cx = Screen_TransformToCursorPosition(row, match - row->render);
      E.rowOff = E.numRows;
      break;
    }
  }

  free(query);
}

void Editor_FindNext() {
  if (E.lastSearch == NULL) return;

  int i;
  for (i = E.cy + 1; i < E.numRows; i++) {
    editorRow* row = &E.rows[i];
    char *match = strstr(row->render, E.lastSearch);

    if (match) {
      E.cy = i;
	  E.cx = Screen_TransformToCursorPosition(row, match - row->render);
      E.rowOff = E.numRows;
      break;
    }
  }
}

void Editor_FindPrevious() {
  if (E.lastSearch == NULL) return;

  int i;
  for (i = E.cy - 1; i >= 0; i--) {
    editorRow* row = &E.rows[i];
    char *match = strstr(row->render, E.lastSearch);

    if (match) {
      E.cy = i;
	  E.cx = Screen_TransformToCursorPosition(row, match - row->render);
      E.rowOff = E.numRows;
      break;
    }
  }
}


void Editor_InsertRow(int at, char *s, size_t len) {
    if (at < 0 || at > E.numRows) return;

    E.rows = realloc(E.rows, sizeof(editorRow) * (E.numRows + 1));
    memmove(&E.rows[at + 1], &E.rows[at], sizeof(editorRow) * (E.numRows - at));

    E.rows[at].size = len;
    E.rows[at].chars = malloc(len + 1);
    memcpy(E.rows[at].chars, s, len);
    E.rows[at].chars[len] = '\0';

    E.rows[at].renderSize = 0;
    E.rows[at].render = NULL;

    Editor_UpdateRow(&E.rows[at]);

    E.numRows++;
    E.dirty++;
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

void Editor_AppendCharAtRow(editorRow *row, int at, int c) {
  if (at < 0 || at > row->size) at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  Editor_UpdateRow(row);
  E.dirty++;
}


void Editor_RemoveCharAtRow(editorRow *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  Editor_UpdateRow(row);
}

void Editor_InsertChar(int c) {
  if (E.cy == E.numRows) {
    Editor_InsertRow(E.numRows, "", 0);
  }
  Editor_AppendCharAtRow(&E.rows[E.cy], E.cx, c);
  E.cx++;
}

void Editor_InsertNewLine() {
  if (E.cx == 0) {
    Editor_InsertRow(E.cy, "", 0);
  } else {
    editorRow *row = &E.rows[E.cy];
    Editor_InsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.rows[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    Editor_UpdateRow(row);
  }
  E.cy++;
  E.cx = 0;
}

void Editor_RowAppendString(editorRow *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  Editor_UpdateRow(row);
  E.dirty++;
}

void Editor_RemoveChar() {
  if (E.cy == E.numRows) return;
  if (E.cx == 0 && E.cy == 0) return;

  editorRow *row = &E.rows[E.cy];
  if (E.cx > 0) {
    Editor_RemoveCharAtRow(row, E.cx - 1);
    E.cx--;
  } else {
    E.cx = E.rows[E.cy - 1].size;
    Editor_RowAppendString(&E.rows[E.cy - 1], row->chars, row->size);
    Editor_DeleteRow(E.cy);
    E.cy--;
  }
}

void Editor_FreeRow(editorRow *row) {
  free(row->render);
  free(row->chars);
}

void Editor_DeleteRow(int at) {
  if (at < 0 || at >= E.numRows) return;
  Editor_FreeRow(&E.rows[at]);
  memmove(&E.rows[at], &E.rows[at + 1], sizeof(editorRow) * (E.numRows - at - 1));
  E.numRows--;
  E.dirty++;
}

char *Editor_GetRowsAsString(int *bufferLength) {
  int toLen = 0;
  int j;
  for (j = 0; j < E.numRows; j++)
    toLen += E.rows[j].size + 1;

  *bufferLength = toLen;

  char *buffer = malloc(toLen);
  char *p = buffer;

  for (j = 0; j < E.numRows; j++) {
    memcpy(p, E.rows[j].chars, E.rows[j].size);
    p += E.rows[j].size;
    *p = '\n';
    p++;
  }

  return buffer;
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

void Editor_ProcessKeyPress() {
  int c = Editor_ReadKey();

  switch (c) {
    case '\r':
      Editor_InsertNewLine();
      break;

    case CTRL_KEY('q'):
      if (E.dirty > 0) {
        Screen_SetStatusMessage("Warning! File has unsaved changes. Press Ctrl+x to quit without saving.");
        break;
      }
	  refreshScreen();
      exit(0);
      break;

    case CTRL_KEY('x'):
      refreshScreen();
      exit(0);
      break;

    case CTRL_KEY('f'):
      Editor_Find();
      break;

    case CTRL_KEY('n'):
      Editor_FindNext();
      break;

    case CTRL_KEY('p'):
      Editor_FindPrevious();
      break;

    case CTRL_KEY('s'):
	  Editor_Save();
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


char *Editor_Prompt(char *prompt) {
  size_t bufSize = 128;
  char *buf = malloc(bufSize);

  size_t bufLen = 0;
  buf[0] = '\0';

  while (1) {
    Screen_SetStatusMessage(prompt, buf);
    Screen_RefreshScreen();

    int c = Editor_ReadKey();
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
      if (bufLen != 0) buf[--bufLen] = '\0';
    } else if(c == '\x1b') {
      Screen_SetStatusMessage("");
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (bufLen != 0) {
        Screen_SetStatusMessage("");
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (bufLen == bufSize - 1) {
        bufSize *= 2;
        buf = realloc(buf, bufSize);
      }
      buf[bufLen++] = c;
      buf[bufLen] = '\0';
    }
  }
}
