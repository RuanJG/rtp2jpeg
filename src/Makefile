TOP_DIR := $(shell pwd)
APP = $(TOP_DIR)/bin/camera_encode

CC = gcc
CFLAGS = -g 
LIBS = -lpthread -ljpeg -lm 
HEADER =
OBJS = video_capture.o sock.o mjpeg_encode.o  engine.o  main.o 

all:  $(OBJS)
	$(CC) -g -o $(APP) $(OBJS) $(LIBS) 

clean:
	rm -f *.o a.out $(APP)/bin/camera_encode core *~ *.jpg
