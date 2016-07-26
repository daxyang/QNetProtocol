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
void QAntProtocol::file_send_ack(void *ptr)
{}

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
  list_ack->file_number = 2;
  strcpy(&list_ack->file[0][0],"dvo_save_001");
  strcpy(&list_ack->file[1][0],"dvo_save_002");
  //<--
  HEAD_PKG(head,NET_TCP_TYPE_FILE,NET_FILE_LIST,0,pkg_len);
  pthis->send_sliding->write_data_to_buffer(pkg_len,buffer,pthis->frame);
  free(buffer);
}
void QAntProtocol::file_start_ack(void *ptr)
{}
void QAntProtocol::file_path_ack(void *ptr)
{}
