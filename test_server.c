#include "QNetTcpServer.h"

int main()
{
  QNetServer *tcp_server = new QNetServer();
  tcp_server->init();
  tcp_server->start();
  while(1)
    sleep(10);

}
