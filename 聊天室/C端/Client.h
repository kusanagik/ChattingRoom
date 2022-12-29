#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <fcntl.h>
#define N 128
#define M 200
#define MAXSIZE 1024
#define ERRLOG(errmsg) do{\
                        perror(errmsg);\
                        printf("%s - %s - %d\n", __FILE__, __func__, __LINE__);\
                        exit(1);\
                        }while(0)


int sockfd;//客户端fd
//通信协议
typedef struct client
{
    char nameid[32];
    char msg[128];//聊天记录
    char password[32];//密码
    char to_name[32];//聊天对象名
    char text[512];//文件内容
    int n;//文件读取大小
    char filename[32];//文件名称
    int flag;//功能标志 注册1 登录2 私聊3等等

}clientlist;
clientlist user;



//创建套接字，连接请求
void C_socket();
//菜单
void Menu();
//功能选择
void SelectFunc();
//注册
void Register();
//第二阶段菜单
void SelectFunc2();
//登录
void Login();
//私聊
void PrivateChat();
//查看所有在线用户
void ViewOnline_user();
//群发
void GroupChat();
//功能标志6 查看聊天记录
void Read();
//传输文件
void TransferFiles();


//客户端接受消息线程函数
void *recv_thread(void *arg);
 //客户端发送消息函数
void *send_thread(void *arg);
 



#endif