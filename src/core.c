
#include "core.h"

void die(const char *s) {
  refreshScreen();
  perror(s);
  exit(1);
}

void refreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void Buffer_Append(struct buffer *pBuffer, const char *s, int len) {
  char *new = realloc(pBuffer->value, pBuffer->len + len);

  if (new == NULL) return;
  memcpy(&new[pBuffer->len], s, len);
  pBuffer->value = new;
  pBuffer->len += len;
}

void Buffer_Free(struct buffer *pBuffer) {
  free(pBuffer->value);
}
