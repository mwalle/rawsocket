CFLAGS := -Wall -Werror
PYTHON_CFLAGS := `python2.7-config --cflags`
PYTHON_LIBS := `python2.7-config --libs`

all: rawsocket.so rawsocket_helper

rawsocket.so: rawsocket.c
	$(CC) -shared -fPIC $(CFLAGS) $(PYTHON_CFLAGS) $(PYTHON_LIBS) -o $@ $<

rawsocket_helper: rawsocket_helper.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f rawsocket.so rawsocket_helper

.PHONY: all clean
