bridge: bridge_epoll.c printheader.c rawsocketutil.c
	gcc -Wall -O2 -I./ bridge_epoll.c -o ./bin/bridge printheader.c rawsocketutil.c

echo_server_prefork: echo_server_prefork.c
	gcc -Wall -O2 -I./ echo_server_prefork.c -o ./bin/echo_server_prefork

sniffer: main.c printheader.c rawsocketutil.c
	gcc -Wall -O2 -I./ main.c -o ./bin/sniffer printheader.c rawsocketutil.c

echo_server_prefork_epoll: echo_server_prefork_epoll.c
	gcc -Wall -O2 -I./ echo_server_prefork_epoll.c -o ../bin/echo_server_prefork_epoll

reuseport_echo_server_prefork_epoll: reuseport_echo_server_prefork_epoll.c
	gcc -Wall -O2 -I./ reuseport_echo_server_prefork_epoll.c -o ../bin/reuseport_echo_server_prefork_epoll
