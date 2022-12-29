#include "Client.h"

int main(int argc, char const *argv[])
{
    C_socket();
    //Menu();
    //SelectFunc2();
    int ret;
    pthread_t tid_recv;
    pthread_t tid_send;

    ret = pthread_create(&tid_send,NULL,(void *)send_thread,(void *)&sockfd);
    ret = pthread_create(&tid_recv,NULL,(void *)recv_thread,(void *)&sockfd);
   
    pthread_join(tid_send,NULL);
    pthread_join(tid_recv,NULL);
    
    close(sockfd);
    return 0;
}
