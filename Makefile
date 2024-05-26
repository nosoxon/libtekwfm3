SRCS = wfm.c

all:
	gcc -shared -fPIC -O3 -s -o libwfm.so $(SRCS)

clean:
	rm -rf libwfm.so
