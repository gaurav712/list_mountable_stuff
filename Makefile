CC="gcc"
CFLAGS="-Wall"

all:
	$(CC) -o list_mountable_stuff $(CFLAGS) list_mountable_stuff.c

clean:
	rm -f list_mountable_stuff

install: all
	chmod +s list_mountable_stuff
	cp list_mountable_stuff /usr/local/bin/list_mountable_stuff

uninstall:
	rm -f /usr/local/bin/list_mountable_stuff
