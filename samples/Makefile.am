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
								 terrain.c

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

samples-local: copy-res common \
							 sample-doublemulticolorshader-local \
							 sample-rotatingcube-local \
							 sample-rotatingplane-local \
							 sample-readobjfile_manual-local \
							 sample-readobjfile-local \
							 sample-advanced_model-local \
							 sample-headless-local \
							 sample-terrain-local
	@echo "All samples are built"

copy-res:
	cp -R ../res res

common: main.c functs.c functs.h usercode.h
	@echo "Build common"
	$(CC) $(CFLAGS) -c main.c
	$(CC) $(CFLAGS) -c functs.c

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

sample-headless-local: copy-res common
	@echo "Build headless"
	$(CC) $(CFLAGS) headless.c $(LDFLAGS) -o headless$(EXEEXT)

sample-terrain-local: copy-res common
	@echo "Build terrain"
	$(CC) $(CFLAGS) terrain.c $(LDFLAGS) -o terrain$(EXEEXT)
