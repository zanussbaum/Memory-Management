OBJECTS1 = tester1.o frame.o kmalloc.o translation.o ll_double.o
OBJECTS2 = tester2.o frame.o kmalloc.o translation.o ll_double.o
OBJECTS3 = test_translation.o translation.o frame.o 

tester1-clean: clean tester1
tester2-clean: clean tester2
test2-clean: clean test2

tester1: $(OBJECTS1)
	clang -g -std=c11 -o tester1 $(OBJECTS1) -I.

tester2: $(OBJECTS2)
	clang -g -std=c11 -o tester2 $(OBJECTS2) -I.

test2: $(OBJECTS3)
	clang -g -std=c11 -o test2 $(OBJECTS3) -I.


%.o: %.c
	clang -g -std=c11 -c -o $@ -I. $<

clean:
	rm -f *.o tester1 tester2 test2
