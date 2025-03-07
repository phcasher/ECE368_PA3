WARNING = -Wall -Wshadow --pedantic
ERROR = -Wvla -Werror
GCC = gcc -O3 -std=c99 -g $(WARNING) $(ERROR)
VAL = valgrind --tool=memcheck --log-file=memcheck.txt --leak-check=full --verbose

OBJS = $(SRCS:%.c=%.o)

%.o: %.c
	$(GCC) -c $< -o $@

test1:
	./pa3 3.po out_file1 out_file2 out_file3 out_file4

test2:
	./pa3 8.po out_file1 out_file2 out_file3 out_file4

test3:
	./pa3 100.po out_file1 out_file2 out_file3 out_file4

test4:
	./pa3 500.po out_file1 out_file2 out_file3 out_file4

test5:
	./pa3 1K.po out_file1 out_file2 out_file3 out_file4

clean:
	rm -f pa3 out_file1 out_file2 out_file3 out_file4 *.o output?