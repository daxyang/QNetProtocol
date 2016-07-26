#ifndef QNETCLIENT_H
#define QNETCLIENT_H
#include "net_protocol.h"
#include "QSlidingWindow.h"
#include "QSlidingWindowConsume.h"
#include "pthread.h"
#define PROTOCOL_BUFFER_LEN (2 * 1024 * 1024)
#define SEND_USER 1
#define RECV_USER 2
//命令链表结构体
struct sub_cmd_link_t
{
  int no;
  u32 sub_cmd_type;
  void (*callback)(char *data,u32 len);
  struct sub_cmd_link_t *next;
};

struct cmd_link_t
{
    int no;
    u16 cmd_type;
    struct sub_cmd_link_t *subcmd_head;
    struct cmd_link_t *next;
};

class QNetClient
{
public:
  QNetClient();
  ~QNetClient();
  int client_socket;
  int connect_server(char *ip,int port);
  void set_protocol_ack_callback(u16 cmd_type,u32 sub_cmd_type,void (*function)(char *data,u32 len)); //添加协议处理函数
  //主命令处理
  void main_cmd_process(u16 cmd_type,u32 sub_cmd_type,char *data,u32 len);

  void start();
  void close_client();
private:
  /*  Send_Window（发送缓冲区):
   *      用户将协议添加到SendWindow中，发送线程通过网络进行发送
   *  Recv_Window (接受缓冲区):
   *      从网络中接受到的命令保存在接受缓冲区中，通过解析后调用用户的回调函数进行处理相应命令
   *＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
   * 用户调用函数,将发送命令添加到发送缓冲区中
   *        [send_buffer]
   *               命令发送线程send_pthread_id，从send_buffer中取出命令进行网络发送
   *＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
   * 网络接受线程recv_pthread_id收到命令，添加到接受缓冲区中
   *        ［recv_buffer]
   *              协议解析线程treasmit_pthread_id对接受缓冲区的协议进行解析，调用相应的回调函数进行处理
   *
   */

  QSlidingWindow *send_sliding;  //发送缓冲区生产者
  QSlidingWindow *recv_sliding;  //接受缓冲区生产者
  QSlidingWindowConsume *send_consume;  //发送缓冲区消费者
  QSlidingWindowConsume *recv_consume;  //接受缓冲区消费者
  char *send_buffer;  //发送缓冲区
  char *recv_buffer;  //接受缓冲区
  //线程id
  pthread_t send_pthread_id;
  pthread_t recv_pthread_id;
  pthread_t treasmit_pthread_id;
  //线程处理函数
  static void *run_send_pthread(void *ptr);
  static void *run_recv_pthread(void *ptr);
  static void *run_treasmit_pthread(void *ptr);
  //线程建立函数
  void send_pthread_start();
  void recv_pthread_start();
  void treasmit_pthread_start();
  //网络读写函数
  int WRITE(int sk, char *buf, int len);
  int READ(int sk, char *buf, int len);
private:
  //命令链表操作函数
  /* head_cmd_node  (添加主命令节点:appent_cmd_link_node())
   *   -> NET_TCP_TYPE_CTRL
   *       =>sub_cmd_head:(根据主节点查找子节点的头:search_subcmd_head())
   *            => NET_CTRL_LOGIN  (添加子节点:append_subcmd_link_node(),查找子节点search_subcmd_node())
   *            => NET_CTRL_LOGOUT
   *            => NET_CTRL_HEART
   *            => ...
   *   -> NET_TCP_TYPE_FILE
   *       =>sub_cmd_head:(根据主节点查找子节点的头:search_subcmd_head())
   *            => NET_FILE_LIST
   *            => NET_FILE_START
   *            => ...
   *   -> NET_TCP_TYPE_VID
   *   -> NET_TCP_TYPE_AID
   */
  struct cmd_link_t *head_cmd_node;   //主命令的链表头
  void append_cmd_link_node(u16 cmd_type);  //添加一个主命令节点
  struct sub_cmd_link_t *search_subcmd_head(u16 cmd_type);  //根据主命令查找子命令头
  struct sub_cmd_link_t *search_subcmd_node(u16 cmd_type,u32 sub_cmd_type);  //查找子命令节点
  void append_subcmd_link_node(u16 cmd_type,u32 sub_cmd_type,void (*function)(char *data,u32 len)); //添加子命令的处理函数
  /*
   * 协议处理部分
   */

  //子命令处理
  /* 添加子命令处理函数(添加一)
  void do_xxx_protocol_process(u32 sub_cmd_type,char *data,u32 len);
   */
  void do_ctrl_protocol_process(u32 sub_cmd_type,char *data,u32 len);
  void do_file_protocol_process(u32 sub_cmd_type,char *data,u32 len);
  void do_audio_protocol_process(u32 sub_cmd_type,char *data,u32 len);
  void do_video_protocol_process(u32 sub_cmd_type,char *data,u32 len);

  struct _frame_info_t *frame;
};




#endif
