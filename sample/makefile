include commands.mk

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) $(shell imlib2-config --cflags) -fPIC -Wall
LDFLAGS := $(shell imlib2-config --libs) -lwebp

SRC = $(wildcard *.c)
OBJ = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP = $(SRC:.c=.d)

PREFIX ?= /usr/local
MIME   ?= /share/mime
THUMB  ?= /share/thumbnailers/
GCONF  ?= /share/gconf
BIN    ?= /bin

ifndef DISABLE_DEBUG
	CFLAGS += -ggdb
endif

.PHONY: all clean

all: imlib2-thumbnailer

imlib2-thumbnailer: imlib2-thumbnailer.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(DEP)
	$(RM) $(OBJ)
	$(RM) imlib2-thumbnailer

install: all
	$(INSTALL_DIR) $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_DIR) $(DESTDIR)/$(PREFIX)/$(THUMB)
	$(INSTALL_DIR) $(DESTDIR)/$(PREFIX)/$(GCONF)/schemas
	$(INSTALL_PROGRAM) imlib2-thumbnailer $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_DATA) webp.xml $(DESTDIR)/$(PREFIX)/$(MIME)/packages
	$(INSTALL_DATA) webp-thumbnailer.desktop $(DESTDIR)/$(PREFIX)/$(THUMB)
	$(INSTALL_DATA) webp-thumbnailer.schemas $(DESTDIR)/usr/$(GCONF)/schemas
	gconf-schemas --register webp-thumbnailer.schemas
	update-mime-database $(DESTDIR)/$(PREFIX)/$(MIME)


uninstall:
	$(RM) $(DESTDIR)/$(PREFIX)/$(BIN)/imlib2-thumbnailer

-include $(DEP)
