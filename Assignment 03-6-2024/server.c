#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *client_proc(void *);

int get_index(int client);
int remove_client(int client, int *client_sockets, char **client_names, int *num_clients);

int check_join(int client);
int process_join(int client, char *buf);
int process_msg(int client, char *buf);
int process_pmsg(int client, char *buf);
int process_op(int client, char *buf);
int process_kick(int client,char *buf);
int process_topic(int client, char *buf);
int process_quit(int client);

int client_sockets[1024];
char *client_names[1024];
int num_clients = 0;
int op=0;
int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    while (1) {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);
        
        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        //pthread_detach(tid);
    }

    return 0;
}

int remove_client(int client, int *client_sockets, char **client_names, int *num_clients) {
    int i = 0;
    for (; i < *num_clients; i++)
        if (client_sockets[i] == client)
            break;
    
    if (i < *num_clients) {
        if (i < *num_clients - 1) {
            client_sockets[i] = client_sockets[*num_clients - 1];
            strcpy(client_names[i], client_names[*num_clients - 1]);
        }

        free(client_names[*num_clients - 1]);
        *num_clients -= 1;
    }
}

void *client_proc(void *arg) {
    int client = *(int *)arg;
    char buf[256];

    // Nhan du lieu tu client
    while (1) {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0) {
            remove_client(client, client_sockets, client_names, &num_clients);
            break;
        }
            
        buf[ret-1] = 0;
        printf("Received from %d: %s\n", client, buf);

        if (strncmp(buf, "JOIN ", 5) == 0)
            process_join(client, buf);
        else if (strncmp(buf, "MSG ", 4) == 0)
            process_msg(client, buf);
        else if (strncmp(buf, "PMSG ", 5) == 0)
            process_pmsg(client, buf);
        else if (strncmp(buf, "OP ", 3) == 0)
            process_op(client,buf);
        else if (strncmp(buf, "KICK ", 5) == 0)
            process_kick(client, buf);
        else if (strncmp(buf, "TOPIC ", 6) == 0)
            process_topic(client, buf);
        else if (strncmp(buf, "QUIT", 4) == 0)
            process_quit(client);
        else {
            char *msg = "999 Unknown command\n";
            send(client, msg, strlen(msg), 0);
        }
    }

    close(client);
}

int check_join(int client) {
    int i = 0;
    for (; i < num_clients; i++)
        if (client_sockets[i] == client)
            break;
    return i < num_clients;
}

int get_index(int client) {
    int i = 0;
    for (; i < num_clients; i++)
        if (client_sockets[i] == client)
            break;
    return i;
}

int process_join(int client, char *buf) {
    if (!check_join(client)) {
        // Chua dang nhap
        char cmd[16], id[32], tmp[32];
        int n = sscanf(buf, "%s %s %s", cmd, id, tmp);
        if (n == 2) {
            // Kiem tra tinh hop le cua id
            // id chi chua ky tu chu thuong va chu so
            int k = 0;
            for (; k < strlen(id); k++)
                if (id[k] < '0' || id[k] > 'z' || (id[k] > '9' && id[k] < 'a')) 
                    break;
            if (k < strlen(id)) {
                // id chua ky tu khong hop le
                char *msg = "201 Invalid nickname\n";
                send(client, msg, strlen(msg), 0);
            } else {
                // Kiem tra id da ton tai chua
                k = 0;
                for (; k < num_clients; k++)
                    if (strcmp(client_names[k], id) == 0)
                        break;
                if (k < num_clients) {
                    // id da ton tai
                    char *msg = "200 Nickname in use\n";
                    send(client, msg, strlen(msg), 0);
                } else {
                    // id chua ton tai
                    char *msg = "100 OK\n";
                    send(client, msg, strlen(msg), 0);

                    // Chuyen client sang trang thai dang nhap
                    client_sockets[num_clients] = client;
                    client_names[num_clients] = malloc(strlen(id) + 1);
                    memcpy(client_names[num_clients], id, strlen(id) + 1);
                    // strcpy(client_names[num_clients], id);
                    num_clients++;

                    // Gui thong diep cho cac client khac
                    for (int k = 0; k < num_clients; k++)
                        if (client_sockets[k] != client) {
                            char msg[512];
                            sprintf(msg, "JOIN %s\n", id);
                            send(client_sockets[k], msg, strlen(msg), 0);
                        }
                }
            }
        } else {
            char *msg = "999 Unknown error\n";
            send(client, msg, strlen(msg), 0);
        }
    } else {
        char *msg = "888 Wrong state\n";
        send(client, msg, strlen(msg), 0);
    }

    return 0;
}

int process_msg(int client, char *buf) {
    if (check_join(client)) {
        int idx = get_index(client);
        // Chuyen tiep tin nhan cho cac client da dang nhap khac
        for (int k = 0; k < num_clients; k++)
            if (client_sockets[k] != client) {
                char msg[512];
                sprintf(msg, "MSG %s %s\n", client_names[idx], buf + 4);
                send(client_sockets[k], msg, strlen(msg), 0);
            }

        char *msg = "100 OK\n";
        send(client, msg, strlen(msg), 0);
    } else {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }
    return 0;
}
int process_pmsg(int client, char *buf) {
    if (check_join(client)) {
        int idx = get_index(client);

        char receiver[32];
        sscanf(buf + 5, "%s", receiver);

        // Chuyen tiep tin nhan cho mot client
        int k = 0; 
        for (; k < num_clients; k++)
            if (strcmp(client_names[k], receiver) == 0)     
                break;
        
        if (k < num_clients) {
            // Tim thay nguoi nhan
            char msg[512];
            sprintf(msg, "PMSG %s %s\n", client_names[idx], buf + strlen(receiver) + 6);
            send(client_sockets[k], msg, strlen(msg), 0);

            char *response = "100 OK\n";
            send(client, response, strlen(response), 0);
        } else {
            // Khong thay nguoi nhan
            char *msg = "202 Unknown nickname\n";
            send(client, msg, strlen(msg), 0);
        }
    } else {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }
    return 0;
}
int process_op(int client, char *buf) {
    char msg[256];
    if(check_join(client)){
        int index=get_index(client);
        if(index!=op){
            sprintf(msg,"203 DENIED");
            send(client,msg,strlen(msg),0);
            return 1;
        }
        int k=0;
        for(;k<num_clients;k++){
            if(strcmp(client_names[k],buf+3)==0){
                break;
            }
        }
        if(k>num_clients){
            strcat(msg,"202 UNKNOWN NICKNAME");
            send(client,msg,strlen(msg),0);
        }
        for(int j=0;j<num_clients;j++){
            if(j!=op){
                send(client_sockets[j],buf,strlen(buf),0);
            }
        }
        strcat(msg,"100 OK");
        send(client,msg,strlen(msg),0);

        op=k;
    }else{
        strcpy(msg,"999 UNKNOWN ERROR");
        send(client,msg,strlen(msg),0);
    }
    return 0;
}
int process_kick(int client,char *buf) {
    printf("%d\n",num_clients);
    if(check_join(client)){
        int index=get_index(client);
        char namekick[128];

        sscanf(buf+5,"%s",namekick);
        puts(namekick);
        puts(buf+5);
        // kiem tra chu phong
        if(index!=op){
            send(client,"203 DENIED",strlen("203 denied"),0);
            return 1;
        }
        int k = 0; 
        for (; k < num_clients; k++)
            if (strcmp(client_names[k], buf + 5) == 0)  {
                puts(client_names[k]);
                break;
            }  
        printf("%d\n",client_sockets[k]);
        if(k<num_clients){
            if(close(client_sockets[k])==-1){
                puts("close error");
            }
            remove_client(client_sockets[k],client_sockets,client_names,&num_clients);
            send(client,"100 OK",strlen("100 ok"),0);
            char msg[256];
            sprintf(msg,"KICK %s %s",namekick,client_names[0]);

            for(int j=0;j<num_clients;j++){
                if(j!=op){


                    send(client_sockets[j],msg,strlen(msg),0);
                }
            }
            return 0;
        }else{
            char *msg="202 UNKNOWN NICKNAME";
            send(client,msg,strlen(msg),0);
        }

    }else{
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }

    return 1;
}
int process_topic(int client,char *buf) {
    if(check_join(client)){
        int index=get_index(client);
        if(index!=op){
            send(client,"203 DENID",strlen("203 denied"),0);
            return 0;
        }
        char *msg;
        sprintf(msg,"TOPIC %s %s",client_names[op],buf+6);
        for(int k=0;k<num_clients;k++){
            if(k!=op){
                send(k,msg,strlen(msg),0);
            }
        }
        send(op,"100 OK",6,0);
    }
    return 0;
}
int process_quit(int client) {
    if(check_join(client)){
        int index=get_index(client);
        char msg[256];
        sprintf(msg,"QUIT %s",client_names[index]);
        remove_client(client,client_sockets,client_names,&num_clients);   
        for(int k=0;k<num_clients;k++){
            send(client_sockets[k],msg,strlen(msg),0);
        }
    }
    close(client);
    return 0;
}