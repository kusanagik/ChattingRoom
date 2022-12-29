#ifndef _SERVER_H_
#define _SERVER_H_
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
#include <sys/stat.h>
#include <fcntl.h>
#define N 128
#define M 200
#define SUCCESS 10000
#define FAILURE 10001
#define MAXSIZE 1024
#define ERRLOG(errmsg) do{\
                        perror(errmsg);\
                        printf("%s - %s - %d\n", __FILE__, __func__, __LINE__);\
                        exit(1);\
                        }while(0)


#define REGISTER 1//注册
#define LOGIN 2 //登录
#define PRIVATE 3 //私聊
#define VIEWUSER 4 //查看在线用户
#define GROUPCHAT 5//群发
#define READ 6//读取
#define TRANSFERFILE 7//文件传输



// 线程池参数
//定义任务节点结构体和线程结构体
struct job//任务结构体
{
    void*(*func)(void *arg);//用于存放任务
    void *arg;//传参
    struct job *next;//队列节点的指针域

};


struct pthreadpool
{
    int m_threadNum;//已开启的线程数
    pthread_t *m_pthreadIDs;//保存在线程池中的id号
    struct job *head;//指向任务队列的头
    struct job *tail;//同上尾

    int m_QueueMaxNum;//最大任务数 
    int m_QueueCurNum;//已有任务数

    pthread_mutex_t mutex;//互斥锁

    pthread_cond_t m_QueueEmpty;//任务队列为空的条件
    pthread_cond_t m_QueueNotEmpty;//任务队列未满的条件
    pthread_cond_t m_QueueNotFull;//任务队列为满的条件
    int pool_close;  
};



//在线用户链表
typedef struct Online_User
{
    char name[32];
    int o_fd;
    struct Online_User *next;
}Online_Userlist;

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

//线程池
typedef struct Myarg
{
    clientlist msg;
    int fd;
}Myarg;

//创建套接字，绑定，监听
int k_socket();
//epoll操作
int k_epoll(int sockfd);
//创建任务队列并添加任务
void ThreadAddJob(struct pthreadpool *pool, void *(*func)(void *arg), void *arg);
//创建线程从队列中取任务
void *ThreadRun(void *arg);  
//创建线程池
struct pthreadpool *InitPthreadPool(int ThreadNum, int QueueMaxNum); 
//销毁线程池
void ThreadDestroy(struct pthreadpool *pool);




//工作函数
void *work(void *arg);
//在线用户链表
Online_Userlist *OnlineUser_Create();
//向数据库插入注册信息
void Insert_data(Myarg *node , clientlist *c);
//需要实现插入数据和查找数据
void Register_User(Myarg *node,clientlist *c);
//数据库查找
int Searchtable(char *tablename);
//登录
void Login(Myarg *node,clientlist *c);
//添加在线人数
void Online_Insert(Myarg *node,clientlist *c);
//核实密码是否错误 1for正确  0for错误
int Verify(clientlist *c);
//私聊功能,先结收消息再转发给客户端指定的客户机
void PrivateChat(Myarg *node,clientlist *c);
//判断是否在线
int Find_person(clientlist *c);
//查看在线人数
void ViewUser(Myarg *node,clientlist *c);
//群发消息
void Fsend(Myarg *node,clientlist *c);

//保存聊天记录至本地
void Save(Myarg *node,clientlist *c);
//读取聊天记录
void Read(Myarg *node,clientlist *c);
//文件传输
void TransferFile(Myarg *node,clientlist *c);









#endif

