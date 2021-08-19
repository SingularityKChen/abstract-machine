#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  static void *program_break = 0;
  if (program_break == 0) {
    if (heap.start == 0) return 0;
    program_break = heap.start;
    //#endif
  }
  size = (size + 15) & ~15;
  void *mem = program_break;
  program_break += size;
  //assert(program_break <= heap.end, "Run out of memory");
  return mem;
}

void *realloc(void *ptr, size_t size) {
  if (size == 0) {
    free(ptr);
    return NULL;
  }
  void *new_ptr = malloc(size);
  if (ptr != 0) {
    memcpy(new_ptr, ptr, size);
    free(ptr);
  }
  return new_ptr;
}

void free(void *ptr) {
}

#endif
