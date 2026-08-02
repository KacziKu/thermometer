#include "Buffer.h"
#include <stdio.h>

void addToBuffer(float value, struct CycleBuffer *buff) {
  buff->buffer[buff->head] = value;
  buff->head = (buff->head + 1) % BUFFER_SIZE;
  if (buff->count < BUFFER_SIZE) {
    buff->count++;
  }
}

void logBuff(struct CycleBuffer *buff) {
  printf("HEAD: %d, COUNT: %d, BUFF: ", buff->head, buff->count);
  for(int i = 0; i < BUFFER_SIZE; i++) {
    printf("%.2f ", buff->buffer[i]);
  }
  printf("\n");
}
