#include <unistd.h> /* fork, close */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <vector>
#include <stdio.h> /* perror, fdopen, fgets */
#include <sys/socket.h>
#include <sys/wait.h> /* waitpid */
#include <netdb.h> /* getaddrinfo */
using namespace std;
#define die(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
const  char PORT[]="42422";
const int  NUM_CHILDREN=3;
const char TARGETADDR[]="0.0.0.0";
const int MAXLEN=1024;
struct client_info
{
string name;
unsigned short int src_port;
string src_addr;
};
int readline(int fd, char *buf, int maxlen); // forward declaration
int recvdata(int fd, char *buf, int maxlen); // forward declaration
void handle_event(int cliendtd);
void say_hello(int clientfd);
void save_client(struct sockaddr_in*,char *lines);
void handle_hello(struct sockaddr_in *client,int cliendtd,char * lines);
void error_hello(struct sockaddr_in * client,int clientfd,char * lines);
void say_bye(int clientfd);

vector<struct client_info> client_pool;




int main(int argc, char** argv)
{
	int n, sockfd, clientfd;
	int yes = 1;    // used in setsockopt(2)
	struct addrinfo *ai;
	struct sockaddr_in *client;
	socklen_t client_t;
	//pid_t cpid;     // child pid
	//char line[MAXLEN];
	//optchar cpid_s[32];
	//char welcome[32];
	/* Create a socket and get its file descriptor -- socket(2) */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		die("Couldn't create a socket");
	}
	/* Prevents those dreaded "Address already in use" errors */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&yes, sizeof(int)) == -1) {
		die("Couldn't setsockopt");
	}
    /* Fill the address info struct (host + port) -- getaddrinfo(3) */
    if (getaddrinfo(TARGETADDR, PORT, NULL, &ai) != 0) {   // get localhost
        die("Couldn't get address");
    }
    /* Assign address to this socket's fd */
    if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) != 0) {  // only bind on localhost ip
        die("Couldn't bind socket to address");
    }
    /* Free the memory used by our address info struct */
    freeaddrinfo(ai);
    /* Mark this socket as able to accept incoming connections */
    /* printf("Process %d Listening/n", getpid()); */
    if (listen(sockfd, 10) == -1) {
        die("Couldn't make socket listen");
    }
    while(1)
    {
        char *lines = new char[MAXLEN];
        client=new struct sockaddr_in;
        /* Necessary initialization for accept(2) */
        client_t = sizeof(struct sockaddr_in);
        /* Blocks! */
        printf("Waiting new connection!\n");
        clientfd = accept(sockfd, (struct sockaddr *)client, &client_t);
        pid_t child = fork();
        if(child==0){
            if (clientfd == -1) {
                die("Couldn't accept a connection");
            }

            n = recvdata(clientfd, lines, MAXLEN);
            switch(atoi(lines))
            {
                case 0:save_client(client,lines);handle_event(clientfd);break;
                case 1:break;
                default:error_hello(client,clientfd,lines);break;
            }
        }
    }
    close(sockfd);
    printf("Close server socket./n");
    return 0;
}

void save_client(struct sockaddr_in *client,char *lines)
{
    struct client_info my_client;
    string data=string(lines);
    my_client.src_port = ntohs(client->sin_port);
    my_client.src_addr=string(inet_ntoa(client->sin_addr));
    unsigned long pos =data.find('#');
    if(pos==data.npos)
    {
        cout<<"error client with wrong data format"<<endl;
    }
    else{
        char *pos_p=lines+pos+1;  //we skip ‘#’
        my_client.name = string(pos_p);
        cout<<my_client.src_addr<<":"<<(my_client.src_port)<<endl;
        client_pool.push_back(my_client);
    }
}
void handle_event(int clientfd)
{
    char *buf = new char[MAXLEN];
    say_hello(clientfd);
    bool flag=true;
    while(flag)
    {
        recvdata(clientfd, buf,MAXLEN);
        switch(atoi(buf))
        {
            case 10: break;
            case 11: say_bye(clientfd);flag=false;break;
            default:say_bye(clientfd);flag=false;break;
        }
    }
    //close(clientfd);  //client will close it
    delete [] buf;

}
void say_bye(int clientfd)
{
    cout<<"bye:==="<<clientfd<<endl;
    send(clientfd, "11", strlen("11"), 0);
}
void say_hello(int clientfd)
{
    send(clientfd, "10", strlen("10"), 0);
}
void error_hello(struct sockaddr_in * client,int clientfd,char * lines)
{
    struct client_info my_client;
    my_client.name=lines;
    my_client.name="UNKNOW";
    send(clientfd, "NAMEERROR", strlen("NAMEERROR"), 0);
    delete client;
    close(clientfd);
}
/*
 * Simple utility function that reads a line from a file descriptor fd,
 * up to maxlen bytes -- ripped from Unix Network Programming, Stevens.
 */
int readline(int fd, char *buf, int maxlen)
{
    int n, rc;
    char c;
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *buf++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0; // EOF, no data read
            else
                break; // EOF, read some data
        } else
            return -1; // error
    }
    *buf = '\0'; // null-terminate

    return n;
}
int recvdata(int fd, char *buf, int maxlen)
{
    return recv(fd, buf, maxlen, 0);
}
