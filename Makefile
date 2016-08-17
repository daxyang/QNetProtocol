TARGET=libQNetProtocol.so

CC=g++
#CC=arm-none-linux-gnueabi-g++

INCLUDE =-I../QSlidingWindowConsume -I../QSlidingWindow -I../GetDVOStream

.PHONY:clean

libQNetProtocol.so:QNetClient.o QNetServer.o QNetTcpServer.o QAntProtocol.o
	$(CC) -shared -fPIC -o libQNetProtocol.so QNetServer.o QNetClient.o QNetTcpServer.o  QAntProtocol.o -L/usr/local/lib -lQSlidingWindow -lQSlidingWindowConsume -lGetDVOStream


QNetTcpServer.o:QNetTcpServer.cpp QNetTcpServer.h net_protocol.h
	$(CC) $(INCLUDE) -c QNetTcpServer.cpp QNetTcpServer.h net_protocol.h -L/usr/local/lib -lQSlidingWindow -lQSlidingWindowConsume

QNetClient.o:QNetClient.cpp QNetClient.h net_protocol.h
	$(CC) $(INCLUDE)  -c QNetClient.cpp QNetClient.h net_protocol.h

QNetServer.o:QNetServer.cpp QNetServer.h net_protocol.h
	$(CC) $(INCLUDE) -c QNetServer.cpp QNetServer.h net_protocol.h

QAntProtocol.o:QAntProtocol.cpp QAntProtocol.h net_protocol.h
	$(CC) $(INCLUDE) -c QAntProtocol.cpp QAntProtocol.h net_protocol.h

install:
	cp -f lib*.so /usr/local/lib
clean:
	rm -f *.o* *.so*
