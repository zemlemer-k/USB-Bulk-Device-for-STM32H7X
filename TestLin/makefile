#Project name
PROJECT = bulktest

#Compiler
CC=gcc

# Colmpiler flags
CFLAGS := -c
CFLAGS += -Os
CFLAGS += -Wall -pedantic
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wl,--gc-sections
CFLAGS += -fno-strict-aliasing
CFLAGS += -std=c99
CFLAGS += -fmax-errors=5

# include files directories
INCDIRS := /usr/include/libusb-1.0/
CFLAGS +=$(foreach i, $(INCDIRS), -I$(i))

# Linker
#LINK=gcc
LD=gcc

#Linker flags
LDFLAGS := -Wl,--gc-sections
LDFLAGS += -lusb-1.0
LDFLAGS += -lm

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: $(SRC) $(PROJECT)

$(PROJECT): $(OBJ) 
	$(LD) $(LDFLAGS) $(OBJ) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	rm *o $(PROJECT) 
