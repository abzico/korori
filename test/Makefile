# Makefile to build the test samples for KRR

srcdir = .

CC = gcc
override CFLAGS += -std=c99 -g -Wall -I. -I../src -I/usr/local/include/SDL2 -I/Volumes/Slave/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/OpenGL.framework/Headers -I/usr/local/include/GL -I/usr/local/include/freetype2 -I../src/externals/texpackr/include -I../src/externals/hashmap_c/include
LIBKRR_FLAGS = -L../ -lkrr -lpng

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
	  readobjfile	\
	  headless	\
	  advanced_model	\
	  terrain

DEPS = \
       usercode.h	\
       libkrr.a		\
       main.o		\
       functs.o		\
       copyresfiles

all: libkrr.a main.o functs.o $(TARGETS) copyresfiles

main.o: $(srcdir)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

functs.o: $(srcdir)/functs.c $(srcdir)/functs.h
	$(CC) $(CFLAGS) -c $< -o $@

main-headless.o: $(srcdir)/main.c
	$(CC) $(CFLAGS) -DSDL_HEADLESS -c $< -o $@

libkrr.a:
	# change directory then make static library in which output is at $(libkrrpath)
	cd .. && make

template: $(srcdir)/template.c $(DEPS)
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

doublemulticolorshader: $(srcdir)/doublemulticolorshader.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

rotatingplane: $(srcdir)/rotatingplane.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

rotatingcube: $(srcdir)/rotatingcube.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

readobjfile_manual: $(srcdir)/readobjfile_manual.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

readobjfile: $(srcdir)/readobjfile.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

headless: $(srcdir)/headless.c usercode.h libkrr.a main-headless.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -DSDL_HEADLESS -o $@.o
	$(CC) $@.o main-headless.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

advanced_model: $(srcdir)/advanced_model.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

terrain: $(srcdir)/terrain.c usercode.h libkrr.a main.o functs.o copyresfiles
	@echo "\nbuilding $@"
	$(CC) -c $< $(CFLAGS) -o $@.o
	$(CC) $@.o main.o functs.o $(LIBKRR_FLAGS) -o $@ $(LFLAGS)

copyresfiles:
	cp -pR ../res ./

clean:
	rm -f $(TARGETS)
	rm -f *.o
	rm -rf ./res

clean-all: clean
	make clean -C ../
