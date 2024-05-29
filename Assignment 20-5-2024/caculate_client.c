#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main() 
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Kết nối server local host
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8000);
    struct addrinfo *info;

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }
        char req[] = "GET / HTTP/1.1\r\n\r\n5 6 sub";
        send(client, req, strlen(req), 0);

    char buf[2048];
    while(1){
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0) {
            close(client);
            return 1;
        }

        buf[ret] = 0;
        printf("%d bytes received\n%s\n", ret, buf);
        char *pos;
        pos=strstr(buf,"\r\n\r\n")+4;
        char msg[20];
        strcpy(msg,pos);
        FILE *f=fopen("text.html","wb");
        fwrite(msg,1,strlen(msg),f);
        fclose(f);

    }
    



    close(client);
}