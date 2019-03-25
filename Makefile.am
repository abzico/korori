ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wall \
	    -std=c99 \
	    -O2 \
	    -pedantic

EXTRA_DIST = README.md LICENSE autogen.sh res

# lib
lib_LTLIBRARIES = libkrr.la
# FIXME: hard-coded freetype's lib path as the library uses <> that causes us problem in linking
libkrr_la_CFLAGS = -I./include -I/usr/local/include/freetype2
libkrr_la_LDFLAGS = -no-defined -version-info 0:1:0 \
		    -lSDL2 \
		    -lSDL2_image \
		    -lSDL2_mixer \
		    -lGLEW \
		    -lfreetype \
		    -lcglm \
		    -lvector \
		    -lhashmap_c \
		    -ltexpackr
libkrr_la_SOURCES = src/foundation/common.c \
		    src/foundation/math.c \
		    src/foundation/mem.c \
		    src/foundation/timer.c \
		    src/foundation/util.c \
		    src/foundation/window.c \
		    src/graphics/font.c \
		    src/graphics/fontpp2d.c \
		    src/graphics/model.c \
		    src/graphics/objloader.c \
		    src/graphics/shaderprog.c \
		    src/graphics/spritesheet.c \
		    src/graphics/terrain.c \
		    src/graphics/terrain_shader3d.c \
		    src/graphics/texture.c \
		    src/graphics/texturedalphapp3d.c \
		    src/graphics/texturedpp3d.c \
		    src/graphics/util.c \
		    src/ui/button.c

krr_foundationdir=$(includedir)/krr/foundation
krr_foundation_HEADERS = include/krr/foundation/common.h \
			 include/krr/foundation/cam.h \
			 include/krr/foundation/common_debug.h \
			 include/krr/foundation/log.h \
			 include/krr/foundation/math.h \
			 include/krr/foundation/mem.h \
			 include/krr/foundation/timer.h \
			 include/krr/foundation/types.h \
			 include/krr/foundation/util.h \
			 include/krr/foundation/window.h

krr_graphicsdir=$(includedir)/krr/graphics
krr_graphics_HEADERS = include/krr/graphics/common.h \
		       include/krr/graphics/font.h \
		       include/krr/graphics/font_internals.h \
		       include/krr/graphics/fontpp2d.h \
		       include/krr/graphics/model.h \
		       include/krr/graphics/objloader.h \
		       include/krr/graphics/shaderprog.h \
		       include/krr/graphics/shaderprog_internals.h \
		       include/krr/graphics/spritesheet.h \
		       include/krr/graphics/terrain.h \
		       include/krr/graphics/terrain_shader3d.h \
		       include/krr/graphics/texture.h \
		       include/krr/graphics/texture_internals.h \
		       include/krr/graphics/texturedalphapp3d.h \
		       include/krr/graphics/texturedpp2d.h \
		       include/krr/graphics/texturedpp3d.h \
		       include/krr/graphics/types.h \
		       include/krr/graphics/util.h

krr_uidir=$(includedir)/krr/ui
krr_ui_HEADERS = include/krr/ui/button.h
