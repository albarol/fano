#ifndef __FANO_EDITOR__
#define __FANO_EDITOR__

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "cons.h"
#include "core.h"
#include "screen.h"

void Editor_Init();
void Editor_Open(char* filename);
void Editor_Save();

void Editor_InsertRow(int at, char *s, size_t len);
void Editor_UpdateRow(editorRow *row);
void Editor_AppendCharAtRow(editorRow *row, int at, int c);
void Editor_RemoveCharAtRow(editorRow *row, int at);
void Editor_InsertChar(int c);
void Editor_RowAppendString(editorRow *row, char *s, size_t len);
void Editor_RemoveChar();
void Editor_FreeRow(editorRow *row);
void Editor_DeleteRow(int at);
char *Editor_GetRowsAsString(int *bufferLength);

void Editor_DrawRows(struct buffer *pBuffer);

void Editor_ProcessKeyPress();
int Editor_ReadKey();
char *Editor_Prompt(char *prompt);


#endif
