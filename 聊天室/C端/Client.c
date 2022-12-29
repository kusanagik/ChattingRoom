#include "Client.h"
    unsigned char ip_k[32] = "192.168.31.131";
    unsigned char port_k[32] = "8887";
    int flag1 = 0;
    int flag2 = 1;


void C_socket()
{
    //第一步创建套接字
    //int sockfd;
    if(-1 == (sockfd = socket(AF_INET,SOCK_STREAM,0)))
    {
        ERRLOG("socket");
    }
    struct sockaddr_in severaddr;
    severaddr.sin_family = AF_INET;//协议族  IPV4
    severaddr.sin_addr.s_addr = inet_addr(ip_k); 
    /*in_addr_t inet_addr(const char *cp);将点分十进制字符串ip地址转化为网络序的无符号4字节
    整数ip地址*/
    int addrlen = sizeof(severaddr);
    severaddr.sin_port = htons(atoi(port_k));
    if(connect(sockfd,(struct sockaddr *)&severaddr,addrlen)==-1)
    {
        ERRLOG("connect");
    }
    printf("sockfd = %d\n",sockfd);
    printf("客户端连接中...\n");
}


void Menu()//菜单
{
    printf("****************欢迎来到传火祭祀场***************\n");
    printf("*****************请输入你的选择******************\n");
    printf("*******************1:注册账号*******************\n");
    printf("*******************2:登录账号*******************\n");
    printf("*******************3.退出频道*******************\n");
    
    int num;
    printf("请输入你的选择:\n");
    scanf("%d",&num);
    getchar();
    switch(num)
    {
        case 1:
        {
            putchar(10);
            Register();
            Menu();
            break;
        }
        case 2:
        {
            putchar(10);
            Login();
            break;
        }
        case 3:
        {
            putchar(10);
            printf("退出程序中...\n");
            sleep(1);
            exit(0);
        }
        default:
        {
            printf("输入有误！请重新输入\n");
            Menu();
        }
    }
}

void Register()//注册
{
    ssize_t ret;
    printf("请输入你要注册的账号名称:\n");
    scanf("%s",user.nameid);
    char password[32] = {0};
    printf("请输入你的密码:\n");
    scanf("%s",password);
    printf("请再次输入你的密码:\n");
    scanf("%s",user.password);
    printf("sockfd = %d\n",sockfd);

    if(strcmp(password,user.password) == 0)
    {
        printf("密码验证通过\n");
        user.flag = 1;//用户标志位注册
        printf("测试成功\n");
        printf("flag = %d\n",user.flag);
        ret = send(sockfd,&user,sizeof(clientlist),0);
        if(-1 == ret) 
        {
            ERRLOG("send");
        }
        return;
    }
    else
    {
        printf("账号或密码输入错误，请重新输入!\n");
        sleep(1);
        Register(user);
    }
}



void Login()//登录
{
    printf("请输入要登录的账号:\n");
    scanf("%s",user.nameid);
    printf("请输入账号密码:\n");
    scanf("%s",user.password);
    ssize_t ret;
    user.flag = 2;//用户标志登录
    ret = send(sockfd,&user,sizeof(clientlist),0);
    if(-1 == ret)
    {
        ERRLOG("send");
    }
    if(1 == flag1)
    {
        flag1 = 0;
        Menu();   
    }
    if(0 == flag1)
    {
        SelectFunc2();
    }
    return;
}


void SelectFunc2()//第二阶段菜单
{
    printf("******************欢迎进入功能菜单***************\n");
    printf("*******************1 for  私聊******************\n");
    printf("*******************2 for  群发******************\n");
    printf("*******************3 for  查看在线人数***********\n");
    printf("*******************4 for  退出******************\n");
    printf("*******************5 for  文件发送******************\n");
    //printf("*******************5 for  查看聊天记录******************\n");
    printf("******************请输入你的选择******************\n");
    int num;
    scanf("%d",&num);
    switch(num)
    {
        case 1:
        {
            printf("启用私聊功能...\n");
            PrivateChat();
            break;
        }
        case 2:
        {
            printf("启用群发功能...\n");
            GroupChat();
            break;
        }
        case 3:
        {
            printf("启用查看在线用户功能...\n");
            ViewOnline_user();
            break;
        }
        case 4:
        {
            printf("即将退出...\n");
            printf("返回主界面中....\n");
            Menu();
            break;
        }
        case 5:
        {
            printf("启用发送文件功能...\n");
            TransferFiles();
            break;
        }
    }
    

}



void PrivateChat()//私聊
{
    ssize_t ret;
    char to_name[32];
    printf("输入你要私聊人的名字:\n");
    scanf("%s",user.to_name);
    user.flag = 3;//私聊
    char buf[200] = "\0";
    while(1)
    {
        printf("请输入私聊内容:\n");
        scanf("%s",buf);
        if(strcmp(buf,"quit") == 0)
        {
            printf("客户端退出私聊\n");
            SelectFunc2();
        }
        strcpy(user.msg,buf);
        ret = send(sockfd,&user,sizeof(clientlist),0);
        if(ret == -1)
        {
            ERRLOG("send");
        }

    }
    return;
}

void GroupChat()//群聊
{
    ssize_t ret;
    user.flag = 5;//私聊
    char buf[200] = "\0";
    while(1)
    {
        printf("请输入群发内容:\n");
        scanf("%s",buf);
        if(strcmp(buf,"quit") == 0)
        {
            printf("客户端退出\n");
            SelectFunc2();
        }
        strcpy(user.msg,buf);
        ret = send(sockfd,&user,sizeof(clientlist),0);
        if(ret == -1)
        {
            ERRLOG("send");
        }

    }
    return;
}
void ViewOnline_user()//查看所有在线用户
{
    user.flag = 4; //功能标志
    int ret;
    ret =send(sockfd,&user,sizeof(clientlist),0);
    if(ret == -1)
    {
        ERRLOG("send");
    }
    return;
}


void Read()//功能标志6 查看聊天记录 
{
        user.flag = 6; 
        int ret;
        ret =send(sockfd,&user,sizeof(clientlist),0);
        if(ret == -1)
        {
            ERRLOG("send");
        }
        return;
}
 
void TransferFiles()//下载文件
{
    user.flag = 7;
    char buf[N] = {0};
    printf("请输入要下载的文件名称：\n");
    scanf("%s",buf);
    strcpy(user.filename,buf);
    int ret;
    if((ret = send(sockfd,&user,sizeof(clientlist),0)) == -1)
    {
        ERRLOG("send");
    }
    return;
}




void *recv_thread(void *arg)//接收线程
{
        // int ret = 1;
    int fd;
    //int num;
    char buf[256] = {0};
    int length;
    sockfd = *((int *)arg);
    while(1)
    {
        memset(buf,0,sizeof(buf));
        length = recv(sockfd,buf,sizeof(buf),0);
        if(length == 0)
        {
            printf("服务器宕机!!\n");
            exit(1);
            pthread_exit(NULL);
            
        }
        buf[length] = '\0';
        printf("\033[5;;36m%s\033[0m\n",buf);
        if(strcmp(buf,"该用户不存在,请注册...") == 0)
        {
            flag1 = 1;
        }
        /*if(strcmp(buf,"对方不在线...") == 0)
        {
            flag3 = 0;
        }*/
        if(strcmp(buf,"密码输入错误..") == 0)
        {
            flag1 = 1;
        }
        if(strcmp(buf,"您已在线,请勿重复登录") == 0)
        {
            flag1 = 1;
        }
    }
}



void *send_thread(void *arg)//发送线程
{
    Menu();
}
    /*开始通信
    char buf[N] = {0};
    while(1)
    {
        fgets(buf,N,stdin);
        buf[strlen(buf) - 1] = '\0';
        if(send(sockfd,buf,N,0) == -1)
        {
            perror("send");
            return -1;
        }
        if(strcmp(buf,"quit") == 0)
        {
            printf("客户端退出了!\n");
            exit(0);
        }
        memset(buf,0,N);
        if(recv(sockfd,buf,N,0) == -1)
        {
            perror("send");
            return -1;
        }
        printf("服务器:%s\n",buf);
    }*/



