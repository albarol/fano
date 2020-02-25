#ifndef __FANO_EDITOR__
#define __FANO_EDITOR__

#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "config.h"
#include "core.h"

void Editor_Init();

void Editor_DrawRows();
void Editor_ProcessKeyPress();
char Editor_ReadKey();
void Editor_RefreshScreen();

int Editor_GetWindowSize(int *rows, int *cols);
int Editor_GetCursorPosition(int *rows, int *cols);

#endif
