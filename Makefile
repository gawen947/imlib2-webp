TARGET = webp.so
MAJOR  = 1
MINOR  = 1
PATCH  = 3

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) $(shell pkg-config imlib2 --cflags) -fPIC -Wall
LDFLAGS := $(shell pkg-config imlib2 --libs) $(shell pkg-config imlib2 --libs libwebp) $(shell pkg-config imlib2 --libs libwebpdemux)

SRC = $(wildcard *.c)
OBJ = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP = $(SRC:.c=.d)

LIBDIR    ?= $(shell pkg-config imlib2 --variable=libdir)
LOADERDIR ?= $(LIBDIR)/imlib2/loaders/

version = $(MAJOR).$(MINOR).$(PATCH)
CFLAGS += -DVERSION="\"$(version)\""

commit = $(shell ./hash.sh)
ifneq ($(commit), UNKNOWN)
	CFLAGS += -DCOMMIT="\"$(commit)\""
endif

imlib2_version = $(shell ./imlib2-version.sh)
ifeq ($(imlib2_version), "")
$(error cannot guess imlib2 version)
endif
CFLAGS += $(imlib2_version)


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
	rm -f $(DEP)
	rm -f $(OBJ)
	rm -f $(TARGET)

install:
	install -d $(DESTDIR)$(LOADERDIR)
	install -s -m 444 webp.so $(DESTDIR)$(LOADERDIR)

uninstall:
	rm -f $(LOADERDIR)/$(TARGET)

-include $(DEP)
