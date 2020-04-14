all:
	g++ ping.cpp ping-helper.cpp -o ping

clean:
	rm *.o ping
