.DEFAULT_GOAL := all

CC = gcc
RM = rm -rf
MKDIR = mkdir -p

CFLAGS = -I/usr/include/libusb-1.0 -Wall -g
LFLAGS =

LIBS = -lusb-1.0 -lpng

EXE = hantek-cli
OUT = build

DEPS = message.h screenshot.h
_OBJ = main.o message.o screenshot.o
OBJ  = $(addprefix $(OUT)/,$(_OBJ))

$(OUT):
	$(MKDIR) $(OUT)

$(OUT)/%.o: %.c $(DEPS) $(OUT)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

compile_commands.json:
	bear -- make all

.PHONY: all clean

all: $(EXE)

clean:
	$(RM) $(OUT)
	$(RM) $(EXE)
	$(RM) compile_commands.json
