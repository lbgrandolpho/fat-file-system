all: main clean

main: main.o fat.o
	gcc $^ -o $@

main.o: main.c
	gcc -c -g $< -o $@

fat.o: fat.c
	gcc -c -g $< -o $@

clean:
	rm *.o