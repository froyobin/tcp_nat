//
//  main.cpp
//  network_nat
//
//  Created by jiaojiao on 15/1/4.
//  Copyright (c) 2015年 jiaojiao. All rights reserved.
//
#include   <sys/stat.h>
#include <sys/socket.h>
#include   <sys/types.h>
#include <cstring>
#include   <sys/socket.h>
#include   <iostream>
#include <cstdio>
#include <cstdlib>
#include   <netdb.h>
#include <arpa/inet.h>
#include   <fcntl.h>
#include   <unistd.h>
#include   <netinet/in.h>
#include <signal.h>
#include   <arpa/inet.h>
using namespace std;
const unsigned long    RES_LENGTH=10240;//接受字符的最大长度
int     connect_socket(string server,int serverPort);
int     send_msg(int sockfd,string sendBuff);
int recv_msg(int sockfd,char *buf);
void localserver_start(struct sockaddr_in guest);
int     close_socket(int sockfd);
int create_server(struct sockaddr_in guest);
const string  HOSTNAME="ROBOT";
const char SERVERALL[]="0.0.0.0";
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
    
    
    
    struct sockaddr_in  guest;
    // socklen_t serv_len = sizeof(serv);
    socklen_t guest_len = sizeof(guest);
    
    getsockname(sockfd, (struct sockaddr *)&guest, &guest_len);
    //   getpeername(sockfd, (struct sockaddr *)&serv, &serv_len);
    
      //cout<<inet_ntoa(localsock.sin_addr)<<endl;
    
    string send_string="0#"+HOSTNAME;
    send_msg(sockfd,send_string);
    char *buf =new char[RES_LENGTH];
    int ret=recv_msg(sockfd,buf);
    if(atoi(buf)==10) {
        cout<<"I will keep"<<endl;
        pid_t localpid=fork();
        if (localpid==0) {
            localserver_start(guest);
        }else{
            
            wait(&localpid);
            cout<<"we exit"<<endl;
            send_msg(sockfd,"11");
            close_socket(sockfd);
        }
        
    }
    
    
    printf("return from recv function\n");
    delete [] buf;
    close_socket(sockfd);
    return 0;
}



void localserver_start(struct sockaddr_in guest)
{
    int fd[2];
    string input;
    pid_t child;
    if (pipe(fd)) {
        cout<<"create pipe failed!\n"<<endl;
        exit(-1);
    }
    
    child=fork();
    if (child==0) {
        
        while(true){
            cin>>input;
            if (input=="QUIT") {
                break;
            }
            
        }
        close(fd[0]);
        write(fd[1],"11",strlen("11"));
        exit(1);
    }else{
        
        int serversockfd=create_server(guest);
        if (serversockfd==-1) {
            kill(child, SIGKILL);
            exit(-1);
        }
        
        pid_t child2 = fork();
        if (child2==0) {
            struct sockaddr_in client;
            socklen_t client_t = sizeof(struct sockaddr_in);
            printf("Waiting new connection!\n");
            while (1) {
                
                
                int clientfd = accept(serversockfd, (struct sockaddr *)&client, &client_t);
              
                char buff[1024];
                int ret=send_msg(clientfd, "aaabbbb");
                if (ret<1) {
                    cout<<"erroraaaa"<<endl;
                    exit(-1);
                }
                
                cout<<"we are about to receive"<<endl;
                int recLenth=recv(clientfd,buff,RES_LENGTH,0);
                
                
        //        int ret2=recv_msg(clientfd, buff);
                cout<<buff<<endl;
            }
            exit(1);
            
        }else{
            close(fd[1]);
            char buf[20];
            bool flag=true;
            while (flag==true) {
                read(fd[0], buf, 20);
                if (atoi(buf)==11) {
                    //we should close the server and exit all!!!!
                    kill(child2,SIGKILL);
                    close_socket(serversockfd);
                    flag=false;
                }
            }
            
        }
        
        cout<<"we are going to stop..."<<endl;
        wait(&child);
        
        exit(1);
        
    }
    
    
}

void die(string line)
{
    cout<<line<<endl;
}

int create_server(struct sockaddr_in guest)
{
    int yes=-1;
    struct addrinfo *ai;
    
    
    int serversockfd=socket(AF_INET,SOCK_STREAM,0);
    if (serversockfd==-1)
    {
        die("socket can not be created!!");
        return -1;
    }
    
    if (setsockopt(serversockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&yes, sizeof(int)) == -1) {
        die("Couldn't setsockopt");
        return -1;
    }
    char portnumber[20];
    sprintf(portnumber, "%d",ntohs(guest.sin_port));
    /* Fill the address info struct (host + port) -- getaddrinfo(3) */
    if (getaddrinfo(SERVERALL,portnumber, NULL, &ai) != 0) {   // get localhost
            cout<<"port number"<<"######"<<portnumber<<endl;
        die("Couldn't get address");
        return -1;
    }
    
    if(bindresvport_sa(serversockfd, ai->ai_addr)!=0) //mac os use this bind   fixme
    {
        
        die("Couldn't bind socket to address");
        return -1;
    }
    //    if (bind(serversockfd, ai->ai_addr, ai->ai_addrlen) != 0) {  // only bind on localhost ip
    //        die("Couldn't bind socket to address");
    //        return -1;
    //    }
    
    /* Free the memory used by our address info struct */
    /* Mark this socket as able to accept incoming connections */
    /* printf("Process %d Listening/n", getpid()); */
    if (listen(serversockfd, 1) == -1) {
        die("Couldn't make socket listen");
        return -1;
    }
    cout<<"creat server at"<<portnumber<<"successfully"<<endl;
    freeaddrinfo(ai);
    //    close(serversockfd);
    //    exit(1);
    return serversockfd;
}
/************************************************************
 * 连接SOCKET服务器，如果出错返回-1，否则返回socket处理代码
 * server：服务器地址(域名或者IP),serverport：端口
 * ********************************************************/
int    connect_socket(string server,int serverPort){
    int    sockfd=0;
    struct    sockaddr_in    addr;
    struct    hostent        * phost;
    int yes=-1;
    //向系统注册，通知系统建立一个通信端口
    //AF_INET表示使用IPv4协议
    //SOCK_STREAM表示使用TCP协议
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        herror("Init socket error!");
        return -1;
    }
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&yes, sizeof(int))==-1)
    {
        herror("cant resuse the port and address");
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
    
    cout<<"a##########"<<endl;
    if(( recLenth=recv(sockfd,response+flag,RES_LENGTH-flag,0))==-1 )
    {
        cout<<response<<endl;
        delete []response;
        printf("Return value : %d\n", recLenth);
        perror("Recv msg error : ");
        return -1;
    }
    cout<<"BBBBBBBBBBBBBB"<<endl;
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
