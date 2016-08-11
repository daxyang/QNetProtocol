#include "QNetClient.h"

QNetClient::QNetClient()
{
  frame = new _frame_info_t;
  frame->frame_type = 20;

  //初始化主命令cmdtype的链表头
  head_cmd_node = new cmd_link_t;
  head_cmd_node->no = 0;
  head_cmd_node->cmd_type = 0;
  head_cmd_node->subcmd_head = NULL;
  head_cmd_node->next = NULL;

  //添加主命令到链表中
  append_cmd_link_node(NET_TCP_TYPE_CTRL);
  append_cmd_link_node(NET_TCP_TYPE_FILE);
  append_cmd_link_node(NET_TCP_TYPE_VID);
  append_cmd_link_node(NET_TCP_TYPE_FILE);
  /*
   * 新增通讯协议主命令  添加二
   append_cmd_link_node(NET_TCP_TYPE_XXX);
   */

  //发送缓冲区初始化
  send_sliding = new QSlidingWindow();
  send_buffer = (char *)malloc(sizeof(char) * PROTOCOL_BUFFER_LEN);
  send_sliding->sliding_init(PROTOCOL_BUFFER_LEN,send_buffer);
  send_sliding->consume_linklist_append(SEND_USER);
  //发送缓冲区消费者--取协议进行网络发送
  send_consume = send_sliding->consume_linklist_getConsume(SEND_USER);
  //接受缓冲区初始化
  recv_sliding = new QSlidingWindow();
  recv_buffer = (char *)malloc(sizeof(char) * PROTOCOL_BUFFER_LEN);
  recv_sliding->sliding_init(PROTOCOL_BUFFER_LEN,recv_buffer);
  recv_sliding->consume_linklist_append(RECV_USER);
  //接受缓冲区消费者--将从网络接受到的协议进行解析
  recv_consume = recv_sliding->consume_linklist_getConsume(RECV_USER);

  //将回调函数初始化为空 添加三
  set_protocol_ack_callback(NET_TCP_TYPE_CTRL,NET_CTRL_HEART,NULL);
  set_protocol_ack_callback(NET_TCP_TYPE_CTRL,NET_CTRL_LOGIN,NULL);
  set_protocol_ack_callback(NET_TCP_TYPE_CTRL,NET_CTRL_LOGOUT,NULL);
  set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_START,NULL);
  set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_SEND,NULL);
  set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_PATH,NULL);
  set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_LIST,NULL);

}
QNetClient::~QNetClient()
{
  free(recv_buffer);
  free(send_buffer);
}
int QNetClient::connect_server(char *ip,int port)
{
  struct sockaddr_in serv_addr;
  fd_set fdr, fdw;
  struct timeval timeout;
  int flags,res;
  unsigned long ul;
  printf("connect to server.\n\r");

#ifdef __WIN32
  WORD sockVersion = MAKEWORD(2,2);
  WSADATA data;

  if(WSAStartup(sockVersion, &data) != 0)
  {
      printf("ESAStart error\n");
      return -1;
  }
  if(LOBYTE(data.wVersion)!=2 || HIBYTE(data.wVersion)!=2)
  {
      WSACleanup();
      printf("Invalid WinSock version!\n");
      return -1;
  }
  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(client_socket == INVALID_SOCKET)
  {
      printf("socket create error %ld\n",GetLastError());
      printf("create socket failed!\n");
      return -1;
  }
#else
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
      printf("create socket failed!\n");
      return -1;
  }
#endif
#ifdef _WIN32
  ul = 0;

  if((flags = ioctlsocket(client_socket, FIONBIO, &ul)) < 0) {

#else
  ul = 1;
  if((flags = ioctl(client_socket,FIONBIO,&ul)) < 0) {
#endif
      perror("Netwrok test...\n");

      close(client_socket);
      return -1;
  }

  memset(&serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
#ifdef _WIN32
  serv_addr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
  if (inet_pton(AF_INET,ip,&serv_addr.sin_addr) <= 0)
  {
      printf("ip error!\n");
      close(client_socket);
      return -1;
  }
#endif

  if(::connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
      if(errno != EINPROGRESS)
      { // EINPROGRESS

          printf("Network test1...\n");
          close(client_socket);
          return -1;
      }
  }
  else {
      printf("Connected1!\n");
  }
  FD_ZERO(&fdr);
  FD_ZERO(&fdw);
  FD_SET(client_socket, &fdr);
  FD_SET(client_socket, &fdw);

  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;


  res = select(client_socket + 1, &fdr, &fdw, NULL, &timeout);
  if(res < 0) {
      perror("Network test...\n");
      close(client_socket);
      return -1;
  }
  if(res == 0) {
      printf("Connect server timeout\n");
      close(client_socket);
      return -1;
  }
  if(res == 1) {
      if(FD_ISSET(client_socket, &fdw))
      {
          printf("Connected2!\n");
      }

  }

  /* Not necessary */
  if(res == 2) {
      printf("Connect server timeout\n");
      close(client_socket);
      return -1;
  }
  ul = 0;
#ifdef _WIN32
  ioctlsocket(client_socket,FIONBIO,&ul);
#else
  ioctl(client_socket, FIONBIO, &ul); //重新将socket设置成阻塞模式
#endif

return client_socket;
}
/********************************************************
 * 链表操作
 ********************************************************
 */
//添加主命令节点
void QNetClient::append_cmd_link_node(u16 cmd_type)
{
  struct sub_cmd_link_t *head_subcmd = new sub_cmd_link_t;
  head_subcmd->no = 0;
  head_subcmd->sub_cmd_type = 0;
  head_subcmd->callback = NULL;
  head_subcmd->next = NULL;


  struct cmd_link_t *new_cmd_link = new cmd_link_t;
  new_cmd_link->no = cmd_type;
  new_cmd_link->cmd_type = cmd_type;
  new_cmd_link->subcmd_head = head_subcmd;
  new_cmd_link->next = NULL;

  struct cmd_link_t *p1;
  if(head_cmd_node->next == NULL)
  {
      head_cmd_node->next = new_cmd_link;
      return;
  }
  p1 = head_cmd_node->next;
  while((p1->next != NULL) && (p1->cmd_type != cmd_type))
    p1 = p1->next;
  if(p1->cmd_type == cmd_type)
  {
    return;
  }
  else if(p1->next == NULL)
  {
    p1->next = new_cmd_link;
  }

  return;
}
//添加子节点
void QNetClient::append_subcmd_link_node(u16 cmd_type,u32 sub_cmd_type,void (*function)(char *data,u32 len))
{
  struct sub_cmd_link_t *head_subcmd = search_subcmd_head(cmd_type);
  struct sub_cmd_link_t *p1;
  struct sub_cmd_link_t *new_subcmd_node = new sub_cmd_link_t;
  new_subcmd_node->no = sub_cmd_type;
  new_subcmd_node->sub_cmd_type = sub_cmd_type;
  new_subcmd_node->callback = function;
  new_subcmd_node->next = NULL;

  if(head_subcmd->next == NULL)
  {
      head_subcmd->next = new_subcmd_node;
      return;
  }
  p1 = head_subcmd->next;
  while((p1->next != NULL) && (p1->sub_cmd_type != sub_cmd_type))
      p1 = p1->next;
  if(p1->next == NULL)
  {
      if(p1->sub_cmd_type == sub_cmd_type)
         p1->callback = function;
      else
          p1->next = new_subcmd_node;
  }
  else if(p1->sub_cmd_type == sub_cmd_type)
  {
      p1->callback = function;
      return;
  }
  else
  {
    return;
  }
}
/*
 * 根据主命令和子命令查找子节点
 */
struct sub_cmd_link_t *QNetClient::search_subcmd_node(u16 cmd_type,u32 sub_cmd_type)
{
  struct sub_cmd_link_t *head_subcmd = search_subcmd_head(cmd_type);
  struct sub_cmd_link_t *p1;
  if(head_subcmd->next == NULL)
    return NULL;
  p1 = head_subcmd->next;
  while((p1->next != NULL) && (p1->sub_cmd_type != sub_cmd_type))
    p1 = p1->next;

  if(p1->next == NULL)
  {
    if(p1->sub_cmd_type == sub_cmd_type)
      return p1;
    else
      return NULL;
  }
  else if(p1->sub_cmd_type == sub_cmd_type)
  {
    return p1;
  }
}
/*
 * 根据主命令查找子节点的头
 */
struct sub_cmd_link_t *QNetClient::search_subcmd_head(u16 cmd_type)
{
  struct cmd_link_t *p1;
  if(head_cmd_node->next == NULL)
      return NULL;
  p1 = head_cmd_node->next;
  while((p1->next != NULL) && (p1->cmd_type != cmd_type))
    p1 = p1->next;

  if(p1->next == NULL)
  {
    if(p1->cmd_type == cmd_type)
        return p1->subcmd_head;
    else
        return NULL;
  }
  else if(p1->cmd_type == cmd_type)
    return p1->subcmd_head;
  else
    return NULL;
}
/*
 * 协议处理部份--生成命令包，添加到发送缓冲区中进行发送 添加四
 */
//主命令处理
void QNetClient::main_cmd_process(u16 cmd_type,u32 sub_cmd_type,char *data,u32 len)
{
  switch(cmd_type)
  {
    case NET_TCP_TYPE_CTRL:
      do_ctrl_protocol_process(sub_cmd_type,data,len);
    break;
    case NET_TCP_TYPE_FILE:
      do_file_protocol_process(sub_cmd_type,data,len);
    break;
    case NET_TCP_TYPE_AID:
      do_audio_protocol_process(sub_cmd_type,data,len);
    break;
    case NET_TCP_TYPE_VID:
      do_video_protocol_process(sub_cmd_type,data,len);
    break;
    /*
     *添加新的协议: NET_TCP_TYPE_xxx
     case NET_TCP_TYPE_XXX:
       do_xxx_protocol_process(sub_cmd_type,data,len);
     break;
     */
    default:
    break;
  }
}
void QNetClient::do_ctrl_protocol_process(u32 sub_cmd_type,char *data,u32 len)
{
  u32 pkg_len = NET_HEAD_SIZE + len;
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  if(len > 0)
    memcpy((buffer+NET_HEAD_SIZE),data,len);

  switch(sub_cmd_type)
  {
    case NET_CTRL_LOGIN:
        HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_LOGIN,0,pkg_len);
    break;
    case NET_CTRL_LOGOUT:
        HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_LOGOUT,0,pkg_len);
    break;
    case NET_CTRL_HEART:
        HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_HEART,0,pkg_len);
    break;
    /*
     * 添加新的协议:
     case NET_CTRL_XXX:
        HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_XXX,0,pkg_len);
     break;
     */
    default:
    break;
  }
  send_sliding->write_data_to_buffer(pkg_len,buffer,frame);
  free(buffer);
}
void QNetClient::do_file_protocol_process(u32 sub_cmd_type,char *data,u32 len)
{
  u32 pkg_len = NET_HEAD_SIZE + len;
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  if(len > 0)
    memcpy(buffer+NET_HEAD_SIZE,data,len);
  switch (sub_cmd_type) {
    case NET_FILE_START:
      HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_START,0,pkg_len);
    break;
    case NET_FILE_LIST:
      HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
    break;
    case NET_FILE_PATH:
      HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_PATH,0,pkg_len);
    break;
    case NET_FILE_SEND:
      HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_SEND,0,pkg_len);
    break;
  }
  send_sliding->write_data_to_buffer(pkg_len,buffer,frame);
  free(buffer);

}
void QNetClient::do_audio_protocol_process(u32 sub_cmd_type,char *data,u32 len)
{

}
void QNetClient::do_video_protocol_process(u32 sub_cmd_type,char *data,u32 len)
{

}
/*  添加五
void QNetClient::do_xxx_protocol_process(u32 sub_cmd_type,char *data,u32 len)
{

}
 */
/*
 * 从网络接受到的协议进行相应的处理
 */
//设定接受到的协议对应的处理函数
void QNetClient::set_protocol_ack_callback(u16 cmd_type,u32 sub_cmd_type,void (*function)(char *data,u32 len))
{
  append_subcmd_link_node(cmd_type,sub_cmd_type,function);
}

/*
 *  线程函数
 */
//发送协议线程处理函数
void *QNetClient::run_send_pthread(void *ptr)
{
  QNetClient *pthis = (QNetClient *)ptr;
  pthis->send_consume->read_init();
  char *buffer = (char *)malloc(sizeof(char) * PROTOCOL_BUFFER_LEN);
  int time_cnt = 0;
  int err;
  while(pthis->quit == 0)
  {
    int len = pthis->send_consume->read_data_to_buffer(buffer,0);  //使用非阻塞模式读数据
    if(len > 0)
    {
      pthis->WRITE(pthis->client_socket,buffer,len);
      time_cnt = 0;
    }
    else  /*add by antony 2016-7-15 send heart package*/
    {
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 1000; //100mS
      do{
        //  pthread_yield_np();
          err = select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno == EINTR);

      time_cnt++;
      if(time_cnt >= 30 * 90)
      {
        time_cnt = 0;
        time_t timep;
        time(&timep);
        struct tm *ptime;
        ptime = localtime(&timep);

        u32 len = sizeof(app_net_ctrl_heart);
        char *buffer = (char *)malloc(sizeof(char) * len);
        app_net_ctrl_heart *heart = (app_net_ctrl_heart *)buffer;
        heart->yy = htons(1900+ptime->tm_year);
        heart->MM = ptime->tm_mon;
        heart->dd = ptime->tm_mday;
        heart->hh = ptime->tm_hour;
        heart->mm = ptime->tm_min;
        heart->ss = ptime->tm_sec;
        pthis->main_cmd_process(NET_TCP_TYPE_CTRL,NET_CTRL_HEART,buffer,len);
          //pthis->itimer_cnt = 0;
        printf("heart:%d:%d:%d\n",heart->hh,heart->mm,heart->ss);
    }

  }
  // #if defined(Q_OS_WIN32)
  //         usleep(1000);
  // #elif defined(Q_OS_MACX)
  //         pthread_yield_np();
  // #elif defined(Q_OS_UNIX)
  //       //  usleep(5000);
  //         pthread_yield();
  // #endif
  }
  free(buffer);
}
//发送协议线程建立函数
void QNetClient::send_pthread_start()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&send_pthread_id,&attr,run_send_pthread,this);
  pthread_attr_destroy (&attr);
}
//接受网络命令线程
void *QNetClient::run_recv_pthread(void *ptr)
{
  QNetClient *pthis = (QNetClient *)ptr;
  while(pthis->quit == 0)
  {
    char *head_buffer = (char *)malloc(sizeof(char) * NET_HEAD_SIZE);
    app_net_head_pkg_t *head = (app_net_head_pkg_t *)head_buffer;
    pthis->READ(pthis->client_socket,head_buffer,NET_HEAD_SIZE);
    u32 len = ntohl(head->Length);
    char *buffer = (char *)malloc(sizeof(char) * len);
    pthis->READ(pthis->client_socket,(buffer+NET_HEAD_SIZE),len - NET_HEAD_SIZE);
    memcpy(buffer,head_buffer,NET_HEAD_SIZE);
    pthis->recv_sliding->write_data_to_buffer(len,buffer,pthis->frame);
    // #if defined(Q_OS_WIN32)
    //           usleep(1000);
    // #elif defined(Q_OS_MACX)
    //           pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //         //  usleep(5000);
    //           pthread_yield();
    // #endif
    free(head_buffer);
    free(buffer);
  }
}
//接受网线命令线程建立函数
void QNetClient::recv_pthread_start()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&recv_pthread_id,&attr,run_recv_pthread,this);
  pthread_attr_destroy (&attr);
}
//接受到网络命令后，进行命令处理的线程
void *QNetClient::run_treasmit_pthread(void *ptr)
{
  QNetClient *pthis = (QNetClient *)ptr;
  pthis->recv_consume->read_init();
  char *buffer = (char *)malloc(sizeof(char) * PROTOCOL_BUFFER_LEN);
  while(pthis->quit == 0)
  {
    int len = pthis->recv_consume->read_data_to_buffer(buffer);
    if(len > 0)
    {
      app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
      u16 cmd_type = ntohs(head->CmdType);
      u32 sub_cmd_type = ntohl(head->CmdSubType);
      u32 len = ntohl(head->Length);

      struct sub_cmd_link_t *subcmd_node = pthis->search_subcmd_node(cmd_type,sub_cmd_type);
      if(subcmd_node->callback != NULL)
          subcmd_node->callback(buffer+NET_HEAD_SIZE,len - NET_HEAD_SIZE);
    }
    // #if defined(Q_OS_WIN32)
    //           usleep(1000);
    // #elif defined(Q_OS_MACX)
    //           pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //         //  usleep(5000);
    //           pthread_yield();
    // #endif
  }
  free(buffer);
}
//解析接受到的命令线程的建立函数
void QNetClient::treasmit_pthread_start()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&treasmit_pthread_id,&attr,run_treasmit_pthread,this);
  pthread_attr_destroy (&attr);
}

void QNetClient::start()
{
  signal(SIGPIPE,reply);
  quit = 0;

  treasmit_pthread_start();
  send_pthread_start();
  recv_pthread_start();


}
/*
 * 网络处理函数 发送和接受
 */
int QNetClient::WRITE(int sk, char *buf, int len)
{
  int ret;
  int left = len;
  int pos = 0;

  while (left > 0)
  {
      if((ret = send(sk,&buf[pos], left,0))<=0)
      {
          printf("write data failed!\n");
          quit = 1;
          return -1;
      }

      left -= ret;
      pos += ret;
  }

  return 0;
}
int QNetClient::READ(int sk, char *buf, int len)
{
  int ret;
  int left = len;
  int pos = 0;

  while (left > 0)
  {
    ret = recv(sk,&buf[pos], left,0);
      //if((ret = recv(sk,&buf[pos], left,0))<=0)
      if(ret == -1)
      {
          printf("read data failed!ret,left: %d,%d,%s\n",ret,left,strerror(errno));
          quit = 1;
          return -1;

      }
      else if((ret < 0) && ((errno == EINTR) ||  \
        (errno == EWOULDBLOCK) || (errno == EAGAIN)))
      {
        continue;
      }

      left -= ret;
      pos += ret;
  }

  return 0;
}
void QNetClient::close_client()
{
  shutdown(client_socket,SHUT_RDWR);
}
