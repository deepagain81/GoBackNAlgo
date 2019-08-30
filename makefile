all: client server

client:
	g++ -std=c++11 client.cpp -o client
	
server:
	g++ -std=c++11 server.cpp -o server	
	
clean:
	\rm *.o client server
