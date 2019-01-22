PROGRAM := krr
OUTPUT := krr

CC := gcc
EXE := 
FDIR := src/foundation
GLDIR := src/gl
UI_DIR := src/ui
EXTERDIR := src/externals

override CFLAGS += -std=c99 -Wall -Isrc/ -I/usr/local/include/SDL2 -I/Volumes/Slave/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/OpenGL.framework/Headers -I/usr/local/include/GL -I/usr/local/include/freetype2

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
	  $(UI_DIR)/button.o \
	  $(EXTERDIR)/vector.o \
	  $(GLDIR)/gl_util.o \
	  $(GLDIR)/gl_LTexture.o \
	  $(GLDIR)/gl_LSpritesheet.o \
	  $(GLDIR)/gl_LFont.o \
	  $(GLDIR)/gl_LShaderProgram.o \
	  $(GLDIR)/gl_LPlainPolygonProgram2D.o \
	  $(GLDIR)/gl_LMultiColorPolygonProgram2D.o \
	  $(GLDIR)/gl_ltextured_polygon_program2d.o \
	  $(GLDIR)/gl_lfont_polygon_program2d.o \
	  $(GLDIR)/gl_ldouble_multicolor_polygon_program2d.o \
	  usercode.o \
	  $(PROGRAM).o \
	  $(OUTPUT)

# targets for linking (just not include $(OUTPUT)
TARGETS_LINK := $(filter-out $(OUTPUT),$(TARGETS))

.PHONY: all clean

all: $(TARGETS) 
	
$(OUTPUT): $(TARGETS_LINK)
	$(CC) $^ -o $(OUTPUT)$(EXE) $(LFLAGS)

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

$(UI_DIR)/button.o: $(UI_DIR)/button.c $(UI_DIR)/button.h
	$(CC) $(CFLAGS) -c $< -o $@

$(VDRDIR)/vector.o: $(VDRDIR)/vector.c $(VDRDIR)/vector.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_util.o: $(GLDIR)/gl_util.c $(GLDIR)/gl_util.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LTexture.o: $(GLDIR)/gl_LTexture.c $(GLDIR)/gl_LTexture.h $(GLDIR)/gl_LTexture_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LSpritesheet.o: $(GLDIR)/gl_LTexture_spritesheet.c $(GLDIR)/gl_LTexture_spritesheet.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LFont.o: $(GLDIR)/gl_LFont.c $(GLDIR)/gl_LFont.h $(GLDIR)/gl_LFont_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LShaderProgram.o: $(GLDIR)/gl_LShaderProgram.c $(GLDIR)/gl_LShaderProgram.h $(GLDIR)/gl_LShaderProgram_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LPlainPolygonProgram2D.o: $(GLDIR)/gl_LPlainPolygonProgram2D.c $(GLDIR)/gl_LPlainPolygonProgram2D.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LMultiColorPolygonProgram2D.o: $(GLDIR)/gl_LMultiColorPolygonProgram2D.c $(GLDIR)/gl_LMultiColorPolygonProgram2D.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_ltextured_polygon_program2d.o: $(GLDIR)/gl_ltextured_polygon_program2d.c $(GLDIR)/gl_ltextured_polygon_program2d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_lfont_polygon_program2d.o: $(GLDIR)/gl_lfont_polygon_program2d.c $(GLDIR)/gl_lfont_polygon_program2d.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_ldouble_multicolor_polygon_program2d.o: $(GLDIR)/gl_ldouble_multicolor_polygon_program2d.c $(GLDIR)/gl_ldouble_multicolor_polygon_program2d.h
	$(CC) $(CFLAGS) -c $< -o $@

usercode.o: usercode.c usercode.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM).o: $(PROGRAM).c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf src/foundation/*.o
	rm -rf src/gl/*.o
	rm -rf $(OUTPUT)
	rm -rf *.o *.dSYM
