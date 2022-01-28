#ifndef __FONT2IMG_
#define __FONT2IMG_
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

int rendercharonscreen32(uint8_t* screen, int screenw, int screenh, char c, int x_l, int y_top, uint32_t color, int scale);


#endif
