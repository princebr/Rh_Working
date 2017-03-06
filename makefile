mainmake: main.c functions.c

	gcc -o run main.c functions.c -l bcm2835 -I.
