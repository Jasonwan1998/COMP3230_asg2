all: psort

psort: psort.c
	gcc $^ -o $@ -Wall -pthread

clean:
	rm -f psort