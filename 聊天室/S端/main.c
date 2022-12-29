#include "Server.h"
Online_Userlist * head = NULL; //在线用户列表
struct pthreadpool *pool;//初始化线程池
extern  struct pthreadpool *Theadpool_init(int threadNum, int queueMaxNum);//初始化线程池
int main()
{
     head = OnlineUser_Create();  //创建在线用户链表
     int sockfd = k_socket();
     pool = InitPthreadPool(1,10);//一个线程对应十个任务
     k_epoll(sockfd);
     return 0;
}
