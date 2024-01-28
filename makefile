CC = cc
CFLAGS = -std=c11 -Wall -pedantic
CPPFLAGS = -I$(INCDIR) -D_XOPEN_SOURCE=700
LDLIBS = -lm
BISON = bison
FLEX = flex
BISONFLAGS = --defines=$(INCDIR)/pml2pg.tab.h -Wall

INCDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(BINDIR))

_OBJ = vector.o list.o pgraph.o cube.o io.o hda.o pml2pg.tab.o lex.yy.o main.o 
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(BINDIR)/pg2hda: $(OBJ) 
	$(CC) -o $@ $^ $(LDLIBS) && rm -f $(SRCDIR)/lex.yy.c $(SRCDIR)/pml2pg.tab.c $(INCDIR)/pml2pg.tab.h 

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(SRCDIR)/lex.yy.c: $(SRCDIR)/pml2pg.l
	$(FLEX) -o $@ $<

$(SRCDIR)/%.tab.c: $(SRCDIR)/%.y
	$(BISON) $(BISONFLAGS) -o $@ $<


.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~ $(BINDIR)/* 

