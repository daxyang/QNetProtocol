#include "QAntProtocol.h"
#include "pthread.h"
QAntProtocol::QAntProtocol()
{

}
QAntProtocol::~QAntProtocol()
{
}
QAntProtocol::QAntProtocol(QSlidingWindow *sliding)
{
  frame = new _frame_info_t;
  frame->frame_type = 20;
  if(sliding == NULL)
    printf("sliding is null\n");
  else
  {
    send_sliding = sliding;
    pthread_mutex_init(&send_buff_mutex,NULL);
  }
}
//登出应答函数
void QAntProtocol::ctrl_logout_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  //<client data process>
  // None
  //<!client data process>
  //<server ack>
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_ctrl_ack_logout);
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_ctrl_ack_logout *ack_logout = (app_net_ctrl_ack_logout *)(buffer + NET_HEAD_SIZE);
  ack_logout->state = htons(1);
  HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_LOGOUT,0,pkg_len);
  //<!server ack>
  pthread_mutex_lock(&pthis->send_buff_mutex);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  pthread_mutex_unlock(&pthis->send_buff_mutex);
  printf("logout");
  free(buffer);
}
//心跳应答函数
void QAntProtocol::ctrl_heart_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度

  app_net_ctrl_heart *heart = (app_net_ctrl_heart *)data;
  printf("heart time:%d-%d-%d %d:%d:%d\n",ntohs(heart->yy),heart->MM,heart->dd,heart->hh,heart->mm,heart->ss);
  //int time_cnt = 0;
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_ctrl_ack_heart);
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_ctrl_ack_heart *heart_ack = (app_net_ctrl_ack_heart *)(buffer + NET_HEAD_SIZE);

  heart_ack->state = 1;
  HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_HEART,0,pkg_len);
  pthread_mutex_lock(&pthis->send_buff_mutex);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  pthread_mutex_unlock(&pthis->send_buff_mutex);
  free(buffer);
}


void QAntProtocol::file_name_ack(void *ptr)
{}
//文件列表应答函数
void QAntProtocol::file_list_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度
  //协议分析-->

  //<--
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_file_ack_list);
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_file_ack_list *list_ack = (app_net_file_ack_list *)(buffer + NET_HEAD_SIZE);
  //返回值设置 -->
  list_ack->file_number = 0;
  DIR *dir;
  struct dirent *dir_ptr;
  struct stat statbuf;
  dir = opendir(pthis->path);
  printf("path:%s\n",pthis->path);
  if(dir != NULL)
  {
    chdir(pthis->path);
    while((dir_ptr = readdir(dir)) != NULL)
    {
      if(dir_ptr->d_name[0] == '.')
        continue;
      lstat(dir_ptr->d_name,&statbuf);
      if((statbuf.st_mode & S_IFMT) == S_IFDIR)
        continue;
      strcpy(&list_ack->file[list_ack->file_number][0],dir_ptr->d_name);
      list_ack->file_number++;
      if(list_ack->file_number >= 20)
      {
        HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
        pthread_mutex_lock(&pthis->send_buff_mutex);
        pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
        pthread_mutex_unlock(&pthis->send_buff_mutex);
        list_ack->file_number = 0;
      }

    }
    HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
    pthread_mutex_lock(&pthis->send_buff_mutex);
    pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
    pthread_mutex_unlock(&pthis->send_buff_mutex);
    closedir(dir);
  }

  //<--
  //HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
  //pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(buffer);
}
//文件发送开始应答函数
void QAntProtocol::file_start_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;
  char *data = (char *)cmd_ptr->data;
  u32 len = (u32)cmd_ptr->len;
  //协议解析 =>
  printf("start recv len: %d\n",len);
  app_net_file_start_read *read = (app_net_file_start_read *)data;
  printf("file:%s\n",read->filename);
  //<=

  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_file_ack_start_read);
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_file_ack_start_read *filestart_ack = (app_net_file_ack_start_read *)(buffer + NET_HEAD_SIZE);
  //返回值设值 =>
  u32 size;
  char *full_file_name = (char *)malloc(sizeof(char) * 2048);
  if(strlen(read->filename) != 0)
  {
    sprintf(full_file_name,"%s/%s",pthis->path,read->filename);
    if(access(full_file_name,F_OK) == 0)
    {
      pthis->file = fopen(full_file_name,"rb");
      fseek(pthis->file,0L,SEEK_END);
      size = ftell(pthis->file);
      fseek(pthis->file,0L,0);
    }
    else
      size = 0;
  }
  else
    size = 0;
  filestart_ack->file_len = htonl(size);
  //<=
  HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_START,0,pkg_len);
  pthread_mutex_lock(&pthis->send_buff_mutex);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  pthread_mutex_unlock(&pthis->send_buff_mutex);
  free(full_file_name);
  free(buffer);
}
//文件路径应答函数
void QAntProtocol::file_path_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度
  //协议分析 -->
  app_net_file_path *file_path = (app_net_file_path *)data;
  memset(pthis->path,0,1024);
  strcpy(pthis->path,file_path->path);
  //<--
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_file_ack_path);
  char *buffer = (char *)malloc(sizeof(char) * (pkg_len));
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_file_ack_path *path_ack = (app_net_file_ack_path *)(buffer + NET_HEAD_SIZE);
  //返回值设置 -->
  DIR *dir;
  if((dir = opendir(pthis->path)) == NULL)
    path_ack->state = htons(0);
  else
  {
    path_ack->state = htons(1);
    closedir(dir);
  }
  //<--
  HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_PATH,0,pkg_len);
  pthread_mutex_lock(&pthis->send_buff_mutex);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  pthread_mutex_unlock(&pthis->send_buff_mutex);
  free(buffer);

}
//文件发送应答函灵敏
void QAntProtocol::file_send_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度
  //协议分析-->
  app_net_file_send *file_send = (app_net_file_send *)data;
  u16 file_data_len = ntohs(file_send->len);
  u32 serial = ntohl(file_send->serial);
  //<--
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_file_ack_send);
  char *buffer = (char *)malloc(sizeof(char) * (pkg_len + file_data_len));
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_file_ack_send *send_ack = (app_net_file_ack_send *)(buffer + NET_HEAD_SIZE);
  //返回值设置 -->
  if(pthis->file != NULL)
  {
    send_ack->serial = htonl(serial);
    u32 file_read_len = fread(buffer+NET_HEAD_SIZE+sizeof(app_net_file_ack_send),sizeof(char),file_data_len,pthis->file);
    pkg_len += file_read_len;
    printf("pkg_len:%d,read_len:%d:serial:%d\n",pkg_len,file_read_len,serial);
    if(file_read_len < file_data_len)
      fclose(pthis->file);
  }
  //<--
  HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_SEND,0,pkg_len);
  pthread_mutex_lock(&pthis->send_buff_mutex);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  pthread_mutex_unlock(&pthis->send_buff_mutex);
  free(buffer);
}
//<add by antony 2016-8-15>
//视频发送连接
void QAntProtocol::vid_connect_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度
  //<协议分析>
  app_net_vid_connect *vid_connect = (app_net_vid_connect *)data;
  pthis->stream_id = vid_connect->streamid;
  printf("current get stream id:%d\n",pthis->stream_id);
  //<!协议分析>
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_vid_ack_connect);
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  //<返回值>
  app_net_vid_ack_connect *vid_ack_connect = (app_net_vid_ack_connect *)(buffer+NET_HEAD_SIZE);
  vid_ack_connect->stat = htons(1);
  //<!返回值>
  HEAD_PKG(head,NET_TCP_TYPE_VID,NET_VID_CONNECT,0,pkg_len);
  pthread_mutex_lock(&pthis->send_buff_mutex);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  pthread_mutex_unlock(&pthis->send_buff_mutex);
  free(buffer);
}
//取码流协议－接收到取码流的命令
void QAntProtocol::vid_stream_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度
  pthis->get_dvo_stream = new QGetDVOStream();
  //<协议分析>
  //none
  //<!协议分析>
  pthis->start_send_stream();
}
/*
 * 启动码流发送线程
 */
void QAntProtocol::start_send_stream()
{
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL); //异步取消
  pthread_create(&stream_pthread_id,&attr,run_send_stream,this);
  pthread_attr_destroy (&attr);
}
int QAntProtocol::send_stream_pthread_cancel()
{
   pthread_cancel(stream_pthread_id);
   pthread_join(stream_pthread_id,NULL);
}
/*
 * 发送码流的线程
 */
void *QAntProtocol::run_send_stream(void *ptr)
{
  QAntProtocol *pthis = (QAntProtocol *)ptr;

  u32 len = NET_HEAD_SIZE + sizeof(app_net_vid_ack_stream);
  char *buffer = (char *)malloc(sizeof(char) * len);
  char *stream_data = (char *)malloc(sizeof(char) * 1024 *1024);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_vid_ack_stream *stream = (app_net_vid_ack_stream *)(buffer + NET_HEAD_SIZE);
  //init_stream_buf(pthis->stream_id);
  pthis->get_dvo_stream->init_stream_buf(pthis->stream_id);
  //<add by Antony 2016-8-17>
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //允许退出线程
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置立即取消
  //<!2016-8-17>
  stream_rtsp_info_t *stream_info = new stream_rtsp_info_t();
  while(1)
  {
    //<返加值设置>
    pthread_testcancel();
    u32 stream_len;//码流长度
    //stream_data = (char *)malloc(sizeof(char) * stream_len);
    //stream_len = get_stream(stream_data,stream_info);
    stream_len = pthis->get_dvo_stream->get_stream(stream_data,stream_info);

    stream->stream_id = htonl(0);
    stream->frame_type = htonl(stream_info->frame_type);
    stream->frame_number = htonl(0);
    stream->sec = htons(0);
    stream->usec = htons(0);
    stream->pts = htonl(0);

    //printf("frame type:%d\n",stream_info->frame_type);
    u32 pkg_len = len + stream_len;
    char *stream_buffer = (char *)malloc(sizeof(char) * pkg_len);
    memcpy(stream_buffer,buffer,len);
    memcpy(stream_buffer + len , stream_data,stream_len);
    head = (app_net_head_pkg_t *)stream_buffer;
    //<!返回值设置>
    HEAD_PKG(head,NET_TCP_TYPE_VID,NET_VID_STREAM,0,pkg_len);
    pthread_mutex_lock(&pthis->send_buff_mutex);
    pthis->send_sliding->write_data_to_buffer(pkg_len,stream_buffer,pthis->frame);
    pthread_mutex_unlock(&pthis->send_buff_mutex);
    free(stream_buffer);
    pthread_testcancel();
    // #if defined(Q_OS_WIN32)
    //           usleep(1000);
    // #elif defined(Q_OS_MACX)
    //           pthread_yield_np();
    // #elif defined(Q_OS_UNIX)
    //         //  usleep(5000);
    //           pthread_yield();
    // #endif

  }
  //<add by Antony 2016-8-19>
  //printf("send stream pthread quit\n");
  //<!2016-8-19>
  free(buffer);
  free(stream_data);
  free(stream_info);
}
