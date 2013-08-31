TARGET     = libgolibc.a
STACKSIZE  =
MALLOCSIZE =
MMAREA     =
OBJS       = abs.o atof.o atoi.o errno.o frexp.o ldexp.o \
	memchr.o memcmp.o memmove.o memset.o qsort.o rand.o \
	sprintf.o strcat.o strcmp.o strcpy.o strcspn.o strdup.o \
	strlen.o strncat.o strncmp.o strncpy.o strpbrk.o strrchr.o \
	strspn.o strstr.o strtol.o strtoul.o strtoul0.o vsprintf.o

CC = /usr/bin/i686-pc-linux-gnu-gcc.exe
CFLAGS = -I. -fno-common -Wall -O2 -c \
         -finhibit-size-directive -fno-ident \
         -fomit-frame-pointer -fcall-used-ebx
AR = ar

ALL : ${OBJS}
	${AR} rv ${TARGET} ${OBJS}
	ranlib ${TARGET}

.c.o:
	${CC} ${CFLAGS} $<

clean :
	rm -f *.o ${TARGET}