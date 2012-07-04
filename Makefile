CC= gcc
CFLAGS= -Wall -Wextra -O2 -DAPPNAME=\"aib\"
OUTFILE= aib

OBJS= common.o aib.o

$(OUTFILE): $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^
clean :
	$(RM) $(OUTFILE) $(OBJS) *~ *.o
