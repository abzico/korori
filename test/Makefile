# Makefile to build the test samples for KRR

srcdir = .

CC = gcc
override CFLAGS += -std=c99 -g -Wall -I. -I../src -I/usr/local/include/SDL2 -I/Volumes/Slave/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/OpenGL.framework/Headers -I/usr/local/include/GL -I/usr/local/include/freetype2
LIBKRR_FLAGS = -L../ -lkrr

# add platform specific
UNAME_S := $(shell uname -s)
# (only for macOS) use -DGL_SILENCE_DEPRECATION to silence deprecation warnings as if running on macOS will generate ton of it because the platform deprecated opengl
ifeq ($(UNAME_S),Darwin)
        CFLAGS += -DGL_SILENCE_DEPRECATION
endif

LFLAGS += -lSDL2 -lSDL2_image -lSDL2_mixer -framework OpenGL -lGLEW -lfreetype -lcglm

TARGETS = \
	  template	\
	  doublemulticolorshader	\
	  rotatingplane	\
	  rotatingcube	\
	  readobjfile_manual	\
	  readobjfile

DEPS = \
       usercode.h	\
       libkrr.a		\
       main.o		\
       copyresfiles

all: libkrr.a main.o $(TARGETS) copyresfiles

main.o: $(srcdir)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

libkrr.a:
	# change directory then make static library in which output is at $(libkrrpath)
	cd .. && make

template: $(srcdir)/template.c $(DEPS)
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

doublemulticolorshader: $(srcdir)/doublemulticolorshader.c usercode.h libkrr.a main.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

rotatingplane: $(srcdir)/rotatingplane.c usercode.h libkrr.a main.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

rotatingcube: $(srcdir)/rotatingcube.c usercode.h libkrr.a main.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

readobjfile_manual: $(srcdir)/readobjfile_manual.c usercode.h libkrr.a main.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

readobjfile: $(srcdir)/readobjfile.c usercode.h libkrr.a main.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

copyresfiles:
	cp -pR ../res ./

clean:
	rm -f $(TARGETS)
	rm -f *.o
	rm -rf ./res
