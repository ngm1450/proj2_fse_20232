CC = gcc
LDFLAGS = -lwiringPi -lpthread
BLDDIR = .
INCDIR = $(BLDDIR)/inc
SRCDIR = $(BLDDIR)/src
OBJDIR = $(BLDDIR)/obj
CFLAGS = -c -w -Wall -I$(INCDIR)
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
EXE = bin/bin

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJDIR)/*.o -o $@ -lwiringPi -lpthread

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

run: all
	./$(EXE)

clean:
	-rm -f $(OBJDIR)/*.o $(EXE)
