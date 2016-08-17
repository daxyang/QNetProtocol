#include "QNetTcpServer.h"

QNetTcpServer::QNetTcpServer()
{
  frame = new _frame_info_t;
  frame->frame_type = 20;

  slidingwnd_send = new QSlidingWindow();
  send_buffer = (char *)malloc(sizeof(char) * CMD_BUFFER_LEN);
  if(send_buffer == NULL)
  {
    printf("send buffer is NULL\n");
    return;
  }
  slidingwnd_send->sliding_init(CMD_BUFFER_LEN,send_buffer);
  slidingwnd_send->consume_linklist_append(SEND_USER);
  consume_send = slidingwnd_send->consume_linklist_getConsume(SEND_USER);

  slidingwnd_recv = new QSlidingWindow();
  recv_buffer = (char *)malloc(sizeof(char) * CMD_BUFFER_LEN);
  if(recv_buffer == NULL)
  {
    printf("recv buff is NULL\n");
    return;
  }
  slidingwnd_recv->sliding_init(CMD_BUFFER_LEN,recv_buffer);
  slidingwnd_recv->consume_linklist_append(RECV_USER);
  consume_recv = slidingwnd_recv->consume_linklist_getConsume(RECV_USER);

  ant_protocol = new QAntProtocol(slidingwnd_send);

  //初始化链表
  head_cmd_link = new struct cmd_link_t;
  head_cmd_link->no = 0;
  head_cmd_link->cmd_type = 0;
  head_cmd_link->subcmd_head = NULL;
  head_cmd_link->next = NULL;
  append_cmd_node(NET_TCP_TYPE_CTRL);
  append_cmd_node(NET_TCP_TYPE_FILE);
  append_cmd_node(NET_TCP_TYPE_VID);
  append_cmd_node(NET_TCP_TYPE_AID);
  /*
   *  CTRL类处理函数
   */
  struct sub_cmd_link_t *sub_cmd_logout = new struct sub_cmd_link_t;
  sub_cmd_logout->no = NET_CTRL_LOGOUT;
  sub_cmd_logout->sub_cmd_type = NET_CTRL_LOGOUT;
  //<modify by Antony 2016-8-17>
  //sub_cmd_logout->callback = ctrl_logout_ack;
  sub_cmd_logout->callback = ant_protocol->ctrl_logout_ack;
  //<!2016-8-17>
  sub_cmd_logout->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_CTRL,sub_cmd_logout);

  struct sub_cmd_link_t *sub_cmd_heart = new struct sub_cmd_link_t;
  sub_cmd_heart->no = NET_CTRL_HEART;
  sub_cmd_heart->sub_cmd_type = NET_CTRL_HEART;
  //<modify by Antony 2016-8-17>
  //sub_cmd_heart->callback = ctrl_heart_ack;
  sub_cmd_heart->callback = ant_protocol->ctrl_heart_ack;
  //<!2016-8-17>
  sub_cmd_heart->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_CTRL,sub_cmd_heart);

  /*
   * FILE文件类处理函数
   */
  struct sub_cmd_link_t *sub_cmd_filelist = new struct sub_cmd_link_t;
  sub_cmd_filelist-> no = NET_FILE_LIST;
  sub_cmd_filelist->sub_cmd_type = NET_FILE_LIST;
  //<modify by Antony 2016-8-17>
  //sub_cmd_filelist->callback = file_list_ack;
  sub_cmd_filelist->callback = ant_protocol->file_list_ack;
  //<!2016-8-17>
  sub_cmd_filelist->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_FILE,sub_cmd_filelist);

  struct sub_cmd_link_t *sub_cmd_filesend = new struct sub_cmd_link_t;
  sub_cmd_filesend-> no = NET_FILE_SEND;
  sub_cmd_filesend->sub_cmd_type = NET_FILE_SEND;
  //<modify by Antony 2016-8-17>
  //sub_cmd_filesend->callback = file_send_ack;
  sub_cmd_filesend->callback = ant_protocol->file_send_ack;
  //<!2016-8-17>
  sub_cmd_filesend->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_FILE,sub_cmd_filesend);

  struct sub_cmd_link_t *sub_cmd_filepath = new struct sub_cmd_link_t;
  sub_cmd_filepath-> no = NET_FILE_PATH;
  sub_cmd_filepath->sub_cmd_type = NET_FILE_PATH;
  //<modify by Antony 2016-8-17>
  //sub_cmd_filepath->callback = file_path_ack;
  sub_cmd_filepath->callback = ant_protocol->file_path_ack;
  //<!2016-8-17>
  sub_cmd_filepath->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_FILE,sub_cmd_filepath);

  struct sub_cmd_link_t *sub_cmd_filename = new struct sub_cmd_link_t;
  sub_cmd_filename-> no = NET_FILE_NAME;
  sub_cmd_filename->sub_cmd_type = NET_FILE_NAME;
  //<modify by Antony 2016-8-17>
  //sub_cmd_filename->callback = file_name_ack;
  sub_cmd_filename->callback = ant_protocol->file_name_ack;
  //<!2016-8-17>
  sub_cmd_filename->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_FILE,sub_cmd_filename);

  struct sub_cmd_link_t *sub_cmd_filestart = new struct sub_cmd_link_t;
  sub_cmd_filestart-> no = NET_FILE_START;
  sub_cmd_filestart->sub_cmd_type = NET_FILE_START;
  //<modify by Antony 2016-8-17>
  //sub_cmd_filestart->callback = file_start_ack;
  sub_cmd_filestart->callback = ant_protocol->file_start_ack;
  //<!2016-8-17>
  sub_cmd_filestart->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_FILE,sub_cmd_filestart);
  /*
   * AUDIO类处理函数 add by antory 2016-8-15
   */
  struct sub_cmd_link_t *sub_cmd_vid_connect = new struct sub_cmd_link_t;
  sub_cmd_vid_connect-> no = NET_VID_CONNECT;
  sub_cmd_vid_connect->sub_cmd_type = NET_VID_CONNECT;
  //<modify by Antony 2016-8-17>
  //sub_cmd_vid_connect->callback = vid_connect_ack;
  sub_cmd_vid_connect->callback = ant_protocol->vid_connect_ack;
  //<!2016-8-17>
  sub_cmd_vid_connect->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_VID,sub_cmd_vid_connect);

  struct sub_cmd_link_t *sub_cmd_vid_stream = new struct sub_cmd_link_t;
  sub_cmd_vid_stream-> no = NET_VID_STREAM;
  sub_cmd_vid_stream->sub_cmd_type = NET_VID_STREAM;
  //<modify by Antony 2016-8-17>
  //sub_cmd_vid_stream->callback = vid_stream_ack;
  sub_cmd_vid_stream->callback = ant_protocol->vid_stream_ack;
  //<!2016-8-17>
  sub_cmd_vid_stream->next = NULL;
  append_sub_cmd_node(NET_TCP_TYPE_VID,sub_cmd_vid_stream);

  //<!2016-8-15>
  /*
   * 添加协议处理函数
   */

}
QNetTcpServer::~QNetTcpServer()
{
  delete slidingwnd_recv;
  delete slidingwnd_send;
  delete consume_recv;
  delete consume_send;
  delete ant_protocol;
  free(send_buffer);
  free(recv_buffer);
  printf("free ant_protocol:%d\n",socket);
  printf("free send_buffer:%d\n",socket);
  printf("free recv_buffer:%d\n",socket);
  send_buffer = NULL;
  recv_buffer = NULL;
}
int QNetTcpServer::login(int sk)
{
  int loginstate = -1;
  u32 len = NET_HEAD_SIZE + sizeof(app_net_ctrl_login);
  char *buffer = (char *)malloc(sizeof(char) * len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_ctrl_login *login = (app_net_ctrl_login *)(buffer + NET_HEAD_SIZE);
  int ret = READ(sk,buffer,len);
  if(ret == -1)
  {
    free(buffer);
    return -1;
  }
  u16 cmdtype = ntohs(head->CmdType);
  u32 cmdsubtype = ntohl(head->CmdSubType);

  u32 ack_len = NET_HEAD_SIZE + sizeof(app_net_ctrl_ack_login);
  char *buffer_ack = (char *)malloc(sizeof(char) * ack_len);
  head = (app_net_head_pkg_t *)buffer_ack;
  app_net_ctrl_ack_login *ack_login = (app_net_ctrl_ack_login *)(buffer_ack + NET_HEAD_SIZE);

  if((cmdtype == NET_TCP_TYPE_CTRL) && (cmdsubtype == NET_CTRL_LOGIN))
  {
      if(strcmp(login->name,"admin") == 0 && strcmp(login->passwd,"123456") == 0)
      {
        ack_login->state = htons(APP_NET_LOGIN_SUCCESS);
        loginstate = 1;
      }
      else
      {
        ack_login->state = htons(APP_NET_LOGIN_PASSWD_FAILE);
        loginstate = -1;
      }
  }
  else
    loginstate = -1;
  HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_LOGIN,0,ack_len);
  WRITE(sk,buffer_ack,ack_len);
  free(buffer);
  free(buffer_ack);
  return loginstate;
}
/*
 * 协议解析线程
 *   （它是接受缓冲区:recv_buffer的消费者和发送缓冲区的生产者)
 */
void *QNetTcpServer::run_cmd_process(void *ptr)
{
  printf("cmd_process pthread start!\n");
  QNetTcpServer *pthis = (QNetTcpServer *)ptr;
  pthis->consume_recv->read_init();
  char *buffer = (char *)malloc(sizeof(char) * 2 * 1024 * 1024);
  while(pthis->quit == 0)
  {
    int len = pthis->consume_recv->read_data_to_buffer(buffer,0);
    if(len > 0)
    {
      app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
      u16 cmd_type = ntohs(head->CmdType);
      u32 cmd_sub_type = ntohl(head->CmdSubType);
      u32 pkg_len =  ntohl(head->Length) - NET_HEAD_SIZE;
      //printf("cmd:%d sub:%d len:%d\n",cmd_type,cmd_sub_type,pkg_len);
      //将数据进行分析(不含头)
      pthis->do_cmd_process(cmd_type,cmd_sub_type,pkg_len,buffer+NET_HEAD_SIZE);
    }
    // usleep(10000);
    // #if defined(Q_OS_WIN32)
    //   usleep(1000);
    // #elif defined(Q_OS_MACX)
    //   pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //   //usleep(5000);
    //   pthread_yield();
    // #endif
  }
  printf("cmd process thread quit:%d!\n",pthis->quit);
  free(buffer);
}

/*
 * 接受线程
 *    （这是接受缓冲区:recv_buffer的生产者）
 */
void *QNetTcpServer::run_recv_cmd(void *ptr)
{
  printf("Reveive pthread start!\n");
  QNetTcpServer *pthis = (QNetTcpServer *)ptr;

  while(pthis->quit == 0)
  {
    char *head_buffer = (char *)malloc(sizeof(char) * NET_HEAD_SIZE);
    app_net_head_pkg_t *head = (app_net_head_pkg_t *)head_buffer;

    int rlen = pthis->READ(pthis->socket,head_buffer,NET_HEAD_SIZE);
    if(rlen == -1)
    {
      printf("read data len : 0_0\n");
      pthis->quit = 1;
      usleep(500000);
      return NULL;
    }
    u32 len = ntohl(head->Length);
    //接受到数据
    char *data_buffer = (char *)malloc(sizeof(char) * len);
    memcpy(data_buffer,head_buffer,NET_HEAD_SIZE);
    rlen = pthis->READ(pthis->socket,data_buffer + NET_HEAD_SIZE,len - NET_HEAD_SIZE);
    if(rlen == -1)
    {
      printf("read data len : 0_1\n");
       pthis->quit = 1;
       usleep(500000);
       return NULL;
    }
    //将数据(含包头)写入接受缓冲区
    pthis->slidingwnd_recv->write_data_to_buffer(len,data_buffer,pthis->frame);
    free(head_buffer);
    free(data_buffer);

    // #if defined(Q_OS_WIN32)
    //   usleep(1000);
    // #elif defined(Q_OS_MACX)
    //   pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //   pthread_yield();
    // #endif
  }
  printf("recv pthread quit:%d!\n",pthis->quit);
}
/*
 * 启动命令分析线程
 */
void QNetTcpServer::start_treasmit()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&treasmit_pthread_id,&attr,run_cmd_process,this);
  pthread_attr_destroy (&attr);
}
/*
 * 启动接受线程
 */
void QNetTcpServer::start_recv()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&recv_pthread_id,&attr,run_recv_cmd,this);
  pthread_attr_destroy (&attr);

}
/*
 * 启动发送线程
 *   （这是发送缓冲区:send_buffer的消费者)
 */
void QNetTcpServer::start_send()
{
    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&send_pthread_id,&attr,run_send_cmd,this);
    pthread_attr_destroy (&attr);

}
void *QNetTcpServer::run_send_cmd(void *ptr)
{
  printf("send pthread start!\n");
  QNetTcpServer *pthis = (QNetTcpServer *)ptr;
  char *buffer = (char *)malloc(sizeof(char) * 1024 * 1024);
  pthis->consume_send->read_init();

  while(pthis->quit == 0)
  {
      //从发送缓冲区中读取命令进行发送
    int len = pthis->consume_send->read_data_to_buffer(buffer,0);  //使用非阻塞模式，可以使该线程在quit=1时退出
    if(len > 0)
    {
      pthis->WRITE(pthis->socket,buffer,len);
          //printf("send len:%d\n", len);
    }
    //usleep(10000);
    // #if defined(Q_OS_WIN32)
    //   usleep(1000);
    // #elif defined(Q_OS_MACX)
    //   pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //   //usleep(5000);
    //   pthread_yield();
    // #endif
  }
  printf("send pthread quit:%d!\n",pthis->quit);
  free(buffer);
}
void QNetTcpServer::server_start(int sk)
{
  socket = sk;
  quit = 0;
  start_send();
  start_treasmit();
  start_recv();
  //usleep(100000);

}

/*  添加通讯协议(2)
 * 根据cmdtype进行命令分发
 */
void QNetTcpServer::do_cmd_process(u16 cmdtype,u32 cmdsubtype,u32 len,char *data)
{
  switch (cmdtype) {
    case NET_TCP_TYPE_CTRL:
      do_ctrl_process(cmdsubtype,len,data);
    break;
    case NET_TCP_TYPE_FILE:
      do_file_process(cmdsubtype,len,data);
    break;
    case NET_TCP_TYPE_VID:
      do_vid_process(cmdsubtype,len,data);
    break;
    case NET_TCP_TYPE_AID:
    break;
    /*添加主命令处理协议(例)
     * case NET_TCP_TYPE_xxx:
     * 		do_xxx_process();
     * break;
     */
    default:
    break;

  }
}
/*
 * 添加通讯协议 （3)
 * 子命令处理 do_xxx_process(u32 cmdsubtype,u32 len,char *data)
 */
void QNetTcpServer::do_ctrl_process(u32 cmdsubtype,u32 len,char *data)
{
  struct cmd_transmit_t cmd;
  cmd.data = data;
  cmd.ptr = ant_protocol;
  cmd.len = len;
  switch (cmdsubtype) {
  case NET_CTRL_LOGOUT:
  {
    struct sub_cmd_link_t *logout = search_subcmd_node(NET_TCP_TYPE_CTRL,NET_CTRL_LOGOUT);
    printf("NET_CTRL_LOGOUT\n");
    logout->callback(&cmd);
  }
  break;
  case NET_CTRL_HEART:
  {
    struct sub_cmd_link_t *heart = search_subcmd_node(NET_TCP_TYPE_CTRL,NET_CTRL_HEART);
    printf("NET_CTRL_HEART:%d ",socket);
    heart->callback(&cmd);
  }
  break;
  /*(例)
   * case NET_xxx_xxx:
   * break;
   */
  default:
  break;
  }
}
void QNetTcpServer::do_file_process(u32 cmdsubtype,u32 len,char *data)
{
  struct cmd_transmit_t cmd;
  cmd.data = data;
  cmd.ptr = ant_protocol;
  cmd.len = len;
  switch (cmdsubtype) {
  case NET_FILE_LIST:
  {
    struct sub_cmd_link_t *filelist = search_subcmd_node(NET_TCP_TYPE_FILE,NET_FILE_LIST);
    printf("NET_FILE_LIST\n");
    filelist->callback(&cmd);
  }
  break;
  case NET_FILE_SEND:
  {
    struct sub_cmd_link_t *filesend = search_subcmd_node(NET_TCP_TYPE_FILE,NET_FILE_SEND);
    printf("NET_FILE_SEND\n");
    filesend->callback(&cmd);
  }
  break;
  case NET_FILE_PATH:
  {
    struct sub_cmd_link_t *filepath = search_subcmd_node(NET_TCP_TYPE_FILE,NET_FILE_PATH);
    printf("NET_FILE_PATH\n");
    filepath->callback(&cmd);
  }
  break;
  case NET_FILE_NAME:
  {
    struct sub_cmd_link_t *filename = search_subcmd_node(NET_TCP_TYPE_FILE,NET_FILE_NAME);
    printf("NET_FILE_PATH\n");
    filename->callback(&cmd);
  }
  break;
  case NET_FILE_START:
  {
    struct sub_cmd_link_t *filestart = search_subcmd_node(NET_TCP_TYPE_FILE,NET_FILE_START);
    printf("NET_FILE_PATH\n");
    filestart->callback(&cmd);
  }
  break;
  }
}
void QNetTcpServer::do_vid_process(u32 cmdsubtype,u32 len,char *data)
{
  struct cmd_transmit_t cmd;
  cmd.data = data;
  cmd.ptr = ant_protocol;
  cmd.len = len;
  switch (cmdsubtype) {
    case NET_VID_CONNECT:
    { //<add by Antony 2016-8-17>
      struct sub_cmd_link_t *connect_ack = search_subcmd_node(NET_TCP_TYPE_VID,NET_VID_CONNECT);
      printf("NET_VID_CONNECT\n");
      connect_ack->callback(&cmd);
    } //<!2016-8-17>
    break;
    case NET_VID_STREAM:
    {
    struct sub_cmd_link_t *stream_ack = search_subcmd_node(NET_TCP_TYPE_VID,NET_VID_STREAM);
    printf("NET_VID_STREAM\n");
    stream_ack->callback(&cmd);
    }
    break;
  }
}
//添加主节点
void QNetTcpServer::append_cmd_node(u16 cmdtype)
{
  struct sub_cmd_link_t *head_sub_cmd_link = new sub_cmd_link_t;
  head_sub_cmd_link->no = 0;
  head_sub_cmd_link->sub_cmd_type = 0;
  head_sub_cmd_link->callback = NULL;
  head_sub_cmd_link->next = NULL;

  struct cmd_link_t *new_cmd_link = new struct cmd_link_t;
  new_cmd_link->no = cmdtype;
  new_cmd_link->cmd_type = cmdtype;
  new_cmd_link->subcmd_head = head_sub_cmd_link;
  new_cmd_link->next = NULL;

  struct cmd_link_t *p1;
  p1 = head_cmd_link;
  while(p1->next != NULL)
  {
      p1 = p1->next;
  }
  if(p1->next == NULL)
  {
      p1->next = new_cmd_link;
  }
  head_cmd_link->no++;
}
//添加子节点
void QNetTcpServer::append_sub_cmd_node(u16 cmd_type,struct sub_cmd_link_t *sub_cmd_link)
{
  struct sub_cmd_link_t *head_subcmd_link;
  head_subcmd_link = search_subcmd_head(cmd_type);

  struct sub_cmd_link_t *p1;
  p1 = head_subcmd_link;
  while(p1->next != NULL)
  {
      p1 = p1->next;
  }
  if(p1->next == NULL)
      p1->next = sub_cmd_link;
  head_subcmd_link->no++;
}
//根据主节点，搜索子节点的头
struct sub_cmd_link_t *QNetTcpServer::search_subcmd_head(u16 cmd_type)
{
  struct cmd_link_t *p1;
  p1 = head_cmd_link->next;
  for(int i = 0;i < head_cmd_link->no;i++)
  {
    if(p1->cmd_type == cmd_type)
    {
      return p1->subcmd_head;
    }
    p1 = p1->next;
  }
  return NULL;
}
//搜索子节点
struct sub_cmd_link_t *QNetTcpServer::search_subcmd_node(u16 cmd_type,u32 sub_cmd_type)
{
  struct sub_cmd_link_t *p1;
  struct sub_cmd_link_t *head_subcmd;
  head_subcmd = search_subcmd_head(cmd_type);
  if(head_subcmd == NULL)
  {
    printf("head_subcmd is NULL\n");
  }
  p1 = head_subcmd->next;
  for(int i = 0;i < head_subcmd->no;i++)
  {
    if(p1->sub_cmd_type == sub_cmd_type)
      return p1;
    p1 = p1->next;
  }
  return NULL;
}
/*
 * 网络处理函数 发送和接受
 */
int QNetTcpServer::WRITE(int sk, char *buf, int len)
{
  int ret;
  int left = len;
  int pos = 0;

  while (left > 0)
  {
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    fd_set wset;

    FD_ZERO(&wset);
    FD_SET(sk,&wset);
    int ret = select(sk + 1,NULL,&wset,NULL,&tv);
    if(ret <= 0)
    {
      printf("select write over timer! %d\n",sk);
      pthread_cancel(ant_protocol->stream_pthread_id);
      quit = 1;
      usleep(500000);
      return -1;
    }
    if(FD_ISSET(sk,&wset))
    {
      if((ret = send(sk,&buf[pos], left,0))<=0)
      {
          printf("write data failed!\n");
          return -1;
      }
    }
    left -= ret;
    pos += ret;
  }

  return 0;
}
int QNetTcpServer::READ(int sk, char *buf, int len)
{
  int ret;
  int left = len;
  int pos = 0;

  while (left > 0)
  {
    struct timeval tv;
    tv.tv_sec = 8;
    tv.tv_usec = 0;
    fd_set rset;

    FD_ZERO(&rset);
    FD_SET(sk,&rset);
    int ret = select(sk + 1,&rset,NULL,NULL,&tv);
    if(ret <= 0)
    {
      printf("select recv over timer! %d\n",sk);
      quit = 1;
      pthread_cancel(ant_protocol->stream_pthread_id);
      usleep(500000);
      close(sk);
      return -1;
    }
    if(FD_ISSET(sk,&rset))
    {
      if((ret = recv(sk,&buf[pos], left,0))<=0)
      {
        printf("read data failed!ret,left: %d,%d,%s\n",ret,left,strerror(errno));
        return -1;
      }
    }

    left -= ret;
    pos += ret;
  }

  return 0;
}
