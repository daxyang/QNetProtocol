#include "QAntProtocol.h"
//QAntProtocol *QAntProtocol::pthis = NULL;
QAntProtocol::QAntProtocol()
{

}

QAntProtocol::QAntProtocol(QSlidingWindow *sliding)
{
  frame = new _frame_info_t;
  frame->frame_type = 20;
  printf("sliding start\n");
  if(sliding == NULL)
    printf("sliding is null\n");
  else
  {
    send_sliding = sliding;
    printf("send_sliding:%d,sliding:%d\n",send_sliding,sliding);
  }

//  pthis = this;


}

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
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  printf("logout");
  free(buffer);
}
void QAntProtocol::ctrl_heart_ack(void *ptr)
{
  struct cmd_transmit_t *cmd_ptr = (struct cmd_transmit_t *)ptr;
  QAntProtocol *pthis = (QAntProtocol *)cmd_ptr->ptr;  //类指针
  char *data = (char *)cmd_ptr->data; //接受到协议内容(不含头)
  u32 len = (u32)cmd_ptr->len; //按受到协议的长度

  app_net_ctrl_heart *heart = (app_net_ctrl_heart *)data;
  printf("pid:%d heart time:%d-%d-%d %d:%d:%d\n",getpid(),ntohs(heart->yy),heart->MM,heart->dd,heart->hh,heart->mm,heart->ss);
  //int time_cnt = 0;
  u32 pkg_len = NET_HEAD_SIZE + sizeof(app_net_ctrl_ack_heart);
  char *buffer = (char *)malloc(sizeof(char) * pkg_len);
  app_net_head_pkg_t *head = (app_net_head_pkg_t *)buffer;
  app_net_ctrl_ack_heart *heart_ack = (app_net_ctrl_ack_heart *)(buffer + NET_HEAD_SIZE);

  heart_ack->state = 1;
  printf("heart\n");
  HEAD_PKG(head,NET_TCP_TYPE_CTRL,NET_CTRL_HEART,0,pkg_len);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(buffer);
}


void QAntProtocol::file_name_ack(void *ptr)
{}

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
  printf("1_path:%s\n",pthis->path);
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
        pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
        list_ack->file_number = 0;
      }

    }
    HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
    pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
    closedir(dir);
  }

  //<--
  //HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
  //pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(buffer);
}
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
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(full_file_name);
  free(buffer);
}
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
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(buffer);

}
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
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(buffer);
}
