//
//  main.cpp
//  network_nat
//
//  Created by jiaojiao on 15/1/4.
//  Copyright (c) 2015年 jiaojiao. All rights reserved.
//
#include   <sys/stat.h>
#include   <sys/types.h>
#include <cstring>
#include   <sys/socket.h>
#include   <iostream>
#include <cstdio>
#include <cstdlib>
#include   <netdb.h>
#include   <fcntl.h>
#include   <unistd.h>
#include   <netinet/in.h>
#include   <arpa/inet.h>
using namespace std;
const unsigned long    RES_LENGTH=10240;//接受字符的最大长度
int     connect_socket(string server,int serverPort);
int     send_msg(int sockfd,string sendBuff);
int recv_msg(int sockfd,char *buf);
int     close_socket(int sockfd);
const string  HOSTNAME="ROBOT";
int main(int argc, char ** argv)
{
    int   sockfd=0;
    int   port = 4242;
    string ip;
    if(argc > 2)
    {
        ip=argv[1];
        port = atoi(argv[2]);
        cout<<"Input IP: "<<ip<<", port : "<< port<<endl;
    }
    else if(argc > 1)
    {
        port = atoi(argv[1]);
        printf("Input port : %d/n", port);
    }
    sockfd=connect_socket(ip, port);
    if (sockfd<0)
    {
        cout<<"server is not ready"<<endl;
        return 0;
    }
    string send_string="0#"+HOSTNAME;

    char *buf =new char[RES_LENGTH];
    int ret=recv_msg(sockfd,buf);
   /*
    switch (atoi(buf)) {
        case 10:
            cout<<"I will keep"<<endl;
            string input;
            while(true){
                cin>>input;
                if (input=="QUIT") {
                     send_msg(sockfd,"11");
                    close_socket(sockfd);
                    break;
                }
            }break;
    }
    */
    cout<<buf<<endl;
    printf("return from recv function\n");
    send_msg(sockfd,"wersfdsfdsf");
   // delete [] buf;
    close_socket(sockfd);
    return 0;
}
/************************************************************
 * 连接SOCKET服务器，如果出错返回-1，否则返回socket处理代码
 * server：服务器地址(域名或者IP),serverport：端口
 * ********************************************************/
int    connect_socket(string server,int serverPort){
    int    sockfd=0;
    struct    sockaddr_in    addr;
    struct    hostent        * phost;
    //向系统注册，通知系统建立一个通信端口
    //AF_INET表示使用IPv4协议
    //SOCK_STREAM表示使用TCP协议
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        herror("Init socket error!");
        return -1;
    }
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = inet_addr(server.c_str());//按IP初始化

    if(addr.sin_addr.s_addr == INADDR_NONE){//如果输入的是域名
        phost = (struct hostent*)gethostbyname(server.c_str());
        if(phost==NULL){
            herror("Init socket s_addr error!");
            return -1;
        }
        addr.sin_addr.s_addr =((struct in_addr*)phost->h_addr)->s_addr;
    }
    if(connect(sockfd,(struct sockaddr*)&addr, sizeof(addr))<0)
    {
        perror("Connect server fail!");
        return -1; //0表示成功，-1表示失败
    }
    else
        return sockfd;
}
/**************************************************************
 * 发送消息，如果出错返回-1，否则返回发送的字符长度
 * sockfd：socket标识，sendBuff：发送的字符串
 * *********************************************************/
int send_msg(int sockfd,string sendBuff)
{
    int sendSize=0;
    if((sendSize=send(sockfd,sendBuff.c_str(),sendBuff.length(),0))<=0){
        herror("Send msg error!");
        return -1;
    }else
        return sendSize;
}
/****************************************************************
 *接受消息，如果出错返回NULL，否则返回接受字符串的指针(动态分配，注意释放)
 *sockfd：socket标识
 * *********************************************************/
int recv_msg(int sockfd,char *response){
    int  flag=0,recLenth=0;
    memset(response,0,RES_LENGTH);


        if(( recLenth=recv(sockfd,response+flag,RES_LENGTH-flag,0))==-1 )
        {
            cout<<response<<endl;
            delete []response;
            printf("Return value : %d\n", recLenth);
            perror("Recv msg error : ");
            return -1;
        }

    return 1;
}
/**************************************************
 *关闭连接
 * **********************************************/
int close_socket(int sockfd)
{
    close(sockfd);
    return 0;
}
