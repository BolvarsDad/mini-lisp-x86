CFLAGS=-std=gnu99 -g -Wall -Wextra

.PHONY: clean test

minilisp: minilisp.c

clean:
	rm -f minilisp *~

test: minilisp
	@./test.sh