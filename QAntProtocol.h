#ifndef QANTPROTOCOL_H
#define QANTPROTOCOL_H
#include "net_protocol.h"
#include "QSlidingWindow.h"
#include "stdio.h"
#include "dirent.h"
#include "sys/stat.h"
#include "sys/shm.h"
//#include "GetDVOStream.h"
#include "QGetDVOStream.h"
struct cmd_transmit_t
{
    void *ptr;
    char *data;
    u32 len;
};

class QAntProtocol
{
public:
  QAntProtocol();
  ~QAntProtocol();
  QAntProtocol(QSlidingWindow *sliding);

  static void ctrl_logout_ack(void *ptr);
  static void ctrl_heart_ack(void *ptr);
  static void file_send_ack(void *ptr);
  static void file_name_ack(void *ptr);
  static void file_list_ack(void *ptr);
  static void file_start_ack(void *ptr);
  static void file_path_ack(void *ptr);
  //<add by Antony 2016-8-15>
  static void vid_connect_ack(void *ptr);
  static void vid_stream_ack(void *ptr);
  //<!2016-8-15>

  //<add by Antony 2016-8-16>
  int send_stream_pthread_cancel();
  pthread_t stream_pthread_id;
  //<!2016-8-16>
protected:

//<add by antony 2016-8-15>
private:
  static void *run_send_stream(void *ptr);
  void start_send_stream();
//<!2016-8-15>

private:
  QSlidingWindow *send_sliding;
  struct _frame_info_t *frame;
  //static QAntProtocol  *pthis;
  FILE *file;
  char path[1024];

  //<add by Antony 2016-8-17>
  int stream_id;
  //<!2016-8-17>

  QGetDVOStream *get_dvo_stream;
  pthread_mutex_t send_buff_mutex;

};
#endif
