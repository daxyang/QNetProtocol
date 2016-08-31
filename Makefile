os=macx
TARGET=libQNetProtocol.so
ifeq ($(os),arm)
CC=arm-none-linux-gnueabi-g++
else
CC=g++
endif

INCLUDE =-I../QSlidingWindowConsume -I../QSlidingWindow -I../QGetDVOStream #-I../GetDVOStream

.PHONY:clean

libQNetProtocol.so:QNetClient.o QNetServer.o QNetTcpServer.o QAntProtocol.o
	$(CC) -shared -fPIC -o libQNetProtocol.so QNetServer.o QNetClient.o QNetTcpServer.o  QAntProtocol.o -L/usr/local/lib -lQSlidingWindow -lQSlidingWindowConsume -lQGetDVOStream #-lGetDVOStream


QNetTcpServer.o:QNetTcpServer.cpp QNetTcpServer.h
	$(CC) $(INCLUDE) -c QNetTcpServer.cpp QNetTcpServer.h  -L/usr/local/lib -lQSlidingWindow -lQSlidingWindowConsume

QNetClient.o:QNetClient.cpp QNetClient.h net_protocol.h
	$(CC) $(INCLUDE)  -c QNetClient.cpp QNetClient.h net_protocol.h

QNetServer.o:QNetServer.cpp QNetServer.h
	$(CC) $(INCLUDE) -c QNetServer.cpp QNetServer.h

QAntProtocol.o:QAntProtocol.cpp QAntProtocol.h net_protocol.h
	$(CC) $(INCLUDE) -c QAntProtocol.cpp QAntProtocol.h net_protocol.h

install:
	cp -f lib*.so /usr/local/lib
clean:
	rm -f *.o* *.so*
