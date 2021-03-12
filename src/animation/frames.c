#include "frames.h"
#include "screen.h"

#define FRAMES_N 7
#define FRAMES_LEN FRAMES_N * LCD_SIZE

/* Frames in 16-bit RGB encoding. */
static const uint16_t frames[FRAMES_LEN] = {
#include "./frames_all.txt"
};

/* Current frame position. */
static int framesPos = 0;

/* Internal functions. */

static int getOffset(int pos) {
  return pos * LCD_SIZE;
}

/* Public interface implementations. */

void updateFramePos(int diff) {
  framesPos = (framesPos + diff + FRAMES_N) % FRAMES_N;
}

const uint16_t *getFrameAddr() {
  return &frames[getOffset(framesPos)];
}



