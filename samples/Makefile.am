CC = gcc
CFLAGS = -g \
				 -O2 \
				 -std=c99 \
				 -I./include \
				 -I../include

if CD_IS_WINDOWS
LINK_KRR_FLAGS = ../.libs/libkrr.lib
else
LINK_KRR_FLAGS = ../.libs/libkrr.a
endif

LDFLAGS =	main.o functs.o \
					$(LINK_KRR_FLAGS) \
					-lm \
					-lSDL2 \
					-lSDL2_image \
					-lGLEW \
					-lfreetype \
					-lcglm \
					-lvector \
					-lhashmap_c \
					-ltexpackr

LDFLAGS_HEADLESS = main-headless.o functs-headless.o \
		   $(LINK_KRR_FLAGS) \
		   -lm \
		   -lSDL2 \
		   -lSDL2_image \
		   -lGLEW \
		   -lfreetype \
		   -lcglm \
		   -lvector \
		   -lhashmap_c \
		   -ltexpackr

dist_data_DATA = template.c \
								 main.c \
								 functs.c functs.h \
								 usercode.h \
								 doublemulticolorshader.c \
								 rotatingcube.c \
								 rotatingplane.c \
								 readobjfile_manual.c \
								 readobjfile.c \
								 advanced_model.c \
								 headless.c \
								 terrain.c \
								 dds.c

if CD_IS_MACOSX
CFLAGS += -DGL_SILENCE_DEPRECATION \
					-I/usr/local/include/freetype2
LDFLAGS += -framework OpenGL \
					 -Wl,-rpath,/usr/local/lib
LDFLAGS_HEADLESS += -framework OpenGL \
		    -Wl,-rpath,/usr/local/lib
endif

if CD_IS_WINDOWS
CFLAGS += -I/usr/include/freetype2
LDFLAGS += -lGL \
					 -Wl,-rpath,/usr/local/lib
LDFLAGS_HEADLESS += -lGL \
		    -Wl,-rpath,/usr/local/lib
endif

if CD_IS_UNIX
CFLAGS += -I/usr/include/freetype2
LDFLAGS += -lGL \
					 -Wl,-rpath,/usr/local/lib
LDFLAGS_HEADLESS += -lGL \
		    -Wl,-rpath,/usr/local/lib
endif

clean-samples-local:
	rm -f *.o
	rm -f *.lo
	rm -rf *.dSYM
	rm -f .libs/*
	rm -rf res
	rm -f doublemulticolorshader
	rm -f rotatingcube
	rm -f rotatingplane
	rm -f readobjfile_manual
	rm -f readobjfile
	rm -f advanced_model
	rm -f headless
	rm -f terrain

samples-local: copy-res common common-headless \
							 sample-doublemulticolorshader-local \
							 sample-rotatingcube-local \
							 sample-rotatingplane-local \
							 sample-readobjfile_manual-local \
							 sample-readobjfile-local \
							 sample-advanced_model-local \
							 sample-headless-local \
							 sample-terrain-local \
							 sample-dds-local
	@echo "All samples are built"

copy-res:
	rsync -av ../res ./

common: main.c functs.c functs.h usercode.h
	@echo "Build common"
	$(CC) $(CFLAGS) -c main.c
	$(CC) $(CFLAGS) -c functs.c

common-headless: main.c functs.c functs.h usercode.h
	@echo "Build common headless"
	$(CC) $(CFLAGS) -DSDL_HEADLESS -c main.c -o main-headless.o
	$(CC) $(CFLAGS) -DSDL_HEADLESS -c functs.c -o functs-headless.o

sample-doublemulticolorshader-local: copy-res common
	@echo "Build doublemulticolorshader"
	$(CC) $(CFLAGS) doublemulticolorshader.c $(LDFLAGS) -o doublemulticolorshader$(EXEEXT)

sample-rotatingcube-local: copy-res common
	@echo "Build rotatingcube"
	$(CC) $(CFLAGS) rotatingcube.c $(LDFLAGS) -o rotatingcube$(EXEEXT)

sample-rotatingplane-local: copy-res common
	@echo "Build rotatingplane"
	$(CC) $(CFLAGS) rotatingplane.c $(LDFLAGS) -o rotatingplane$(EXEEXT)

sample-readobjfile_manual-local: copy-res common
	@echo "Build readobjfile_manual"
	$(CC) $(CFLAGS) readobjfile_manual.c $(LDFLAGS) -o readobjfile_manual$(EXEEXT)

sample-readobjfile-local: copy-res common
	@echo "Build readobjfile"
	$(CC) $(CFLAGS) readobjfile.c $(LDFLAGS) -o readobjfile$(EXEEXT)

sample-advanced_model-local: copy-res common
	@echo "Build advanced_model"
	$(CC) $(CFLAGS) advanced_model.c $(LDFLAGS) -o advanced_model$(EXEEXT)

sample-headless-local: copy-res common-headless
	@echo "Build headless"
	$(CC) $(CFLAGS) headless.c $(LDFLAGS_HEADLESS) -o headless$(EXEEXT)

sample-terrain-local: copy-res common
	@echo "Build terrain"
	$(CC) $(CFLAGS) terrain.c $(LDFLAGS) -o terrain$(EXEEXT)

sample-dds-local: copy-res common
	@echo "Build dds"
	$(CC) $(CFLAGS) dds.c $(LDFLAGS) -o dds$(EXEEXT)
