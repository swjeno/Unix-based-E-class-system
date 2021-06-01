CC = gcc
CFLAGS = -D_REENTRANT
CLIBS = -pthread
CMDS  = eserver eclient

all : $(CMDS)

eserver : eserver.c
        $(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

eclient : eclient.c
        $(CC) $(CFLAGS) $^ -o $@ $(CLIBS)

clean :
        rm eserver
        rm eclient
