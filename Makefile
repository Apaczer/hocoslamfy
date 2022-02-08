TARGET      ?= hocoslamfy-gcw0

ifeq ($(TARGET), hocoslamfy-gcw0)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o text/text_ttf.o repository/sqlite.o score/score_extended.o
  HEADERS    = repository.h
  DEFS      := -DOPK -DUSE_HOME -DSCREEN_BPP=16 -DLOGGING -DUSE_DATABASE
  FLAGS     := -lshake -lSDL_ttf
  DEVICE    := gcw0
else
ifeq ($(TARGET), hocoslamfy-gcw0-v1)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o text/text.o score/score.o
  DEFS      := -DOPK -DUSE_HOME -DSCREEN_BPP=16
  FLAGS     := -lshake
  DEVICE    := gcw0
else
ifeq ($(TARGET), hocoslamfy-lepus)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o text/text.o score/score.o
  DEFS      := -DOPK -DNO_SHAKE
  FLAGS     := 
  DEVICE    := lepus
else
ifeq ($(TARGET), hocoslamfy-rs90)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o text/text.o score/score.o
  DEFS      := -DOPK -DSCREEN_WIDTH=240 -DSCREEN_HEIGHT=160 -DSCREEN_BPP=16 -DNO_SHAKE
  FLAGS     := 
  DEVICE    := rs90
else
ifeq ($(TARGET), hocoslamfy)
  CC        := gcc
  STRIP     := strip
  OBJS       = platform/general.o text.o
  DEFS      := 
  FLAGS     := 
else
  $(error Unknown target: $(TARGET))
endif
endif
endif
endif
endif

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CONFIG  ?= $(SYSROOT)/usr/bin/sdl-config
SDL_CFLAGS  := $(shell $(SDL_CONFIG) --cflags)
SDL_LIBS    := $(shell $(SDL_CONFIG) --libs)

OBJS        += main.o init.o title.o game.o audio.o bg.o unifont.o path.o log.c/src/log.o

HEADERS     += main.h init.h platform.h title.h game.h score.h audio.h bg.h text.h unifont.h path.h log.c/src/log.h

INCLUDE     := -I.
DEFS        +=

CFLAGS       = $(SDL_CFLAGS) -Wall -Wno-unused-variable \
               -O2 -fomit-frame-pointer $(DEFS) $(INCLUDE)
LDFLAGS     := $(SDL_LIBS) -lm -lSDL_image -lSDL_mixer -lsqlite3 $(FLAGS)

ifneq (, $(findstring MINGW, $(shell uname -s)))
	CFLAGS+=-DDONT_USE_PWD
endif

WAVS        := $(wildcard sound/*.wav)
OGGS        := $(WAVS:sound/%.wav=data/%.ogg)

DATA_TO_CLEAN := $(OGGS)

.PHONY: all opk

all: $(TARGET) $(OGGS)

include Makefile.rules

$(OGGS): data/%.ogg: sound/%.wav
	$(SUM) "  OGG     $@"
	$(CMD)oggenc --resample 44100 -q2 $< -o $@

opk: $(TARGET).opk

$(TARGET).opk: $(TARGET) $(OGGS)
	$(SUM) "  OPK     $@"
	$(CMD)rm -rf .opk_data
	$(CMD)mkdir -p .opk_data
	$(CMD)cp data/default.$(DEVICE).desktop .opk_data/
	$(CMD)cp data/*.png .opk_data/
	$(CMD)cp data/*.ttf .opk_data/
	$(CMD)cp $(OGGS) .opk_data/
	$(CMD)cp data/*.txt .opk_data/
	$(CMD)cp COPYRIGHT .opk_data/COPYRIGHT
	$(CMD)cp $< .opk_data/$(TARGET)
	$(CMD)$(STRIP) .opk_data/$(TARGET)
	$(CMD)mksquashfs .opk_data $@ -all-root -noappend -no-exports -no-xattrs -no-progress >/dev/null

# The two below declarations ensure that editing a .c file recompiles only that
# file, but editing a .h file recompiles everything.
# Courtesy of Maarten ter Huurne.

# Each object file depends on its corresponding source file.
$(C_OBJS): %.o: %.c

# Object files all depend on all the headers.
$(OBJS): $(HEADERS)
