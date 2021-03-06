#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "server.h"

// function about thread
void main_thread_work(void*);
void child_thread_work(void*);

// function about file
int search_file(char*,char*);
int process_file(char*,char*);

// function about queue
void push(int);
int pop(void);
int isEmpty(void);

char str_head[100],*search[10];
char *port,*root;

typedef struct queue
{
    int capacity;
    int size;
    int front;
    int rear;
    int *elements;
} queue;

struct node
{
    int data;
    struct node* next;
};

struct node * queue_head = NULL;
struct node * queue_tail = NULL;
int queue_num = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
    //get input and argv
    root = argv[2];
    if(root==NULL)
    {
        printf("root error\n");
        exit(-1);
    }

    port = argv[4];
    if(port==NULL)
    {
        printf("port error\n");
        exit(-1);
    }

    int thread_num = atoi(argv[6]);
    if(thread_num <=0 )
    {
        printf("thread error\n");
        exit(-1);
    }

    memset(str_head,0,sizeof(str_head));
    strcat(str_head,root);

    // socket creation
    int server_socket, client_socket;
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket < 0)
    {
        printf("server socket creation error\n");
    }

    // socket connection
    struct sockaddr_in server_addr,client_addr;
    socklen_t addrlen = sizeof(client_addr);
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port)); // host to network short integer
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind and listen the socket
    bind(server_socket,(struct sockaddr *)&server_addr,sizeof(server_addr));

    listen(server_socket,100);

    //create threadpool
    pthread_mutex_init(&mutex,NULL);
    pthread_t *thread_pool = (pthread_t *)malloc(sizeof(pthread_t)* thread_num);
    for(int i=0; i<thread_num; i++)
    {
        pthread_create(&(thread_pool[i]),NULL,child_thread_work,(void *)&i);
        //pthread_create(&(thread_pool[i]),NULL,child_thread_work,NULL);
        sleep(1);
    }

    while(1)
    {
        client_socket = accept(server_socket,(struct sockaddr*)&client_addr,&addrlen);

        pthread_mutex_lock(&mutex);
        push(client_socket);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    pthread_join(*thread_pool,NULL);

    close(server_socket);
    close(client_socket);

    return 0;
}

void main_thread_work(void *args)
{
    int num;
    while(isEmpty())
    {
        pthread_cond_wait(&cond,&mutex);
    }
    while(1)
    {

    }
    return;
}

void child_thread_work(void* args)
{
    //int num = *(int*)args;
    while(1)
    {
        pthread_mutex_lock(&mutex);

        while(isEmpty())
        {
            pthread_cond_wait(&cond,&mutex);
        }

        int buffer_socket = pop();
        char input_str[1000] ;
        memset(input_str,0,sizeof(input_str));
        recv(buffer_socket,input_str,sizeof(input_str),0);

        char buffer_tmp[256],*temp,*buffer;

        memset(buffer_tmp,0,sizeof(buffer_tmp));


        sprintf(buffer_tmp,"Query %s",input_str);

        pthread_mutex_unlock(&mutex);

        // buffer = Query
        buffer = strtok(buffer_tmp,"\"");

        int search_num = 0 ;

        while(search[search_num]=strtok(NULL,"\""))
        {
            //printf("search[%d] %s\n",search_num,search[search_num]);
            search_num++;
        }

        // print Query "**" "***"
        printf("%s ",buffer);
        for(int i=0; i<search_num; i++)
        {
            printf("\"%s\" ",search[i]);
        }
        printf("\n");
        //printf("%s\n",query_string);

        // //
        char buf[30][500];
        memset(buf,0,sizeof(buf));

        char paths[20][200];
        memset(paths,0,sizeof(paths));

        int path_num = 0;
        int start_num = 0;
        int n=0;
        char dir_path[1000],count_str[1000];

        //find files
        struct stat sb;

        for(int i=0; i<search_num; i++)
        {
            // init
            path_num = 0;
            start_num = 0;
            // printf("%s\n",search[i]);

            memset(dir_path,0,sizeof(dir_path));
            strcpy(dir_path,str_head);

            while(start_num <= path_num)
            {
                char output[1000];
                memset(output,0,sizeof(output));
                char out[1000];
                memset(out,0,sizeof(out));

                if(stat(dir_path,&sb)==0)
                {
                    // find dir
                    if(S_ISDIR(sb.st_mode))
                    {
                        search_file(dir_path,output);

                        char *child_path;

                        child_path = strtok(output," ");

                        sprintf(paths[path_num],"%s/%s",dir_path,child_path);

                        path_num++;

                        while(child_path = strtok(NULL," "))
                        {
                            char path_buf[1000];
                            memset(path_buf,0,sizeof(path_buf));
                            sprintf(path_buf,"%s/%s",dir_path,child_path);
                            strcat(paths[path_num],path_buf);
                            path_num++;
                        }
                        // printf("paths[%d] = %s\n",start_num,dir_path);
                    }
                    // file
                    else if(S_ISREG(sb.st_mode))
                    {
                        // %s|%s|%d
                        sprintf(buf[n],"%s|%s|",search[i],dir_path);
                        
                        // if the string exists in the file
                        if(process_file(dir_path,search[i]) != 0)
                        {
                            // printf("%d\n",count);
                            memset(count_str,0,sizeof(count_str));
                            sprintf(count_str,"%d",process_file(dir_path,search[i]));
                            strcat(buf[n],count_str);
                        }
                        else
                        {
                            // printf("Not found\n");
                            strcat(buf[n],"Notfound");
                        }
                        // printf("buf[%d] = %s\n",n,buf[n]);
                        n++;
                    }
                }
                strcpy(dir_path,paths[start_num]);
                start_num++;
            }
            memset(paths,0,sizeof(paths));
        }


        // send the final message  ****|****|****
        char back[10000];
        memset(back,0,sizeof(back));

        // connect each string to back string
        int b = 0;
        // n is string-file-count num
        while(b < n)
        {
            strcat(back,buf[b++]);
            strcat(back,"|");
        }

        // printf("*%s*\n",back);
        send(buffer_socket,back,sizeof(back),0);

        close(buffer_socket);
    }
    pthread_exit(NULL);
}

int search_file(char* path,char* output)
{
    //printf("path: %s\n",path);
    DIR *dp;
    char glue = '/';
    struct dirent* filename;
    char fp[200];
    memset(fp,0,sizeof(fp));
    sprintf(fp,"./%s",path);
    dp = opendir(fp);
    if(dp)
    {
        while((filename = readdir(dp))!=NULL)
        {
            if(strcmp(filename->d_name,".")!=0 && strcmp(filename->d_name,"..")!=0)
            {
                char buf[1000];
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%s ",filename->d_name);
                strcat(output,buf);
            }
        }
        closedir(dp);
        return 1;
    }
    else
    {
        //printf("path: %s\n",file_path);
        return 0;
    }
}

int process_file(char *path,char *key)
{
    char buffer[1000],goal[1000];
    memset(buffer,0,sizeof(buffer));

    strcpy(goal,key);
    char word;
    FILE *fp;
    fp = fopen(path,"r");
    int count = 0, k = 0;

    while(fgets(buffer,1000,fp)!=NULL)
    {
        //printf("buffer %s\n",buffer);
        int len = strlen(buffer)-strlen(goal)+1 ;
        // printf("len %d\n",len);
        for(int i=0; i< len ; i++)
        {
            int same = 0;
            for(int j=0; j<strlen(goal); j++)
            {
                if(buffer[i+j]==goal[j])
                    same++;
                else
                    continue;
            }
            if(same == strlen(goal))
                count++;
        }
    }
    close(fp);

    //printf("count: %d\n",count);
    return count;
}

void push(int data)
{
    queue_num++;
    // first data
    if(queue_head == NULL)
    {
        queue_head = (struct node*)malloc(sizeof(struct node));
        queue_head->data = data;
        queue_head->next = NULL;
        queue_tail = queue_head;
    }
    // others
    else
    {
        struct node* ptr;
        ptr = (struct node*)malloc(sizeof(struct node));
        ptr->data = data;
        ptr->next = NULL;

        queue_tail = ptr;
        queue_tail->next = ptr;
    }
}

int pop(void)
{
    queue_num--;
    int result;
    struct node* ptr = queue_head;
    result = ptr->data;
    queue_head = ptr->next;
    free(ptr);
    return result;
}

int isEmpty(void)
{
    if(queue_num == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}