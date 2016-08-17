#include "QNetServer.h"
#include "sys/syscall.h"
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
  //run_server(this);
}
void *QNetServer::run_server(void *ptr)
{
  QNetServer *pthis = (QNetServer *)ptr;
  (void)signal(SIGCHLD,reaper);
  printf("\033[7;5;32m***********************************\033[0m\n");
  printf("\033[7;5;32m****AntNetProtocol server start****\033[0m\n");
  printf("\033[7;5;32m***********************************\033[0m\n");
  while(1)
  {
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    usleep(100000);

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
    printf("\033[31m***********************************\033[0m\n");
    printf("\033[31m**** New User connect to server****\033[0m\n");
    printf("\033[31m***********************************\033[0m\n");

    pthis->start_sub_server(new_client_socket);

    // #if defined(Q_OS_WIN32)
    //   usleep(1000);
    // #elif defined(Q_OS_MACX)
    //   pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //   //usleep(5000);
    //   pthread_yield();
    // #endif
  }
}
void QNetServer::start_sub_server(int sk)
{
  sock_id = sk;
  pthread_t sub_server_id;
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&sub_server_id,&attr,/*(void *(*)(void *))*/run_sub_server,&sock_id);
  pthread_attr_destroy (&attr);
}
void *QNetServer::run_sub_server(void *ptr)
{
  QNetTcpServer *tcp_server = new QNetTcpServer();
  int new_client_socket = *(int *)ptr;
  if(tcp_server->login(new_client_socket) == 1)
  {
    printf("\033[31m**** Login OK sock:%d ****\033[0m\n",new_client_socket);
    tcp_server->server_start(new_client_socket);
    while (tcp_server->quit == 0) {
      usleep(500000);
    }
    close(new_client_socket);
    printf("free sock:%d\n",new_client_socket);
    delete tcp_server;
  }
  else
  {
    close(new_client_socket);
    delete tcp_server;
  }
}
