#include "Server.h"
//Myarg *node 线程池指针
//处理服务器方面的套接字的流程
extern struct pthreadpool *pool;
extern Online_Userlist*head;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sqlite3 *db; //数据库句柄指针，用来表示操作某一个数据库
    int k_socket()
    {
        unsigned char ip_k[32] = "192.168.31.131";
        unsigned char port_k[32] = "8887";
        int sockfd;
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(-1 == sockfd)
        {
            perror("socket");
            return -1;
        }    
        //适当加上套接字的判断
        struct sockaddr_in severaddr,clientAddr;
        int addrlen  = sizeof(severaddr);
        severaddr .sin_family = AF_INET;//代表主机信域是IPV4地址
        severaddr.sin_addr.s_addr = inet_addr(ip_k);//ip
        //atoi:将字符串转为正数
        //hton1：将主机序转为网络序
        severaddr.sin_port = htons(atoi(port_k));//填写服务器端口号
        if(bind(sockfd,(struct  sockaddr*)&severaddr,addrlen) == -1)
        {
            perror("bind");
            return -1;
        }
    
    //将套接字设置为被动监听状态
    //int k_listen(int sockfd,int t)
        if(listen(sockfd,5) == -1)
        {
            perror("listen");
            return -1;
        }
        printf("客户机已经开启,等待用户连接中.....\n");
        return sockfd;
    }
   


//epoll
//作用：epoll是一种IO多路转接技术，在LINUX网络编程中，经常用来做事件触发，即当有特定事件到来时，能够检测到，而不必阻塞进行监听。
//epoll有两种工作方式，ET-水平触发 和 LT-边缘触发（默认工作方式），主要的区别是：
//LT，内核通知你fd是否就绪，如果没有处理，则会持续通知。而ET，内核只通知一次。



//epoll操作
//创建epoll对象
int k_epoll(int sockfd)
{
    struct sockaddr_in clientAddr;
    socklen_t addrlen  = sizeof(clientAddr);//计算网络信息结构体的大小
    int L = sizeof(clientlist);//Msg为通信协议，计算其大小


    //创建epoll对象
    int epollfd = epoll_create(2000);//创建代表epoll实例的文件描述符,最大保存文件描述符个数为2000
    if(-1 == epollfd)
    {
         ERRLOG("epoll_create");
    }
    printf("pollfd = %d\n",epollfd);
    //创建结构体
    struct epoll_event ev, events[2000] = {0};//事件初始化，读 写 异常等
    ev.events = EPOLLIN; //监听数据可读
    ev.data.fd = sockfd;//用户自定义数据 

    //操作epoll内核时间表
    // 参数一create生成的专用文件描述符  二操作类型 增 删 改  三 关联的文件描述符 四epoll_event结构体也就是上面的结构体
    int ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&ev);
    if(-1 == ret)
    {
        ERRLOG("epoll_ctl");
    }
    
    ssize_t bytes;//定义接收用文件描述符
    char buf[N] = {0};
 
 //阻塞等待客户端连接
 int i ;
 int acceptfd = 0;//定义阻塞的客户端的文件描述符
 while(1)
 {
    clientlist agreement;
    //准备好的文件描述符的个数
    printf("epoll-wait等待事件到来!\n");
    int num = epoll_wait( epollfd, events, 2000,-1);
    if(num == -1)
    {
        printf("epoll_wait() failed");
        return -1;     
    }

    
    printf("开始处理事件！\n");
    for(int i = 0;i < num; i++)
    {
        if(events[i].data.fd == sockfd)//代表有客户端连接了
        {
            //阻塞客户端的连接
            if((acceptfd = accept(sockfd,(struct sockaddr *)&clientAddr,&addrlen)) == -1)
            {
                ERRLOG("accept");
                continue;
            }
            printf("acceptfd = %d\n", acceptfd);
            ev.events = EPOLLIN;
            ev.data.fd = acceptfd;
            ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,acceptfd,&ev);//注册事件：acceptfd
            if(-1 == ret)
            {
                ERRLOG("epoll_ctl");
            }
            //打印客户段信息
            printf("客户端%s:%d连接了\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
        }
            //如果有客户端发消息
            else
            {
                Myarg *arg = (Myarg *)malloc(sizeof(Myarg));
                memset(arg,0,sizeof(Myarg));
                if(events[i].events & EPOLLIN)//如果事件可读
                {
                    memset(&agreement,0,sizeof(agreement));
                    bytes = recv(events[i].data.fd,&agreement,L,0);
                    printf("flag = %d\n",agreement.flag);
                    if(-1 == bytes)
                    {
                        perror("recv");
                        return -1;
                    }
                    else if(0 == bytes)//发送方关闭文件描述符或退出
                    {
                        printf("客户端%s:%d退出了\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
                        ev.events = EPOLLIN;
                        ev.data.fd = events[i].data.fd;//events[i].data.fd相当于中间变量
                        ret = epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);//删除事件：events[i].data.fd 
                        if(-1 == ret)
                        {
                            ERRLOG("epoll_ctl");
                        }
                        close(events[i].data.fd);
                    }          
                    else
                    {
                        // //信息拷贝
                            arg->msg.flag=agreement.flag;
                            printf("flag = %d\n",agreement.flag);
                            arg->msg.n=agreement.n;
                            strcpy(arg->msg.nameid, agreement.nameid);
                            printf("agreement.nameid = %s\n",agreement.nameid);
                            strcpy(arg->msg.to_name, agreement.to_name);
                            printf("argeement.to_name = %s\n",agreement.to_name);
                            strcpy(arg->msg.msg, agreement.msg);
                            printf("agreement.msg = %s\n",agreement.msg);
                            strcpy(arg->msg.password, agreement.password);
                            printf("agreement.password = %s\n",agreement.password);
                            strcpy(arg->msg.text, agreement.text);
                            printf("argeement.text = %s\n",agreement.text);
                            strcpy(arg->msg.filename, agreement.filename);
                            printf("argeement.filename = %s\n",agreement.filename);
                            arg->fd = events[i].data.fd;
                            ThreadAddJob(pool, work, (void *)arg);//任務添加到任務隊列
                    }        
                    
                }
            }
            
    }
  }


}


        
//线程函数从任务队列取任务
void *ThreadRun(void *arg)  
{
    struct pthreadpool *pool = (struct pthreadpool *)arg;
    struct job *pjob = NULL;//任务队列指针
    while (1)
    {
        pthread_mutex_lock(&pool->mutex);
        if (pool->m_QueueCurNum == 0)
        {
            printf("当前任务队列为空，线程%ld阻塞等待任务到来!\n", pthread_self());
            pthread_cond_wait(&pool->m_QueueNotEmpty, &pool->mutex);
        }
        pjob = pool->head;//指针指向任务队列的头部
        pool->m_QueueCurNum--;//当前任务数量--
        if (pool->m_QueueCurNum != pool->m_QueueMaxNum)//如果当前任务队列没有满 
        {
            pthread_cond_signal(&pool->m_QueueNotFull); //每次线程取出一个任务之后，都要唤醒线程去添加任务
        }
        if (pool->m_QueueCurNum == 0)//如果当前任务对列没有任务
        {
            pool->head = pool->tail = NULL;
            pthread_cond_broadcast(&pool->m_QueueEmpty); //当通知任务队列添加任务无果后，发送条件变量，通知其销毁线程
        }
        else
        {
            pool->head = pool->head->next;
        }
        pthread_mutex_unlock(&pool->mutex);
        pjob->func(pjob->arg);//运行任务函数
        free(pjob);
        pjob = NULL;
    }
}
 
//参数分析  参数1线程池线程数量   参数二   任务队列最大任务数
struct pthreadpool *InitPthreadPool(int ThreadNum, int QueueMaxNum) //初始化線程池  創建線程池
{
    struct pthreadpool *pool = (struct pthreadpool *)malloc(sizeof(struct pthreadpool)); //创建一个线程池初始化的结构体
    pool->m_QueueCurNum = 0;//当前任务队列的任务数
    pool->m_QueueMaxNum = QueueMaxNum;//任务队列的最大最大任务数
    pool->head = NULL;
    pool->tail = NULL;
    pool->m_threadNum = ThreadNum;//已经开启的线程数量也就是创建的线程数量
 
    pool->m_pthreadIDs = (pthread_t *)malloc(sizeof(pthread_t) * ThreadNum);//保存已开启的线程IO
        //动态数组名  保证后续指针的移动合法
    pthread_mutex_init(&pool->mutex, NULL);//初始化一把锁
    pthread_cond_init(&pool->m_QueueEmpty, NULL);//任务队列为空条件
    pthread_cond_init(&pool->m_QueueNotEmpty, NULL);//任务队列不为空的条件
    pthread_cond_init(&pool->m_QueueNotFull, NULL);//任务队列不满的条件
    for (int i = 0; i < ThreadNum; i++)
    {
        pthread_create(&pool->m_pthreadIDs[i], NULL, ThreadRun, pool);
    }
    return pool;
}
 
//將任務函數添加到任務隊列
void ThreadAddJob(struct pthreadpool *pool, void *(*func)(void *arg), void *arg)//
{
    pthread_mutex_lock(&pool->mutex);
    if (pool->m_QueueCurNum == pool->m_QueueMaxNum)
    {
        printf("任务队列已满，挂起等待线程执行完毕。。。\n");
        pthread_cond_wait(&pool->m_QueueNotFull, &pool->mutex);
    }
    struct job *pjob = (struct job *)malloc(sizeof(struct job)); //创建任务队列
    pjob->func = func;//任务函数通过任务传参传入任务函数
    pjob->arg = arg;//任务函数的参数
 
    // pjob->func(pjob->arg);
    pjob->next = NULL;
    if (pool->head == NULL)//如果任务队列 里面没有任务
    {
        pool->head = pool->tail = pjob;//那么将任务函数放置头结点处
        pthread_cond_broadcast(&pool->m_QueueNotEmpty); //添加任务后，唤醒任意一个线程开始执行任务
    }
    else
    {
        pool->tail->next = pjob;//如果任务队列有任务  就将新的任务函数放在任务队的末尾
        pool->tail = pjob;//尾指针指向最后一盒任务函数
    }
    pool->m_QueueCurNum++;//当前任务数量++
    pthread_mutex_unlock(&pool->mutex);
}
 
//銷毀線程池
void ThreadDestroy(struct pthreadpool *pool)
{
    pthread_mutex_lock(&pool->mutex);
    while (pool->m_QueueCurNum != 0)
    {
        printf("阻塞等待销毁线程。。。\n");
        pthread_cond_wait(&pool->m_QueueEmpty, &pool->mutex);
    }
    printf("任务结束，线程%ld被销毁\n", pthread_self());
    pthread_mutex_unlock(&pool->mutex);
    pthread_cond_broadcast(&pool->m_QueueNotEmpty);
    pthread_cond_broadcast(&pool->m_QueueNotFull);
    int i;
    for (i = 0; i < pool->m_threadNum; i++)
    {
        pthread_join(pool->m_pthreadIDs[i], NULL);
    }
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->m_QueueEmpty);
    pthread_cond_destroy(&pool->m_QueueNotEmpty);
    pthread_cond_destroy(&pool->m_QueueNotFull);
    free(pool->m_pthreadIDs);
    struct job *tmp;
    while (pool->head != NULL)
    {
        tmp = pool->head;
        pool->head = pool->head->next;
        free(tmp);
    }
    free(pool);
}


//工作函数

void *work(void *arg)
{
   
    Myarg *work_arg = (Myarg *)arg;
    printf("work: %d\n", work_arg->fd);
    printf("msg: %s\n",work_arg->msg.msg);
    printf("msg: %d\n",work_arg->msg.flag);
 

    switch(work_arg->msg.flag)
    {
        case REGISTER://注册
        {
            Register_User(work_arg,&work_arg->msg);
            break;

        }
        case LOGIN://登录
        {
            Login(work_arg,&work_arg->msg);
            break;

        
        }        
        case PRIVATE://私聊
        {
            PrivateChat(work_arg,&work_arg->msg);
            break;
        }
        case VIEWUSER://查看在线用户
        {
             ViewUser(work_arg,&work_arg->msg);
             break;
        }
        case GROUPCHAT://群发
        {
            Fsend(work_arg,&work_arg->msg);
            break;
        }
        case READ: //读取
        {
            Read(work_arg,&work_arg->msg);
            break;
        }
        case TRANSFERFILE: //文件传输
        {
            TransferFile(work_arg,&work_arg->msg);
            break;
        }
        default:
        {
            break;
        }
    }
    free(arg);
}



//在线用户链表
Online_Userlist *OnlineUser_Create()
{
    Online_Userlist * head = (Online_Userlist *)malloc(sizeof(Online_Userlist ));
    head->next = NULL;
    return head;
}


//注册用户
//用户的信息
//向数据库插入注册信息
void Insert_data(Myarg *node , clientlist *c)
{
    printf("注册的账号:%s\n",c->nameid);
    char sql[128] = {0};
    sqlite3 *db =NULL;
    int ret = sqlite3_open("user.db",&db);
    if(SQLITE_OK != ret)
    {
        ERRLOG("sqlite3_open");
    }
    sprintf(sql,"create table if not exists user(nameid char,password char);");
    ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    {
        printf("sqlite3_exec error :%s\n",sqlite3_errmsg(db));
    }
    sprintf(sql,"insert into user(nameid,password) values('%s','%s');",c->nameid,c->password);
    char *errmsg = NULL;
    printf("数据插入成功！\n");
    char buf[100] = {"注册成功...\n"};
    if(send(node->fd,buf,sizeof(buf),0) == -1)
    {
        ERRLOG("send error");
    }
    ret = sqlite3_close(db);
    if(SQLITE_OK != ret)
    {
        perror("sqlite_close:");
        exit(1);
    }
}

//注册操作
//需要实现插入数据和查找数据
void Register_User(Myarg *node,clientlist *c)
{
    // 判断是否注册
    //遍历数据库
    printf("判断用户%s是否已注册\n",c->nameid);
    int ret = Searchtable(c->nameid);
    if(ret == 1)
    {
        char buf[N] = {"账号已注册，请重新输入选择"};
        if(-1 == send(node->fd,buf,N, 0))
        {
            ERRLOG("send");
        }
        return; 
    }
    else
    {
       Insert_data(node,c);
    }
     
}



//数据库查找  存在返回1，不存在返回0
int Searchtable(char *tablename)
{
    char sql[N] = "\0";
    int ret = sqlite3_open("user.db",&db);
    if(ret != SQLITE_OK)
    {
        printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
    }
    //printf("将要查找%s\n",c->nameid);
    memset(sql,0,sizeof(sql));
    sprintf(sql,"select *from user;");
    int flag;//1for存在,0for不存在
    ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    {
        printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
    }
    char *errmsg;//指向错误信息
    char **result;//保存搜索到的内容
    int row;//行
    int column;//列
    ret = sqlite3_get_table(db,sql,&result,&row,&column,NULL);
    if(ret != SQLITE_OK)
    {
        printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
    }
    int i;
    //int index = column;
    for(i = 0;i <row;i++)
    {
        int ret = strcmp(result[i*column],tablename);
        {
           if(ret == 0)
           {
              flag = 1;
           }
           if(flag == 1)
           {
             printf("账号已注册！\n");
             sqlite3_free_table(result);//释放数据库
             ret = sqlite3_close(db);
             if(ret != SQLITE_OK)
             {
                printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
             }
             return flag;
           } 
           else
           {
             flag = 0;
             //printf("账号未注册！\n");
             sqlite3_free_table(result);//释放数据库
             ret = sqlite3_close(db);
             if(ret != SQLITE_OK)
             {
                printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
             }
             return flag;
           }
            
        }
    }
}



void Login(Myarg *node,clientlist *c)//登录
{
    //判断是否注册
    printf("判断用户%s是否已注册\n",c->nameid);
    int ret = Searchtable(c->nameid);
    if(ret != 0)
    {
        char buf[128] = "账号不存在，请重新注册!";
        if(send(node->fd,buf,sizeof(buf),0) == -1)
        {
            ERRLOG("send error");
        }
    }
    /*else//验证账号密码是否错误
    {
        ret = Verify(c);
        if(ret != 0)
        {
            char buf[128] = "账号密码错误，请重新输入!";
            if(send(node->fd,buf,sizeof(buf),0) == -1)
            {
                ERRLOG("send error");
            }
        }
    }*/
    char buf[128] = "账号密码验证通过，正在登录中...";
    if(send(node->fd,buf,sizeof(buf),0) == -1)
    {
        ERRLOG("send error");
    }
    c->flag = 1;
    //标志位代表是否为登录状态
    char buf1[128] = "登录成功!";
    if(send(node->fd,buf1,sizeof(buf1),0) == -1)
    {
        ERRLOG("send error");
    }
    Online_Insert(node,c);
    Online_Userlist *p = head;//插入到链表中
    p = p->next;
    while(p != NULL)
    {
        if(p->o_fd != 0)//代表有人上线
        {   
            char buf[N];
            sprintf(buf,"%s上线了\n",c->nameid);
            if(-1 == send(p->o_fd,buf,N,0))
            {
                ERRLOG("send");
            }
        }
        p = p->next;
    }
 
}

void Online_Insert(Myarg *node,clientlist *c)//添加在线人数
{
    Online_Userlist * temp = (Online_Userlist *)malloc(sizeof(Online_Userlist ));
    temp->next = NULL;
    Online_Userlist *p = head;//定义表头
    //将登录的用户和fd复制到链表中
    strcpy(temp->name,c->nameid);
    temp->o_fd = node->fd;
    temp->next = p->next;
    p->next = temp;
    printf("插入链表成功！\n");
}   


int Verify(clientlist *c)//核实密码是否错误 1for正确  0for错误
//遍历数据库对比其中的密码一行
{
        printf("核验密码中...\n");
        char sql[N] = "\0";
        int flag;//1for 通过,0for不通过
        int ret = sqlite3_open("user.db",&db);
        if(ret != SQLITE_OK)
        {
            printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
            exit(1);
        }
        printf("将要查找%s\n",c->password);
        memset(sql,0,sizeof(sql));
        sprintf(sql,"select *from user;");
        printf("遍历数据库中...\n");
        ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
        if(ret != SQLITE_OK)
        {
            printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
        }
        printf("遍历数据库中...\n");
        char *errmsg;//指向错误信息
        char **result;//保存搜索到的内容
        int row;//行
        int column;//列
        ret = sqlite3_get_table(db,sql,&result,&row,&column,&errmsg);
        if(ret != SQLITE_OK)
        {
            printf("sqlite3_get_table: %s\n",sqlite3_errmsg(db));
        }
        int ret1 = 0;
        int ret2 = 0;
        printf("遍历数据库中...\n");
        for(int i = 0;i < row + 1;i++)
        {
            printf("正在校验...\n");
            ret1 = (strcmp(result[i*column + 0],c->nameid));
            printf("ret1 = %d\n",ret1);
            ret2 = (strcmp(result[i*column + 1],c->password));
            printf("ret2 = %d\n",ret2);
            printf("账号%s\n",result[i*column+0]);
            printf("密码%s\n",result[i*column+1]);
            printf("密码%s\n",c->password);
            if(ret1 == 0 && ret2 == 0)
            {
                flag = 1;
                break;
            }
        }
        if(flag == 1)
        {
        
            printf("账号已注册！\n");
            sqlite3_free_table(result);//释放数据库
            ret = sqlite3_close(db);
            if(ret != SQLITE_OK)
            {
                printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
            }
            return flag;
        } 
        else
        {
            flag = 0;
            sqlite3_free_table(result);//释放数据库
            ret = sqlite3_close(db);
            if(ret != SQLITE_OK)
            {
               printf("sqlite3_open: %s\n",sqlite3_errmsg(db));
            }
            return flag;
        }

}  



void PrivateChat(Myarg *node,clientlist *c)//私聊功能,先结收消息再转发给客户端指定的客户机
{
    //进行通信
    int flag ;
    printf("进行私聊中...\n");
    char buf[N] = {0};//存放发来的消息
    int to_cfd = Find_person(c);//在线返回对方的文件描述符
    printf("%d = to_cfd\n",to_cfd);
    if(0 == to_cfd)
    {
        char buf[N] = "对方不在线";
        if(-1 == send(node->fd,buf,N,0))
        {
            ERRLOG("send");
        }
    }
    else
    {
        if(0 == flag)
        {
            printf("正在处理...\n");
            sprintf(buf,"正在与%s私聊",c->to_name);
            //将接收的消息转发给客户机要发送的对象
            if(-1 == send(node->fd,buf,N,0))
            {
                ERRLOG("send");
            }
            flag++;
        }
        printf("正在进行消息:%s转发\n",c->msg);
        char buf2[M] = "\0";
        sprintf(buf2,"%s说:%s",c->nameid,c->msg);
        if(-1 == send(to_cfd,buf2,M,0))
        {
            ERRLOG("send");
        }
        Save(node,c);
    }
}


int Find_person(clientlist *c)//判断是否在线
{
    
    char s1[32] = {0};
    strcpy(s1,c->to_name);
    printf("判断%s是否在线\n",c->to_name);
    Online_Userlist *p =head;
    while(p != NULL)
    {
        if(strcmp(s1,p->name) == 0)
        {
            printf("%s在线\n",p->name);
            //p->o_fd++;
            return p->o_fd;
            
        }
        p = p->next;
    }
    return 0;
}


void ViewUser(Myarg *node,clientlist *c)//查看在线人数
{
    //判断链表是否为空
    Online_Userlist *p =head->next;
    while(p == NULL)
    {
        char buf[N] = "当前无用户在线";
        if(-1 == send(node->fd,buf,N,0))
        {
            ERRLOG("send");
        }
        exit(1);
    }
    while(p != NULL)
    {
        if(p->o_fd != 0)
        {
            char buf1[N];
            memset(buf1,0,N);
            printf("%s在线\n",p->name);
            sprintf(buf1,"%s当前在线",p->name);
            if(-1 == send(node->fd,buf1,N,0))
            {
                ERRLOG("send");
            }
            sleep(1);
        }
        p = p->next;
    }
    
}


void Fsend(Myarg *node,clientlist *c)//群发消息
{
    //进行通信
    //int flag = 1;
        //判断链表是否为空
        Online_Userlist *p =head->next;
        while(p == NULL)
        {        
            char buf[N] = "当前无用户在线";
            if(-1 == send(node->fd,buf,N,0))
            {
                ERRLOG("send");
            }
            break;
        }
        while(p != NULL)
        {
            printf("正在进行群发...\n");
            if(p->o_fd != 0)
            {
                if(strcmp(p->name,c->nameid) != 0)
                {
                    char buf1[M];
                    memset(buf1,0,M);

                    sprintf(buf1 ,"%s说%s",c->nameid,c->msg);
                    if(-1 == send(p->o_fd,buf1,M,0))
                    {
                        ERRLOG("send");
                    }
                    memset(buf1,0,sizeof(buf1));

                }
            }
            Save(node,c);
            p = p->next;
        }
    
}

void Save(Myarg *node,clientlist *c)//保存聊天记录至本地
{
    FILE *fp = fopen("./1.txt","a");
    if( NULL == fp)
    {
        ERRLOG("fopen");
    }
    int ret = fwrite(c->msg,sizeof(c->msg),1,fp);
    if(-1 == ret)
    {
        ERRLOG("fwrite");
    }
    fclose(fp);
    char buf[N] = "保存聊天记录中...";
    if(-1 == send(node->fd,buf,N,0))
    {
        ERRLOG("send");
    }
}

void Read(Myarg *node,clientlist *c)//读取聊天记录
{
    FILE *fp = fopen("./1.txt","r");
    if( NULL == fp)
    {
        ERRLOG("fopen");
    }
    while(0 == feof(fp))
    {
        clientlist *tmp = (clientlist *)malloc(sizeof(clientlist));
        if(0 == fread(tmp ,sizeof(c->msg),1,fp))
        {
            if(feof(fp))
            {
                char buf[N] = "读取文件中...";
                if(-1 == send(node->fd,buf,N,0))
                {
                    ERRLOG("send");
                }
                break;
            }
            else
            {
                ERRLOG("fread");
            }

                
            
        }
    }
}


//文件传输
//服务器检索文件并发送给客户端
//客户端需要recv
void TransferFile(Myarg *node,clientlist *c)
{
    char buf[N] = {0};
    printf("查找文件中...\n");
    printf("user.filename = %s\n",user.filename);
    
    /*char buf[N] = "正在查找要下载的文件...";
    if( -1 == send(node->fd,buf,N,0))
    {
        ERRLOG("send");
    }*/
    //memset(buf,0,N);
    int acceptfd;
    ssize_t bytes;
    /*if((bytes = recv(acceptfd,buf,N,0)) == -1)
    {
        ERRLOG("recv");
    }*/
    int fd;//文件描述符
    int num;
    while(1)
    {
        NEXT:
        //判断文件是否存在
        printf("2查找文件中...\n");
        if((fd = open(user.filename,O_RDONLY)) == -1)
        {
            printf("3查找文件中...\n");
            if(errno == ENOENT)
            {
            printf("文件不存在...\n");
            //文件不存在
            if(send(node->fd,"**NOEXIST**",N,0) == -1)
            {
                sleep(1);
                printf("文件不存在...\n");
                ERRLOG("send");
            }            
            
            }        //goto NEXT;            
            else
            {
                printf("文件查找成功...\n");
                ERRLOG("open");        
            }

        }
        //读取内容并发送给客户端
        while((num = read(fd,buf,N)) != 0)
        {
            user.n = num;
            strcpy(user.text,buf);
            if(send(node->fd,&user,sizeof(clientlist),0) == -1)
            {
                ERRLOG("send");
            }
        }
        //发送结束标志
        user.n = 0;
        strcpy(user.text,"**OVER**");
        if(send(node->fd,&user,sizeof(clientlist),0) == -1)
        {
            ERRLOG("send");
        }
        printf("文件传输完毕!\n");
    }
}