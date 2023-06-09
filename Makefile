CFLAGS+=-Wall -Wextra
CFLAGS+=`pkg-config --cflags gtk+-2.0`
LIBS+=`pkg-config --libs gtk+-2.0`
PROG=editor

help:
	@echo -e "Available commands:"
	@echo -e "\tall - compile the program"
	@echo -e "\tclean - clean up"
	@echo -e "\thelp - print this help message"

all:
	$(CC) $(CFLAGS) -o $(PROG) ./*.c $(LIBS)

clean:
	rm -f $(PROG)
