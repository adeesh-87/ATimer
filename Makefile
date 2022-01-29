all:
	gcc src/main.c -o build/sim.out -lpthread -lrt

clean:
	rm -f build/sim.out