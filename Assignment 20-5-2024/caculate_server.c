#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *client_proc(void *);

int clients[64];

int main()
{
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        //pthread_detach(tid);
    }

    return 0;
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[256];
    int a;
    int b;
    int result;
    char cmd[5];
    // Nhan du lieu tu client
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;
        //printf("Received from %d: %s\n", client, buf);

        if (strncmp(buf, "GET / ", 6) == 0)
        {
            // lấy a b cmd
            char *pos;
            pos = strstr(buf, "\r\n\r\n") + 4;
            sscanf(pos, "%d %d %s", &a, &b, cmd);
            printf("%d %d %s\n", a, b,cmd);
            if(strcmp(cmd,"add")==0){
                result=a+b;
            }else if(strcmp(cmd,"sub")==0){
                result=a-b;
            }else if(strcmp(cmd,"mul")==0){
                result=a*b;
            }else if(strcmp(cmd,"div")==0){
                result=a/b;
            }
            sprintf(buf,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>%d %s %d = %d</h1>",a,cmd,b,result);
            send(client, buf, strlen(buf), 0);
        }
    }

    close(client);
}