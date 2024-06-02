C_OBJS = wfm.o capture.o

CFLAGS = -flto -fPIC -O3

SHARED_OBJ = libtekwfm.so

lib: $(C_OBJS)
	gcc -shared -s $(CFLAGS) -o $(SHARED_OBJ) $^

$(filter %.o,$(C_OBJS)): %.o: %.c
	gcc -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(C_OBJS) $(SHARED_OBJ)
