project: libraries/basic_client_udp.c src/main.c
	gcc -o project libraries/basic_client_udp.c src/main.c -I.

clean:
	rm project

run:
	./project