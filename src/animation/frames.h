#ifndef ANIMATION_FRAMES_H
#define ANIMATION_FRAMES_H

#include <stdint.h>

void updateFramePos(int diff);

const uint16_t *getFrameAddr();

#endif // ANIMATION_FRAMES_H
