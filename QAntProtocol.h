#ifndef QANTPROTOCOL_H
#define QANTPROTOCOL_H
#include "net_protocol.h"
#include "QSlidingWindow.h"
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
  QAntProtocol(QSlidingWindow *sliding);

  static void ctrl_logout_ack(void *ptr);
  static void ctrl_heart_ack(void *ptr);
  static void file_send_ack(void *ptr);
  static void file_name_ack(void *ptr);
  static void file_list_ack(void *ptr);
  static void file_start_ack(void *ptr);
  static void file_path_ack(void *ptr);
protected:

private:
  QSlidingWindow *send_sliding;
  struct _frame_info_t *frame;
  //static QAntProtocol  *pthis;


};
#endif
