#include "QNetClient.h"
#include "stdlib.h"
#include "stdio.h"
QNetClient *client;
FILE *file;
int file_serial;
u32 file_len;
u32 read_file_count;
int viewinfo;
int stream_id;
void login_ack(char *data, u32 len)
{
    app_net_ctrl_ack_login *login = (app_net_ctrl_ack_login *)data;
    if(ntohs(login->state) == APP_NET_LOGIN_SUCCESS)
    {
        printf("login success!\n\n");
    }
    else
        printf("login failed!\n\n");

}


void logout_ack(char *data, u32 len)
{
    app_net_ctrl_ack_logout *logout = (app_net_ctrl_ack_logout *)data;
    printf("logout state:%d\n",ntohs(logout->state));
    //pthis->client->stop_heart();
    client->close_client();

}
void heart_ack(char *data, u32 len)
{

}
void filelist_ack(char *data, u32 len)
{
    app_net_file_ack_list *list = (app_net_file_ack_list *)data;
}


void filesend_ack(char *data, u32 len)
{
    u32 pkg_len = sizeof(app_net_file_ack_send);
    u32 data_len = len - pkg_len;
    app_net_file_ack_send *filesend_ack = (app_net_file_ack_send *)data;
    int ret = fwrite(data+pkg_len,1,data_len,file);
    read_file_count += ret;
    if(read_file_count > (file_len / 100) * (viewinfo * 10))
    {
      printf("%d%%  \n",viewinfo * 10);
      viewinfo++;
    }

    if(data_len < 1024)
    {
        fclose(file);
    }
    else
    {
        file_serial++;
        u32 len = sizeof(app_net_file_send);
        char *buffer = (char *)malloc(sizeof(char) * len);
        app_net_file_send *file_send = (app_net_file_send *)buffer;
        file_send->len = htons(1024);
        file_send->serial = htonl(file_serial);
        client->main_cmd_process(NET_TCP_TYPE_FILE,NET_FILE_SEND,buffer,len);
        //printf("serial:%d",file_serial);
        //usleep(10000);
        free(buffer);
    }
    //usleep(40000);
}

void connect(char *ip)
{
    int port = 6666;
    client->connect_server(ip,port);
    //usleep(200);
    client->start();
    usleep(2000);
    u32 len = sizeof(app_net_ctrl_login);
    char *buffer = (char *)malloc(sizeof(char) * len);
    app_net_ctrl_login *login = (app_net_ctrl_login *)buffer;
    strcpy(login->name,"admin");
    strcpy(login->passwd,"123456");
    client->main_cmd_process(NET_TCP_TYPE_CTRL,NET_CTRL_LOGIN,buffer,len);
}

// void MainWindow::on_pushButton_logout_clicked()
// {
//
//     client->main_cmd_process(NET_TCP_TYPE_CTRL,NET_CTRL_LOGOUT,NULL,0);
// }

void send_command(int index)
{
    switch(index)
    {
    case 2:
    {
        u32 len = sizeof(app_net_file_path);
        char *buffer = (char *)malloc(sizeof(char) * len);
        app_net_file_path *path = (app_net_file_path *)buffer;
        strcpy(path->path,"/home/qianzhengyang/nfs");
        client->main_cmd_process(NET_TCP_TYPE_FILE,NET_FILE_PATH,buffer,len);

        free(buffer);
    }
        break;
    case 3:
    {
        client->main_cmd_process(NET_TCP_TYPE_FILE,NET_FILE_LIST,NULL,0);
    }
        break;
    case 4:
    {
        file_serial = 0;
        read_file_count = 0;
        viewinfo = 1;
        file = fopen("/Users/qianzhengyang/Desktop/qzy.zip","w");
        u32 len = sizeof(app_net_file_send);
        char *buffer = (char *)malloc(sizeof(char) * len);
        app_net_file_send *file_send = (app_net_file_send *)buffer;
        file_send->len = htons(1024);
        file_send->serial = htonl(file_serial);
        client->main_cmd_process(NET_TCP_TYPE_FILE,NET_FILE_SEND,buffer,len);
        usleep(10000);
        free(buffer);
    }
        break;
    case 5:
    {
        u32 len = sizeof(app_net_file_start_read);
        printf("start struct len:%d \n",len);
        char *buffer = (char *)malloc(sizeof(char) * len);
        app_net_file_start_read *start = (app_net_file_start_read *)buffer;
        strcpy(start->filename,"/Users/qianzhengyang/testfile.mp4");
        //strcpy(start->filename,"/root/Videos/testfile.mp4");
        printf("file:%s\n",start->filename);
        client->main_cmd_process(NET_TCP_TYPE_FILE,NET_FILE_START,buffer,len);

        free(buffer);
    }
        break;
    case 6:
    {
      printf("video connect\n");
      u32 len = sizeof(app_net_vid_connect);
      char *buffer = (char *)malloc(sizeof(char) * len);
      app_net_vid_connect *connect = (app_net_vid_connect *)buffer;
      connect->streamid = stream_id;
      client->main_cmd_process(NET_TCP_TYPE_VID,NET_VID_CONNECT,buffer,len);
      free(buffer);
    }
      break;
    case 7:
      {
        printf("get stream command\n");
        client->main_cmd_process(NET_TCP_TYPE_VID,NET_VID_STREAM,NULL,0);
      }
      break;

    }
}
void filepath_ack(char *data, u32 len)
{
    app_net_file_ack_path *path = (app_net_file_ack_path *)data;

}

void filestart_ack(char *data, u32 len)
{
  app_net_file_ack_start_read *read = (app_net_file_ack_start_read *)data;
  file_len = ntohl(read->file_len);
  printf("file len:%d\n",file_len);
}
void set_read_file_menu()
{
  printf("***** set read file menu *****\n");
}
void read_file_menu()
{
  printf("***** read file ******\n");

}
void vidstream_ack(char *data,u32 len)
{
  printf("stream len:%d\n",len);
}
void main_menu()
{
  int index;
  printf("****** Main Menu ***********\n");
  printf("*** 4 - read file **********\n");
  printf("*** 5 - set read file name *\n");
  printf("*** 6 - send stream connect*\n");
  printf("*** 7 - get dvo_ipc stream *\n");
  printf("intput:");
  scanf("%d", &index);
  send_command(index);
  usleep(100000);
  switch (index) {
    case 4:
      read_file_menu();
    break;
    case 5:
      set_read_file_menu();
    break;
  }
}
void close_socket(int sig)
{
    client->quit = 1;
    printf("\033[31mClose client\033[0\n");
    //free(client);
    delete client;
    exit(0);
}
int main(int argc,char **argv)
{
  if(argc != 3)
    return 0;
  client = new QNetClient();
  client->set_protocol_ack_callback(NET_TCP_TYPE_CTRL,NET_CTRL_LOGIN,login_ack);
  client->set_protocol_ack_callback(NET_TCP_TYPE_CTRL,NET_CTRL_LOGOUT,logout_ack);
  client->set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_LIST,filelist_ack);
  client->set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_SEND,filesend_ack);
  client->set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_PATH,filepath_ack);
  client->set_protocol_ack_callback(NET_TCP_TYPE_FILE,NET_FILE_START,filestart_ack);
  client->set_protocol_ack_callback(NET_TCP_TYPE_VID,NET_VID_STREAM,vidstream_ack);
  client->reply = close_socket;
  file_serial = 0;
  stream_id = atoi(argv[2]);
  connect(argv[1]);
  usleep(100000);
  while(1)
    main_menu();


}
