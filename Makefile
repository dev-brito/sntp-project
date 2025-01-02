project: libraries/basic_client_udp.c src/main.c
	gcc -o project libraries/basic_client_udp.c src/main.c -I. -Wall -Wextra -g3

clean:
	rm -f project

run: project
	./project 129.6.15.28

