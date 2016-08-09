#include "QNetServer.h"
void reaper(int sig)
{
    int status;
    while(wait3(&status,WNOHANG,(struct rusage*)0)  >=0)
        ;
}

QNetServer::QNetServer()
{

}

void QNetServer::init()
{
  bzero(&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

  server_socket = socket(AF_INET,SOCK_STREAM,0);
  if( server_socket < 0){
      printf("Create Socket Failed!\n");
      exit(1);
  }

  if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))){
      printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT);
      exit(1);
  }

  fprintf(stderr,"Before Main Process listen.\n");
  if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) ){
      printf("Server Listen Failed!");
      exit(1);
  }
  //LOG0_INFO("After Main Process Listen.\n");
  printf("After Main Process Listen.\n");
}
void QNetServer::start()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&server_pthread_id,&attr,run_server,this);
  pthread_attr_destroy (&attr);
}
void *QNetServer::run_server(void *ptr)
{
  QNetServer *pthis = (QNetServer *)ptr;
  (void)signal(SIGCHLD,reaper);
  while(1)
  {
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    usleep(100000);
    //fprintf(stderr,"Before accept. In %d.\n",getpid());
    printf("7777777777777777777\n");

    int new_client_socket = accept(pthis->server_socket,(struct sockaddr*)&client_addr,&length);
    if ( new_client_socket < 0)
    {
      if(errno == EINTR)
      {
        printf("Server Accept Failed!\n");
        usleep(10000);
        continue;
      }
      else
      {
        printf("break\n");
        break;  //注意，一定用break;
      }
    }
    // QNetTcpServer *tcp_server = new QNetTcpServer();
    // if(tcp_server->login(new_client_socket) == 1)
    // {
    // //  close(pthis->server_socket);
    //   tcp_server->server_start(new_client_socket);
    // }
    // else
    // {
    //   free(tcp_server);
    // }
    pthis->start_sub_server(new_client_socket);
    #if defined(Q_OS_WIN32)
      usleep(1000);
    #elif defined(Q_OS_MACX)
      pthread_yield_np();
    #elif defined(Q_OS_UNIX)
      //usleep(5000);
      pthread_yield();
    #endif
  }
}
void QNetServer::start_sub_server(int sk)
{
  sock_id = sk;
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&sub_server_id,&attr,run_sub_server,&sock_id);
  pthread_attr_destroy (&attr);
}
void *QNetServer::run_sub_server(void *ptr)
{
  QNetTcpServer *tcp_server = new QNetTcpServer();
  int *new_client_socket = (int *)ptr;
  if(tcp_server->login(*new_client_socket) == 1)
  {
  //  close(pthis->server_socket);
    tcp_server->server_start(*new_client_socket);
    while (tcp_server->quit == 0) {
      usleep(500000);
    }
    printf("free \n");
    free(tcp_server);
  }
  else
  {
    free(tcp_server);
  }


}
