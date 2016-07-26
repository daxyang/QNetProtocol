TARGET=libQNetProtocol.so

CC=g++

INCLUDE =-I../QSlidingWindowConsume -I../QSlidingWindow

.PHONY:clean

libQNetProtocol.so:QNetClient.o QNetServer.o QNetTcpServer.o QAntProtocol.o
	$(CC) -shared -fPIC -o libQNetProtocol.so QNetTcpServer.o QNetClient.o QNetServer.o  QAntProtocol.o -L/usr/local/lib -lqslidingwindow -lQSlidingWindowConsume


QNetTcpServer.o:QNetTcpServer.cpp QNetTcpServer.h net_protocol.h
	$(CC) $(INCLUDE) -c QNetTcpServer.cpp QNetTcpServer.h net_protocol.h -L/usr/local/lib -lqslidingwindow -lQSlidingWindowConsume

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
