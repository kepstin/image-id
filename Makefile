CC ?= gcc
RM ?= rm -f
CFLAGS ?= -g -Wall

IMAGE_ID_LIBS:=$(shell pkg-config --libs libdiscid 'libmirage >= 2.0.0')
IMAGE_ID_CFLAGS:=-std=c99 $(shell pkg-config --cflags libdiscid 'libmirage >= 2.0.0')

IMAGE_ID_SOURCES:=image_id.c
IMAGE_ID_OBJECTS:=$(addsuffix .o,$(basename $(filter %.c,$(IMAGE_ID_SOURCES))))

all: image_id

clean:
	$(RM) image_id
	$(RM) $(IMAGE_ID_OBJECTS)

.PHONY: all clean

image_id: $(IMAGE_ID_OBJECTS)
	$(CC) $(IMAGE_ID_LIBS) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) -c $(IMAGE_ID_CFLAGS) $(CFLAGS) $(CPPFLAGS) $< -o $@
