TOP_DIR := $(shell pwd)
APP = camera_encode

CC = gcc
CFLAGS = -g 
LIBS = -lpthread -ljpeg -lm 
HEADER =$(wildcard *.h)
SRCFILE = $(wildcard *.c)
OBJS = video_capture.o sock.o mjpeg_encode.o  engine.o  main.o 

all:  $(OBJS) $(HEADER) $(SRCFILE)
	$(CC) -g -o $(APP) $(OBJS) $(LIBS) 

clean:
	rm -f *.o a.out camera_encode core *~ *.jpg
