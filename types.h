/* types.h - Basic type definitions and utilities */
#ifndef TYPES_H
#define TYPES_H

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef int            int32_t;
typedef short          int16_t;
typedef char           int8_t;

typedef uint32_t size_t;

#define NULL  ((void*)0)

// String and memory utilities (inline for efficiency)
static inline size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static inline int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

static inline char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

static inline void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

static inline void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

#endif
