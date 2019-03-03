CC := gcc
EXE := 
FDIR := src/foundation
GPDIR := src/graphics
UI_DIR := src/ui
EXTERDIR := src/externals

override CFLAGS += -std=c99 -g -Wall -Isrc/ -I/usr/local/include/SDL2 -I/Volumes/Slave/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/OpenGL.framework/Headers -I/usr/local/include/GL -I/usr/local/include/freetype2

# add platform specific cflags
UNAME_S := $(shell uname -s)
# (only for macOS) use -DGL_SILENCE_DEPRECATION to silence deprecation warnings as if running on macOS will generate ton of it because the platform deprecated opengl
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -DGL_SILENCE_DEPRECATION
endif

# assume you install cglm on your system
override LFLAGS += -lSDL2 -lSDL2_image -lSDL2_mixer -framework OpenGL -lGLEW -lfreetype -lcglm
TARGETS := \
	  $(FDIR)/common.o \
	  $(FDIR)/math.o \
	  $(FDIR)/util.o \
	  $(FDIR)/window.o \
	  $(FDIR)/timer.o \
	  $(FDIR)/mem.o	\
	  $(UI_DIR)/button.o \
	  $(EXTERDIR)/vector.o \
	  $(GPDIR)/util.o \
	  $(GPDIR)/texture.o \
	  $(GPDIR)/spritesheet.o \
	  $(GPDIR)/font.o \
	  $(GPDIR)/shaderprog.o \
	  $(GPDIR)/texturedpp2d.o \
	  $(GPDIR)/texturedpp3d.o \
	  $(GPDIR)/texturedalphapp3d.o \
	  $(GPDIR)/fontpp2d.o \
	  $(GPDIR)/objloader.o \
	  $(GPDIR)/model.o	\
	  $(GPDIR)/terrain_shader3d.o	\
	  $(GPDIR)/terrain.o

.PHONY: all clean

all: $(TARGETS) 
	# create a static library
	ar rcs libkrr.a $^

$(FDIR)/common.o: $(FDIR)/common.c $(FDIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/math.o: $(FDIR)/math.c $(FDIR)/math.h $(FDIR)/types.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/util.o: $(FDIR)/util.c $(FDIR)/util.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/window.o: $(FDIR)/window.c $(FDIR)/window.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/timer.o: $(FDIR)/timer.c $(FDIR)/timer.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/mem.o: $(FDIR)/mem.c $(FDIR)/mem.h
	$(CC) $(CFLAGS) -c $< -o $@

$(UI_DIR)/button.o: $(UI_DIR)/button.c $(UI_DIR)/button.h
	$(CC) $(CFLAGS) -c $< -o $@

$(VDRDIR)/vector.o: $(VDRDIR)/vector.c $(VDRDIR)/vector.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/util.o: $(GPDIR)/util.c $(GPDIR)/util.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/texture.o: $(GPDIR)/texture.c $(GPDIR)/texture.h $(GPDIR)/texture_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/spritesheet.o: $(GPDIR)/spritesheet.c $(GPDIR)/spritesheet.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/font.o: $(GPDIR)/font.c $(GPDIR)/font.h $(GPDIR)/font_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/shaderprog.o: $(GPDIR)/shaderprog.c $(GPDIR)/shaderprog.h $(GPDIR)/shaderprog_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/texturedpp2d.o: $(GPDIR)/texturedpp2d.c $(GPDIR)/texturedpp2d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/texturedpp3d.o: $(GPDIR)/texturedpp3d.c $(GPDIR)/texturedpp3d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/texturedalphapp3d.o: $(GPDIR)/texturedalphapp3d.c $(GPDIR)/texturedalphapp3d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/fontpp2d.o: $(GPDIR)/fontpp2d.c $(GPDIR)/fontpp2d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/objloader.o: $(GPDIR)/objloader.c $(GPDIR)/objloader.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/model.o: $(GPDIR)/model.c $(GPDIR)/model.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/terrain_shader3d.o: $(GPDIR)/terrain_shader3d.c $(GPDIR)/terrain_shader3d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GPDIR)/terrain.o: $(GPDIR)/terrain.c $(GPDIR)/terrain.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM).o: $(PROGRAM).c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/foundation/*.o
	rm -f src/graphics/*.o
	rm -f src/externals/*.o
	rm -f src/ui/*.o
	rm -f $(OUTPUT)
	rm -f libkrr.a
	rm -f *.o *.dSYM
