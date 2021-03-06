#ifndef NET_PROTOCOL_H
#define NET_PROTOCOL_H

#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/signal.h>
#include<sys/wait.h>
#include <sys/ioctl.h>  //FIONBIO
#include <arpa/inet.h>  //inet_pton()
#include "errno.h"
#define HELLO_WORLD_SERVER_PORT    6666
#define LENGTH_OF_LISTEN_QUEUE 20

#define TIMEOUT 3

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int         u32;
typedef  unsigned long long u64;
//通讯协议 CmdType
#define NET_TCP_TYPE_CTRL  0x0001 //控制命令
#define NET_TCP_TYPE_FILE   0x0002 //转输文件
#define NET_TCP_TYPE_VID    0x0003 //视频码流
#define NET_TCP_TYPE_AID    0x0004 //音频码流
//通讯协议 CmdSubType
//控制命令
#define NET_CTRL_LOGIN     0x0001
#define NET_CTRL_LOGOUT    0x0002
#define NET_CTRL_HEART     0x0003
//文件转输命令
#define NET_FILE_LIST        0x0001
#define NET_FILE_START       0x0002
#define NET_FILE_PATH        0x0003
#define NET_FILE_SEND        0x0004
#define NET_FILE_NAME        0x0005

//视频传输
#define NET_VID_CONNECT 0x0001
#define NET_VID_STREAM  0x0002


typedef struct app_net_head_pkg_t
{
    char Margic[4];  //4
    u32 Length;  //4
    u16 CmdType;  //2
    u16 Version;  //2
    u32 CmdSubType;  //4
    u32 Hardware;  //2
    u32 DataType; //2
    u64 Sn;  //8
    u16 Rev1;  //2
    u16 Rev2; //2
    u32 Rev3;  //4

} app_net_head_pkg_t;


#define _head_pkg(p,len,cmd_type,version,cmd_sub_type,hardware,datatype,sn,rev1,rev2,rev3) \
    p->Margic[0] = 0x5a; \
    p->Margic[1] = 0xa5; \
    p->Margic[2] = 0x5a; \
    p->Margic[3] = 0xa5; \
    p->Length  = htonl(len); \
    p->CmdType = htons(cmd_type); \
    p->Version = htons(version); \
    p->CmdSubType = htonl(cmd_sub_type); \
    p->Hardware = htons(hardware); \
    p->DataType = htons(datatype); \
    p->Sn = htons(sn); \
    p->Rev1 = htons(rev1); \
    p->Rev2 = htons(rev2); \
    p->Rev3 = htons(rev3);

#define NET_HEAD_SIZE sizeof(struct app_net_head_pkg_t)

#define HEAD_PKG(p,cmd_type,cmd_sub_type,data_type,len) \
    _head_pkg(p,len,cmd_type,0,cmd_sub_type,0,data_type,0,0,0,0)


//*****登录(client)*****
typedef struct
{
    char name[16];
    char passwd[16];
    u32 Rev1;
}app_net_ctrl_login;
//登录(server)
typedef struct
{
    u16 state;
    u16 Rev1;
}app_net_ctrl_ack_login;

enum status{
    APP_NET_LOGIN_SUCCESS = 0,
    APP_NET_LOGIN_USER_FAILE,
    APP_NET_LOGIN_PASSWD_FAILE,
    APP_NET_LOGIN_USER_LOCK,
    APP_NET_LOGIN_UNKNOW,
};
//*****登出(client)******
//none
//登出(server)
typedef struct
{
    u16 state;
}app_net_ctrl_ack_logout;
enum log_out_statue{
    APP_NET_LOGOUT_OK = 0,
    APP_NET_LOGOUT_ERROR,
};
//******文件列表(client)*****
//none
//文件列表(server)
typedef struct
{
    u8 file_number;
    char file[20][256];
    u16 Rev1;
    u16 Rev2;
} app_net_file_ack_list;

//*****文件路径(client)******
typedef struct
{
        char path[1024];
        u16 Rev1;
}app_net_file_path;
//文件路径(server)
typedef struct
{
  u16 state;
  u16 Rev1;
}app_net_file_ack_path;

//*****开始读取文件(client)*****
typedef struct
{
  char filename[1024];
  u32 Rev1;
}app_net_file_start_read;
//开始读取文件(server)
typedef struct
{
  u32 file_len;
}app_net_file_ack_start_read;
//*****传送文件(client)*****
typedef struct
{
        u32 serial;
        u16 len;
        u16 Rev;
}app_net_file_send;
//传送文件(server)
typedef struct
{
        u32 serial;
        u32 Rev1;
}app_net_file_ack_send;
//*****心跳(client)*****
typedef struct
{
    u16  yy;
    u8   MM;
    u8   dd;
    u8		hh;
    u8		mm;
    u8		ss;
    u8  Rev1;
}app_net_ctrl_heart;
//心跳(server)
typedef struct
    {
        u8 state;
        u8 Rev1;
        u16 Rev2;
}app_net_ctrl_ack_heart;

typedef struct
{
		char filename[64];
		u32 Rev1;
}app_net_file_set_fileame;

typedef struct
{
    u16 state;
	  u16 Rev1;
}app_net_file_ack_set_filename;

/******传送视频码流******/
//取码流连接
//client
typedef struct
{
  u8 streamid;
  u8 Rev1;
  u16 Rev2;
}app_net_vid_connect;
//server
typedef struct
{
  u16 stat; //1:OK;0:Err;
  u16 Rev1;
}app_net_vid_ack_connect;

//码流发送
//client
//none
//server
typedef struct
{
  u32 stream_id;
  u32 frame_type;
  u32 frame_number;
  u16 sec;
  u16 usec;
  u32 pts;
  u16 Rev1;
  u16 Rev2;
}app_net_vid_ack_stream;


#endif
