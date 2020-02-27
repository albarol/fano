#ifndef __FANO_SCREEN__
#define __FANO_SCREEN__

#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#include "editor.h"
#include "core.h"
#include "config.h"
#include "cons.h"

void Screen_RefreshScreen();
int Screen_GetWindowSize(int *rows, int *cols);
int Screen_GetCursorPosition(int *rows, int *cols);
void Screen_MoveCursor(int key);
void Screen_Scroll();

#endif
