#ifndef QNETSERVER_H
#define QNETSERVER_H
#include "net_protocol.h"
#include "pthread.h"
#include "QNetTcpServer.h"
class QNetServer
{
public:
  QNetServer();
  void init();
  void start();
protected:
private:
  struct sockaddr_in server_addr;
  int server_socket ;
  struct cmd_link_t *head_cmd_link;//主节点的头
  pthread_t server_pthread_id;


private:
  static void *run_server(void *ptr);


};

#endif
