all:
	gcc -c -Isrc/ src/main.c -o build/main.o
	gcc -c -Isrc/ src/ATimer.c -o build/ATimer.o
	gcc build/main.o build/ATimer.o -o ./test_atimer -lrt -lpthread

clean:
	rm -f build/*.o
	rm -f ./test_atimer