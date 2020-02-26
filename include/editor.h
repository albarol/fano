#ifndef __FANO_EDITOR__
#define __FANO_EDITOR__

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "cons.h"
#include "core.h"

void Editor_Init();
void Editor_Open();

void Editor_AppendRow(char *s, size_t len);
void Editor_UpdateRow(editorRow *row);
void Editor_DrawRows(struct buffer *pBuffer);
void Editor_DrawStatusBar(struct buffer *pBuffer);
void Editor_ProcessKeyPress();
int Editor_ReadKey();
void Editor_RefreshScreen();

int Editor_GetWindowSize(int *rows, int *cols);
int Editor_GetCursorPosition(int *rows, int *cols);
void Editor_MoveCursor(int key);
void Editor_Scroll();

#endif
