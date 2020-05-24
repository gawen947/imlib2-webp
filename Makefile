TARGET = webp.so
MAJOR  = 1
MINOR  = 1
PATCH  = 0

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) $(shell imlib2-config --cflags) -fPIC -Wall
LDFLAGS := $(shell imlib2-config --libs) $(shell pkg-config --libs libwebp) $(shell pkg-config --libs libwebpdemux)

SRC = $(wildcard *.c)
OBJ = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP = $(SRC:.c=.d)

LIBDIR    ?= $(shell pkg-config --variable=libdir imlib2)
LOADERDIR ?= $(LIBDIR)/imlib2/loaders/

version = $(MAJOR).$(MINOR).$(PATCH)
CFLAGS += -DVERSION="\"$(version)\""

commit = $(shell ./hash.sh)
ifneq ($(commit), UNKNOWN)
	CFLAGS += -DCOMMIT="\"$(commit)\""
endif

ifndef DISABLE_DEBUG
	CFLAGS += -ggdb
endif

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $<

clean:
	rm $(DEP)
	rm $(OBJ)
	rm $(TARGET)

install:
	install -d $(DESTDIR)$(LOADERDIR)
	install -s -m 444 webp.so $(DESTDIR)$(LOADERDIR)

uninstall:
	rm $(LOADERDIR)/$(TARGET)

-include $(DEP)
