#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

typedef struct OutputTarget {
  size_t len, limits;
  char *buf;
  int (*putch)(struct OutputTarget *, char);
} OutputTarget;

int putch2screen(OutputTarget *ot, char ch) {
  putch(ch);
  ot->len++;
  return 1;
}
int putch2buf(OutputTarget *ot, char ch) {
  ot->buf[ot->len] = ch;
  if (ot->len < ot->limits) ot->len++;
  return 1;
}

#define PUTCH(ch) ot->putch(ot, ch);
#define PUTS(s) _puts(ot, s);

static inline void _puts(OutputTarget *ot, const char *s) {
  for (; *s != 0; s++) PUTCH(*s);
  return;
}

typedef struct Specifier {
  int width;
  bool zeropadding;
} Specifier;

static inline void specifier_clear(Specifier *spec) {
  spec->width = 0;
  spec->zeropadding = false;
  return;
}

static inline void print_ch(OutputTarget *ot, Specifier *spec, const int ch) {
  for (size_t i = 1; i < spec->width; i++) PUTCH(' ');
  PUTCH(ch);
}

static inline void print_str(OutputTarget *ot, Specifier *spec,
                             const char *str) {
  for (size_t i = strlen(str); i < spec->width; i++) PUTCH(' ');
  PUTS(str);
}

static inline void print_uint32(OutputTarget *ot, Specifier *spec,
                                uint32_t num) {
  uint32_t len = 0, buf[11];
  if (num == 0) buf[len++] = '0';
  while (num > 0) {
    buf[len++] = num % 10 + '0';
    num = num / 10;
  }
  for (size_t i = len; i < spec->width; i++)
    PUTCH(spec->zeropadding ? '0' : ' ');
  while (len--) PUTCH(buf[len]);
  return;
}

static inline void print_int32(OutputTarget *ot, Specifier *spec, int32_t num) {
  uint32_t len = 0;
  uint8_t buf[10];
  if (num == 0) buf[len++] = '0';
  if (num < 0) {
    PUTCH('-');
    if(spec->width > 0) spec->width--;
    num = -num;
  }
  while (num > 0) {
    buf[len++] = num % 10 + '0';
    num = num / 10;
  }
  for (size_t i = len; i < spec->width; i++)
    PUTCH(spec->zeropadding ? '0' : ' ');
  while (len--) PUTCH(buf[len]);
  return;
}

static inline void print_hex32(OutputTarget *ot, Specifier *spec,
                               uint32_t num) {
  uint32_t len = 0;
  uint8_t buf[8];
  if (num == 0) buf[len++] = '0';
  while (num > 0) {
    buf[len] = num % 16;
    buf[len] += buf[len] > 9 ? 'a' - 10 : '0';
    len++;
    num = num / 16;
  }
  for (size_t i = len; i < spec->width; i++)
    PUTCH(spec->zeropadding ? '0' : ' ');
  while (len--) PUTCH(buf[len]);
  return;
}

static inline void print_hex64(OutputTarget *ot, Specifier *spec,
                               uint64_t num) {
  uint32_t len = 0;
  uint8_t buf[16];
  if (num == 0) buf[len++] = '0';
  while (num > 0) {
    buf[len] = num % 16;
    buf[len] += buf[len] > 9 ? 'a' - 10 : '0';
    len++;
    num = num / 16;
  }
  for (size_t i = len; i < spec->width; i++)
    PUTCH(spec->zeropadding ? '0' : ' ');
  while (len--) PUTCH(buf[len]);
  return;
}

static inline int _vprintf(OutputTarget *ot, const char *fmt, va_list ap) {
  Specifier spec;

  while (*fmt != 0) {
    if (*fmt != '%') {
      PUTCH(*fmt);
      fmt++;
      continue;
    }
    fmt++;

    specifier_clear(&spec);
    while (true) {
      if (*fmt == 0) panic("Unexpected end of format string");
      switch (*fmt) {
      case '%':
        PUTCH('%');
        goto end;
      case '0':
        spec.zeropadding = true;
        fmt++;
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        spec.width = 0;
        while (*fmt > '0' && *fmt <= '9') {
          spec.width = spec.width * 10 + *fmt - '0';
          fmt++;
        }
        break;
      case 'c':
        print_ch(ot, &spec, va_arg(ap, int));
        goto end;
      case 's':
        print_str(ot, &spec, va_arg(ap, char *));
        goto end;
      case 'u':
        print_uint32(ot, &spec, va_arg(ap, uint32_t));
        goto end;
      case 'd':
        print_int32(ot, &spec, va_arg(ap, uint32_t));
        goto end;
      case 'x':
        print_hex32(ot, &spec, va_arg(ap, uint32_t));
        goto end;
      case 'p': {
        int min_width = sizeof(void *) * 2;
        spec.width = spec.width > min_width ? spec.width : min_width;
        if (sizeof(void *) == 4)
          print_hex32(ot, &spec, (uintptr_t)va_arg(ap, void *));
        else
          print_hex64(ot, &spec, (uintptr_t)va_arg(ap, void *));
        goto end;
      }
      default:
        PUTCH('\n');
        PUTS("Unsupported format: ");
        PUTS(fmt);
        PUTCH('\n');
        panic("Unsupported format");
      }
    }
  end:
    fmt++;
  }
  return 1;
}

#undef PUTCH
#undef PUTDEC
#undef PUTS

int printf(const char *fmt, ...) {
  OutputTarget ot;
  ot.len = 0;
  ot.putch = putch2screen;
  va_list ap;
  va_start(ap, fmt);
  _vprintf(&ot, fmt, ap);
  va_end(ap);
  return ot.len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, -1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int res = vsprintf(out, fmt, ap);
  va_end(ap);
  return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int res = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return res;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  OutputTarget ot;
  ot.len = 0;
  ot.buf = out;
  ot.limits = n;
  ot.putch = putch2buf;
  _vprintf(&ot, fmt, ap);
  out[ot.len] = 0;
  return ot.len;
}

#endif

