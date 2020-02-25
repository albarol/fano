
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define CTRL_KEY(k) ((k) & 0x1f)

void die(const char *s);
void refreshScreen();
