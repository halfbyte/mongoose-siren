PROG=	siren
CFLAGS=	-W -Wall -pthread -g -lusb -lk8055 -lm

all:
	OS=`uname`; \
	  test "$$OS" = Linux && LIBS="-ldl" ; \
	  $(CC) $(CFLAGS) siren.c mongoose.c  $$LIBS $(ADD) -o $(PROG)
	./$(PROG)
