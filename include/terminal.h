#ifndef __FANO_TERMINAL__
#define __FANO_TERMINAL__

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "config.h"
#include "core.h"

void Terminal_EnableRawMode();
void Terminal_DisableRawMode();

#endif
