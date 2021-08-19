#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char* s) {
  size_t len = 0;
  while (s[len]) len++;
  return len;
}

char* strcpy(char* dst, const char* src) {
  // may not overlap
  size_t i = 0;
  for (i = 0; src[i] != 0; i++) dst[i] = src[i];
  dst[i] = 0;
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  // may not overlap
  size_t i = 0;
  for (i = 0; i < n && src[i] != 0; i++) dst[i] = src[i];
  for (; i < n; i++) dst[i] = 0;
  return dst;
}

char* strcat(char* dst, const char* src) {
  // may not overlap
  size_t i, j;
  for (i = 0; dst[i] != 0; i++)
    ;
  for (j = 0; src[j] != 0; j++) dst[i + j] = src[j];
  dst[i + j] = 0;
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  for (size_t i = 0; s1[i] != 0 || s2[i] != 0; i++)
    if (s1[i] < s2[i])
      return -1;
    else if (s1[i] > s2[i])
      return 1;
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  for (size_t i = 0; i < n && (s1[i] != 0 || s2[i] != 0); i++)
    if (s1[i] < s2[i])
      return -1;
    else if (s1[i] > s2[i])
      return 1;
  return 0;
}

void* memset(void* v, int c, size_t n) {
  while (n--) ((char*)v)[n] = (char)c;
  return v;
}

void* memmove(void* dst, const void* src, size_t n) {
  // may overlap
  if (dst < src) {
    for (size_t i = 0; i < n; i++) ((char*)dst)[i] = ((char*)src)[i];
  } else if (dst > src) {
    while (n--) ((char*)dst)[n] = ((char*)src)[n];
  }
  return dst;
}

void* memcpy(void* out, const void* in, size_t n) {
  // may not overlap
  while (n--) ((char*)out)[n] = ((char*)in)[n];
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (((unsigned char*)s1)[i] < ((unsigned char*)s2)[i])
      return -1;
    else if (((unsigned char*)s1)[i] > ((unsigned char*)s2)[i])
      return 1;
  return 0;
}

#endif
