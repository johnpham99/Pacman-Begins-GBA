#include "gba.h"
#include <stdio.h>

volatile unsigned short *videoBuffer = (volatile unsigned short *) 0x6000000;
u32 vBlankCounter = 0;

void waitForVBlank(void) {
  // (1)
  // Write a while loop that loops until we're NOT in vBlank anymore:
  // (This prevents counting one VBlank more than once if your app is too fast)
  while (SCANLINECOUNTER > 160);

  // (2)
  // Write a while loop that keeps going until we're in vBlank:
  while (SCANLINECOUNTER < 160);
  // (3)
  // Finally, increment the vBlank counter:
  vBlankCounter++;
}

static int __qran_seed = 42;
static int qran(void) {
  __qran_seed = 1664525 * __qran_seed + 1013904223;
  return (__qran_seed >> 16) & 0x7FFF;
}

int randint(int min, int max) { return (qran() * (max - min) >> 15) + min; }

void setPixel(int row, int col, u16 color) {
  *(videoBuffer + OFFSET(row, col, WIDTH) ) = color;
}

void drawRectDMA(int row, int col, int width, int height, volatile u16 color) {
  //is this lcolor necessary????
  volatile unsigned short lcolor;
  lcolor = color;
  for (int r = 0; r < height; r++) {
    DMA[3].src = &lcolor;
    DMA[3].dst = &videoBuffer[OFFSET(row+r,col,WIDTH)];
    DMA[3].cnt = width | DMA_ON | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT;
  }
}

void drawFullScreenImageDMA(const u16 *image) {
  // TODO: IMPLEMENT
  DMA[3].src = image;
  DMA[3].dst = videoBuffer;
  DMA[3].cnt = 240 * 160 | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT | DMA_ON;
}

void drawImageDMA(int row, int col, int width, int height, const u16 *image) {
  // TODO: IMPLEMENT
  for (int r = 0; r < height; r++) {
      DMA[3].src = &image[r*width];
      DMA[3].dst = &videoBuffer[OFFSET(row+r,col,WIDTH)];
      DMA[3].cnt = width | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT | DMA_ON;
  }
}

void fillScreenDMA(volatile u16 color) {
  // TODO: IMPLEMENT
  DMA[3].src = &color;
  DMA[3].dst = videoBuffer;
  DMA[3].cnt = 240 * 160 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;
}

void drawChar(int row, int col, char ch, u16 color) {
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 8; j++) {
      if (fontdata_6x8[OFFSET(j, i, 6) + ch * 48]) {
        setPixel(row + j, col + i, color);
      }
    }
  }
}

void drawString(int row, int col, char *str, u16 color) {
  while (*str) {
    drawChar(row, col, *str++, color);
    col += 6;
  }
}

void drawCenteredString(int row, int col, int width, int height, char *str, u16 color) {
  u32 len = 0;
  char *strCpy = str;
  while (*strCpy) {
    len++;
    strCpy++;
  }

  u32 strWidth = 6 * len;
  u32 strHeight = 8;

  int new_row = row + ((height - strHeight) >> 1);
  int new_col = col + ((width - strWidth) >> 1);
  drawString(new_row, new_col, str, color);
}

void drawPacman(int row, int col, volatile u16 color) {
  volatile unsigned short lcolor;
  lcolor = color;
  DMA[3].src = &lcolor;
  for (int r = 0; r < 13; r ++) {
    if (r == 0 || r == 12) {
      DMA[3].dst = &videoBuffer[OFFSET(row + r,col + 4,WIDTH)];
      DMA[3].cnt = 5 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;
      
    }
    if (r == 1 || r == 11) {
      DMA[3].dst = &videoBuffer[OFFSET(row + r,col + 2,WIDTH)];
      DMA[3].cnt = 9 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;    
    }
    if (r == 2 || r == 3 || r == 9 || r == 10) {
      DMA[3].dst = &videoBuffer[OFFSET(row + r,col + 1,WIDTH)];
      DMA[3].cnt = 11 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;    
    }
    if (r == 4 || r == 8) {
      DMA[3].dst = &videoBuffer[OFFSET(row + r,col,WIDTH)];
      DMA[3].cnt = 10 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;    
    }
    if (r == 5 || r == 7) {
      DMA[3].dst = &videoBuffer[OFFSET(row + r,col,WIDTH)];
      DMA[3].cnt = 7 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;    
    }
    if (r == 6) {
      DMA[3].dst = &videoBuffer[OFFSET(row + r,col,WIDTH)];
      DMA[3].cnt = 4 | DMA_SOURCE_FIXED | DMA_DESTINATION_INCREMENT | DMA_ON;    
    }
  }
}

void erasePacmanLeft(int row, int col, const u16 *image) {
  setPixel(row, col+4, image[row * WIDTH + (col+4)]);
  setPixel(row+1, col+2, image[(row+1) * WIDTH + (col+2)]);
  setPixel(row+2, col+1, image[(row+2) * WIDTH + (col+1)]);
  setPixel(row+3, col+1, image[(row+3) * WIDTH + (col+1)]);
  setPixel(row+4, col, image[(row+4) * WIDTH + (col)]);
  setPixel(row+5, col, image[(row+5) * WIDTH + (col)]);
  setPixel(row+6, col, image[(row+6) * WIDTH + (col)]);
  setPixel(row+7, col, image[(row+7) * WIDTH + (col)]);
  setPixel(row+8, col, image[(row+8) * WIDTH + (col)]);
  setPixel(row+9, col+1, image[(row+9) * WIDTH + (col+1)]);
  setPixel(row+10, col+1, image[(row+10) * WIDTH + (col+1)]);
  setPixel(row+11, col+2, image[(row+11) * WIDTH + (col+2)]);
  setPixel(row+12, col+4, image[(row+12) * WIDTH + (col+4)]);
}

void erasePacmanRight(int row, int col, const u16 *image) {
  setPixel(row, col+8, image[row * WIDTH + (col+8)]);
  setPixel(row+1, col+10, image[(row+1) * WIDTH + (col+10)]);
  setPixel(row+2, col+11, image[(row+2) * WIDTH + (col+11)]);
  setPixel(row+3, col+11, image[(row+3) * WIDTH + (col+11)]);
  setPixel(row+4, col+9, image[(row+4) * WIDTH + (col+9)]);
  setPixel(row+5, col+6, image[(row+5) * WIDTH + (col+6)]);
  setPixel(row+6, col+3, image[(row+6) * WIDTH + (col+3)]);
  setPixel(row+7, col+6, image[(row+7) * WIDTH + (col+6)]);
  setPixel(row+8, col+9, image[(row+8) * WIDTH + (col+9)]);
  setPixel(row+9, col+11, image[(row+9) * WIDTH + (col+11)]);
  setPixel(row+10, col+11, image[(row+10) * WIDTH + (col+11)]);
  setPixel(row+11, col+10, image[(row+11) * WIDTH + (col+10)]);
  setPixel(row+12, col+8, image[(row+12) * WIDTH + (col+8)]);
}

void erasePacmanDown(int row, int col, const u16 *image) {
  setPixel(row+8, col, image[(row+8) * WIDTH + col]);
  setPixel(row+10, col+1, image[(row+10) * WIDTH + (col+1)]);
  setPixel(row+11, col+2, image[(row+11) * WIDTH + (col+2)]);
  setPixel(row+11, col+3, image[(row+11) * WIDTH + (col+3)]);
  setPixel(row+12, col+4, image[(row+12) * WIDTH + (col+4)]);
  setPixel(row+12, col+5, image[(row+12) * WIDTH + (col+5)]);
  setPixel(row+12, col+6, image[(row+12) * WIDTH + (col+6)]);
  setPixel(row+12, col+7, image[(row+12) * WIDTH + (col+7)]);
  setPixel(row+12, col+8, image[(row+12) * WIDTH + (col+8)]);
  setPixel(row+11, col+9, image[(row+11) * WIDTH + (col+9)]);
  setPixel(row+11, col+10, image[(row+11) * WIDTH + (col+10)]);
  setPixel(row+10, col+11, image[(row+10) * WIDTH + (col+11)]);

  setPixel(row+5, col+4, image[(row+5) * WIDTH + (col+4)]);
  setPixel(row+5, col+5, image[(row+5) * WIDTH + (col+5)]);
  setPixel(row+5, col+6, image[(row+5) * WIDTH + (col+6)]);

  setPixel(row+4, col+7, image[(row+4) * WIDTH + (col+7)]);
  setPixel(row+4, col+8, image[(row+4) * WIDTH + (col+8)]);
  setPixel(row+4, col+9, image[(row+4) * WIDTH + (col+9)]);

  setPixel(row+3, col+10, image[(row+3) * WIDTH + (col+10)]);
  setPixel(row+3, col+11, image[(row+3) * WIDTH + (col+11)]);
}

void erasePacmanUp(int row, int col, const u16 *image) {
  setPixel(row, col+4, image[(row) * WIDTH + (col+4)]);
  setPixel(row, col+5, image[(row) * WIDTH + (col+5)]);
  setPixel(row, col+6, image[(row) * WIDTH + (col+6)]);
  setPixel(row, col+7, image[(row) * WIDTH + (col+7)]);
  setPixel(row, col+8, image[(row) * WIDTH + (col+8)]);  

  setPixel(row+1, col+2, image[(row+1) * WIDTH + (col+2)]);
  setPixel(row+1, col+3, image[(row+1) * WIDTH + (col+3)]);
  setPixel(row+1, col+9, image[(row+1) * WIDTH + (col+9)]);
  setPixel(row+1, col+10, image[(row+1) * WIDTH + (col+10)]);

  setPixel(row+2, col+1, image[(row+2) * WIDTH + (col+1)]);
  setPixel(row+2, col+11, image[(row+2) * WIDTH + (col+11)]);

  setPixel(row+4, col, image[(row+4) * WIDTH + col]);
  setPixel(row+7, col+4, image[(row+7) * WIDTH + (col+4)]);
  setPixel(row+7, col+5, image[(row+7) * WIDTH + (col+5)]);
  setPixel(row+7, col+6, image[(row+7) * WIDTH + (col+6)]);
  setPixel(row+8, col+7, image[(row+8) * WIDTH + (col+7)]);
  setPixel(row+8, col+8, image[(row+8) * WIDTH + (col+8)]);
  setPixel(row+8, col+9, image[(row+8) * WIDTH + (col+9)]);
  setPixel(row+9, col+10, image[(row+9) * WIDTH + (col+10)]);
  setPixel(row+9, col+11, image[(row+9) * WIDTH + (col+11)]);
}

void moveRight(int row, int col, volatile u16 color, const u16 *image) {
  drawPacman(row, col+1, color);
  erasePacmanLeft(row, col, image);
}

void moveLeft(int row, int col, volatile u16 color, const u16 *image) {
  drawPacman(row, col-1, color);
  erasePacmanRight(row, col, image);
}

void moveUp(int row, int col, volatile u16 color, const u16 *image) {
  drawPacman(row-1, col, color);
  erasePacmanDown(row, col, image);
}

void moveDown(int row, int col, volatile u16 color, const u16 *image) {
  drawPacman(row+1, col, color);
  erasePacmanUp(row, col, image);
}

int checkCollision(int pacRow, int pacCol, int wallRow, int wallCol, int wallWidth, int wallHeight) {
  if (pacCol >= wallCol && pacCol <= wallCol+wallWidth && pacRow <= wallRow+wallHeight && wallRow <= pacRow) {
    return 1;
  }
  if (pacCol >= wallCol && pacCol <= wallCol+wallWidth && pacRow + 12 <= wallRow+wallHeight && wallRow <= pacRow + 12) {
    return 1;
  }
  if (pacRow >= wallRow && pacRow <= wallRow+wallHeight && pacCol <= wallCol+wallWidth && wallCol <= pacCol) {
    return 1;
  }
  if (pacRow >= wallRow && pacRow <= wallRow+wallHeight && pacCol + 11 <= wallCol+wallWidth && wallCol <= pacCol + 11) {
    return 1;
  }
  return 0;
}

void eraseDot(int row, int col, const u16 *image) {
  setPixel(row, col, image[(row) * WIDTH + (col)]); 
  setPixel(row+1, col, image[(row+1) * WIDTH + (col)]);
  setPixel(row+2, col, image[(row+2) * WIDTH + (col)]);
  setPixel(row, col+1, image[(row) * WIDTH + (col+1)]); 
  setPixel(row+1, col+1, image[(row+1) * WIDTH + (col+1)]);
  setPixel(row+2, col+1, image[(row+2) * WIDTH + (col+1)]);
  setPixel(row, col+2, image[(row) * WIDTH + (col+2)]);
  setPixel(row+1, col+2, image[(row+1) * WIDTH + (col+2)]);
  setPixel(row+2, col+2, image[(row+2) * WIDTH + (col+2)]);
}

void drawPresses(int counter, int row, int col, volatile u16 color) {
  volatile unsigned short lcolor;
  lcolor = color;
  counter++;
  char s[50];
  snprintf(s, 50, "%d", counter);
  drawString(row, col, s, lcolor);
}
