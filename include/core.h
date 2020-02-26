#ifndef __FANO_CORE__
#define __FANO_CORE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define BUFFER_INIT {NULL, 0}

struct buffer {
  char *value;
  int len;
};


void die(const char *s);
void refreshScreen();

void Buffer_Append(struct buffer *pBuffer, const char *s, int len);
void Buffer_Free(struct buffer *pBuffer);

#endif
