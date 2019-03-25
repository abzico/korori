CC = gcc
CFLAGS = -g \
				 -O2 \
				 -I./include \
				 -I../include \
				 -I/usr/local/include/freetype2
LDFLAGS =	-lSDL2 \
					-lSDL2_image \
					-lGLEW \
					-lfreetype \
					-lcglm \
					-lvector \
					-lhashmap_c \
					-ltexpackr \
					main.o functs.o

if CD_IS_MACOSX
CFLAGS += -DGL_SILENCE_DEPRECATION
LDFLAGS += -framework OpenGL \
					 ../.libs/libkrr.a
endif

if CD_IS_WINDOWS
LDFLAGS += -lGL \
					 ../.libs/libkrr.lib
endif

if CD_IS_UNIX
LDFLAGS += -lGL \
					 ../.libs/libkrr.a
endif

samples-local: copy-res common \
							 sample-terrain
	@echo "All samples are built"

copy-res:
	cp -R ../res res

common: main.c functs.c functs.h usercode.h
	@echo "Build common"
if CD_IS_MACOSX
	glibtool --tag=CC --mode=compile $(CC) $(CFLAGS) -c main.c
	glibtool --tag=CC --mode=compile $(CC) $(CFLAGS) -c functs.c
else
	libtool --tag=CC --mode=compile $(CC) $(CFLAGS) -c main.c
	libtool --tag=CC --mode=compile $(CC) $(CFLAGS) -c functs.c
endif

sample-terrain:
	@echo "Build terrain"
	$(CC) $(CFLAGS) terrain.c $(LDFLAGS) -o terrain$(EXEEXT)
