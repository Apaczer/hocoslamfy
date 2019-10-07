TARGET ?= hocoslamfy
VERSION ?= $(shell date +%Y-%m-%d\ %H:%M)
RELEASEDIR = package
ASSETSDIR = assets
OPKG_ASSETSDIR = opkg_assets
LINK = hocoslamfy.lnk
DESTDIR = games
SECTION = games
ALIASES = aliases.txt

ifeq ($(TARGET), hocoslamfy)
  CC        := arm-linux-gcc
  STRIP     := arm-linux-strip
  OBJS       = platform/opendingux.o
  DEFS      := -DNO_SHAKE -DUSE_HOME -DMIYOO
  FLAGS     :=
  DEVICE    := miyoo
else
ifeq ($(TARGET), hocoslamfy-gcw0)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o
  DEFS      := -DOPK -DUSE_HOME -DUSE_16BPP
  FLAGS     := -lshake
  DEVICE    := gcw0
else
ifeq ($(TARGET), hocoslamfy-lepus)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o
  DEFS      := -DOPK -DNO_SHAKE
  FLAGS     := 
  DEVICE    := lepus
else
ifeq ($(TARGET), hocoslamfy-rs90)
  CC        := mipsel-linux-gcc
  STRIP     := mipsel-linux-strip
  OBJS       = platform/opendingux.o
  DEFS      := -DOPK -DSCREEN_WIDTH=240 -DSCREEN_HEIGHT=160 -DSCREEN_BPP=16 -DNO_SHAKE
  FLAGS     := 
  DEVICE    := rs90
else
ifeq ($(TARGET), hocoslamfy_pc)
  CC        := gcc
  STRIP     := strip
  OBJS       = platform/general.o
  DEFS      := -DNO_SHAKE
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

OBJS        += main.o init.o title.o game.o score.o audio.o bg.o text.o unifont.o path.o
              
HEADERS     += main.h init.h platform.h title.h game.h score.h audio.h bg.h text.h unifont.h path.h

INCLUDE     := -I.
DEFS        +=

CFLAGS       = $(SDL_CFLAGS) -Wall -Wno-unused-variable \
               -O2 -fomit-frame-pointer $(DEFS) $(INCLUDE)
LDFLAGS     := $(SDL_LIBS) -lm -lSDL_image -lSDL_mixer $(FLAGS)

ifneq (, $(findstring MINGW, $(shell uname -s)))
	CFLAGS+=-DDONT_USE_PWD
endif

WAVS        := $(wildcard sound/*.wav)
OGGS        := $(WAVS:sound/%.wav=$(ASSETSDIR)/data/%.ogg)

DATA_TO_CLEAN := $(OGGS)

.PHONY: all opk package zip ipk

all: $(TARGET) $(OGGS)

include Makefile.rules

$(OGGS): $(ASSETSDIR)/data/%.ogg: sound/%.wav
	$(SUM) "  OGG     $@"
	$(CMD)oggenc --resample 44100 -q2 $< -o $@

opk: $(TARGET).opk

$(TARGET).opk: $(TARGET) $(OGGS)
	$(SUM) "  OPK     $@"
	$(CMD)rm -rf .opk_data
	$(CMD)mkdir -p .opk_data
	$(CMD)cp $(ASSETSDIR)/data/default.$(DEVICE).desktop .opk_data/
	$(CMD)cp $(ASSETSDIR)/data/*.png .opk_data/
	$(CMD)cp $(OGGS) .opk_data/
	$(CMD)cp $(ASSETSDIR)/data/*.txt .opk_data/
	$(CMD)cp COPYRIGHT .opk_data/COPYRIGHT
	$(CMD)cp $< .opk_data/$(TARGET)
	$(CMD)$(STRIP) .opk_data/$(TARGET)
	$(CMD)mksquashfs .opk_data $@ -all-root -noappend -no-exports -no-xattrs -no-progress >/dev/null

package: all
	@mkdir -p $(RELEASEDIR)
	@cp *$(TARGET) $(RELEASEDIR)/
	@mkdir -p $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)
	@mkdir -p $(RELEASEDIR)/mnt/gmenu2x/sections/$(SECTION)
	@mv $(RELEASEDIR)/*$(TARGET) $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)/
	@cp -r $(ASSETSDIR)/* $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)
	@cp $(OPKG_ASSETSDIR)/$(LINK) $(RELEASEDIR)/mnt/gmenu2x/sections/$(SECTION)
	@cp $(OPKG_ASSETSDIR)/$(ALIASES) $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)

zip: package
	@cd $(RELEASEDIR) && zip -rq $(TARGET)$(VERSION).zip ./* && mv *.zip ..
	@rm -rf $(RELEASEDIR)

ipk: package
	@mkdir -p $(RELEASEDIR)/data
	@mv $(RELEASEDIR)/mnt $(RELEASEDIR)/data/
	@cp -r $(OPKG_ASSETSDIR)/control $(RELEASEDIR)
	@sed "s/^Version:.*/Version: $(VERSION)/" $(OPKG_ASSETSDIR)/control/control > $(RELEASEDIR)/control/control
	@echo 2.0 > $(RELEASEDIR)/debian-binary
	@tar --owner=0 --group=0 -czvf $(RELEASEDIR)/data.tar.gz -C $(RELEASEDIR)/data/ . >/dev/null 2>&1
	@tar --owner=0 --group=0 -czvf $(RELEASEDIR)/control.tar.gz -C $(RELEASEDIR)/control/ . >/dev/null 2>&1
	@ar r $(TARGET).ipk $(RELEASEDIR)/control.tar.gz $(RELEASEDIR)/data.tar.gz $(RELEASEDIR)/debian-binary
	@rm -rf $(RELEASEDIR)

# The two below declarations ensure that editing a .c file recompiles only that
# file, but editing a .h file recompiles everything.
# Courtesy of Maarten ter Huurne.

# Each object file depends on its corresponding source file.
$(C_OBJS): %.o: %.c

# Object files all depend on all the headers.
$(OBJS): $(HEADERS)
