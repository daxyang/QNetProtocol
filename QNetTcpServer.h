#ifndef QNETTCPSERVER_H
#define QNETTCPSERVER_H
#include "QNetServer.h"
#include "net_protocol.h"
#include "QSlidingWindow.h"
#include "QSlidingWindowConsume.h"
#include "QAntProtocol.h"
#include "pthread.h"

#define SEND_USER  1
#define RECV_USER  2
#define CMD_BUFFER_LEN  (2*1024*1024)

//命令链表结构体
struct sub_cmd_link_t
{
  int no;
  u32 sub_cmd_type;
  void (*callback)(void *ptr);
  struct sub_cmd_link_t *next;
};

struct cmd_link_t
{
    int no;
    u16 cmd_type;
    struct sub_cmd_link_t *subcmd_head;
    struct cmd_link_t *next;
};
//<modify by Antony 2016-8-17>
//cass QNetTcpServer:public QAntProtocol
class QNetTcpServer
//<!2016-8-17>
{
public:
  QNetTcpServer();
  ~QNetTcpServer();

  int login(int sk);
  void server_start(int sk);
  int quit;
protected:


private:
  static void *run_recv_cmd(void *ptr);
  static void *run_send_cmd(void *ptr);
  static void *run_cmd_process(void *ptr);
  int socket;


  QSlidingWindow *slidingwnd_recv,*slidingwnd_send;
  QSlidingWindowConsume  *consume_recv,*consume_send;
  struct _frame_info_t *frame;
  char *send_buffer;
  char *recv_buffer;

  pthread_t recv_pthread_id;
  pthread_t send_pthread_id;
  pthread_t treasmit_pthread_id;
  void start_recv();
  void start_send();
  void start_treasmit();

  //协议解析主函数
  void do_cmd_process(u16 cmdtype,u32 cmdsubtype,u32 len,char *data);
  void do_file_process(u32 cmdsubtype,u32 len,char *data); //File类协议
  void do_ctrl_process(u32 cmdsubtype,u32 len,char *data); //Ctrl类协议
  //<add by antony 2016-8-16>
  void do_vid_process(u32 cmdsubtype,u32 len,char *data);
  //<!2016-8-16>


  //命令链表管理
  void append_cmd_node(u16 cmdtype);  //添加主命令节点
  void append_sub_cmd_node(u16 cmd_type,struct sub_cmd_link_t *sub_cmd_link);//添加子命令节点
  struct sub_cmd_link_t *search_subcmd_head(u16 cmd_type); //搜索子节点的头
  struct sub_cmd_link_t *search_subcmd_node(u16 cmd_type,u32 sub_cmd_type);//搜索子节点
  struct cmd_link_t *head_cmd_link;//主节点的头

  QAntProtocol *ant_protocol;

  int WRITE(int sk, char *buf, int len);
  int READ(int sk, char *buf, int len);

};




#endif
