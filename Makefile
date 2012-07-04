CC= gcc
CFLAGS= -Wall -Wextra -O2 -DAPPNAME=\"aib\" -DDEBUG
OUTFILE= aib

OBJS= common.o aib.o

$(OUTFILE): $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^
clean :
	$(RM) $(OUTFILE) $(OBJS) *~ *.o
