APP = ldpc-ogl

CC ?= g++
COPTS ?= -O0 -g -Wall

LIBS = -L/usr/X11R6/lib -lGL -lGLU -lglut -lXi -lXmu -lXext -lX11 -lm

all : $(APP)

clean : 
	rm -f $(APP)

run : $(APP)
	./$(APP)

$(APP) : $(APP).cpp
	$(CC) -o $@ $(COPTS) $< $(LIBS)
