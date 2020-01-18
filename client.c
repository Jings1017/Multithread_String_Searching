#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "client.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// function about thread
void main_thread_work(void*);
void child_thread_work(void*);

char new_dest[128];
char* port;
char* host;

int main(int argc, char *argv[])
{
    host = argv[2];
    if(host==NULL)
    {
        printf("host error\n");
        exit(-1);
    }

    port = argv[4];
    if(port==NULL)
    {
        printf("port error\n");
        exit(-1);
    }

    int network_socket;
    int i,j,check=0;

    char input[1000],toServer[5000];
    char *str[50],*ignore;


    while(1)
    {
        //get input message: Query "test" "tt"
        gets(input);

        // put "Query" in str[0]
        str[0] = strtok(input,"\""); // str[0]=Query
        i=1;

        // printf("%s.\n",tmp[0]);
        if(strcmp(str[0],"Query ")!=0)
        {
            printf("format error\n");
            continue;
        }

        memset(toServer,0,sizeof(toServer));
        // check the "" is a pair
        check=0;
        while(str[i] = strtok(NULL,"\"")) // test , tt
        {
            check++;
            // printf("%s.\n",str[i]);
            char buf[1000];
            memset(buf,0,sizeof(buf));
            sprintf(buf,"\"%s\"",str[i]);
            strcat(toServer,buf);

            if(ignore = strtok(NULL,"\""))
            {
                check++;
            }
            i++;
        }
        /*printf("str[]\n");
        for(int k=0;k<i;k++){
            printf("%d: %s\n",k,str[k]);
        }*/
        //printf("check: %d\n",check);

        if((check%2)!=1) // num of " is single
        {
            printf("format error (hint: \")\n");
            continue;
        }

        network_socket = socket(AF_INET,SOCK_STREAM,0);
        if(network_socket < 0)
        {
            printf("socket creation error \n");
            exit(-1);
        }

        //connection of the socket
        struct sockaddr_in info ;
        memset(&info,0,sizeof(info));
        info.sin_family = AF_INET;

        //local host test
        info.sin_addr.s_addr = inet_addr(host);
        info.sin_port = htons(atoi(port));

        int thread_count = 4;
        pthread_t *thread_pool = (pthread_t *)malloc(sizeof(pthread_t)* thread_count);
        for(int i=0; i<thread_count; i++)
        {
            pthread_create(&(thread_pool[i]),NULL,child_thread_work,(void *)&i);
            //sleep(1);
        }

        int connect_status = connect(network_socket,(struct sockaddr*)&info,sizeof(info));
        if(connect_status < 0)
        {
            printf("socket connection error.\n");
            exit(-1);
        }

        // printf("%s\n",toServer);
        send(network_socket, toServer, sizeof(toServer), 0);

        //recv msg
        char recvmsg[10000];
        memset(recvmsg,0,sizeof(recvmsg));
        recv(network_socket, recvmsg, sizeof(recvmsg), 0);
        //printf("receive: %s\n",recvmsg);

        char *msg[1000];

        // first group
        msg[0] = strtok(recvmsg,"|");
        //printf("msg[0]: %s\n",msg[0]);

        i=1;
        int start = 0,files = 1,notfound = 0;
        // find how many out string
        while(msg[i] = strtok(NULL,"|"))
        {
            i++;
        }

        int count = i;

        // each group
        for(i=0; i<count; i=i+3)
        {
            if(!msg[i+3] || strcmp(msg[i],msg[i+3])!=0)
            {
                // first string
                printf("String: \"%s\"\n",msg[start]);

                for(int j=0; j<files; j++)
                {
                    // check the string that count = 0
                    if(strcmp(msg[start+2+(3*j)],"Notfound")==0)
                    {
                        notfound++;
                    }
                }

                // can't find the string in each file
                if(notfound==files)
                {
                    printf("Not found\n");
                }
                // string exists
                else
                {
                    for(int j=0; j<files; j++)
                    {
                        if(strcmp(msg[start+2+(3*j)],"Notfound")!=0)
                        {
                            printf("File: ./%s, Count: %s\n",msg[start+1+(3*j)],msg[start+2+(3*j)]);
                        }
                    }
                }
                start = start + files*3 ;
                files = 1;
                notfound = 0;
            }
            // same string , add the # of file
            else
                files++;
        }
        close(network_socket);
    }
}

void main_thread_work(void *args)
{

}

void child_thread_work(void* args)
{

}
