
#ifndef _terimber_osinc_h_
#define _terimber_osinc_h_

#define _POSIX_SOURCE 1

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <unistd.h>
#include <sys/filio.h>

// memcpy
extern "C"
{
void* memcpy(void*, const void*, size_t);
void* memset(void*, int, size_t);
void* memmove(void* d, const void*, size_t);
int memcmp(const void*, const void*, size_t);
void* memchr(const void*, int, size_t);
char* strcpy(char*, const char*);
char* strncpy(char*, const char*, size_t);
char* strcat(char*, const char*);
char* strncat(char*, const char*, size_t);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);
int strcoll(const char*, const char*);
size_t strxfrm(char*, const char*, size_t);
size_t strcspn(const char*, const char*);
size_t strspn(const char*, const char*);
char* strtok(char*, const char*);
char* strerror(int);
size_t strlen(const char*);
char* strchr(const char*, int);
char* strrchr(const char*, int);
char* strpbrk(const char*, const char*);
char* strstr(const char*, const char*);
}




#include <sys/timeb.h>
#include <math.h>
#include <assert.h>
#include <netinet/in.h>
#include <algorithm>

#include "osdef.h"
#include "ostypes.h"
#include "oserror.h"
#include "ossock.h"

#endif
