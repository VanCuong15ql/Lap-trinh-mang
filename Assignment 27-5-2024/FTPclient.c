#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#define defaultIP "192.168.89.103"
int client;
char buftype;
unsigned short send_pasv();
int send_list();
int download_file(char *remote_file);
int upload_file(char *local_file);
int rename_file(char *cur_file, char *new_file);
int delete_file(char *filename);
int print_working_dir();
int change_working_dir(char *dirname);
int make_dir(char *dirname);
int remove_dir(char *dirname);

int main()
{
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(defaultIP);
    addr.sin_port = htons(21);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhan xau chao
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Dang nhap
    char user[32], pass[32];
    printf("Nhap username: ");
    fgets(user, sizeof(user), stdin);
    printf("Nhap password: ");
    fgets(pass, sizeof(pass), stdin);

    // Xoa ky tu xuong trong
    user[strlen(user) - 1] = 0;
    pass[strlen(pass) - 1] = 0;

    // Gui lenh USER
    sprintf(buf, "USER %s\r\n", user);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Gui lenh PASS
    sprintf(buf, "PASS %s\r\n", pass);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Kiem tra dang nhap thanh cong
    if (strncmp(buf, "230 ", 4) == 0)
    {
        puts("Dang nhap thanh cong.");
    }
    else
    {
        puts("Dang nhap that bai.");
        close(client);
        return 1;
    }
    // danh sach thu muc
    send_list();
    // Hien thi menu chuc nang
    int command;
    int bt;
    while (1)
    {
        puts("Vui long chon chuc nang");
        puts("1.In noi dung thu muc hien tai");
        puts("2.Doi thu muc hien tai");
        puts("3.Tao thu muc");
        puts("4.Xoa thu muc");
        puts("5.Download file");
        puts("6.Upload file");
        puts("7.Doi ten file");
        puts("8.Xoa file");
        puts("9.Thoat va ngat ket noi");
        printf("Vui long nhap chuc nang: ");
        scanf("%d", &command);
        bt = getchar();
        // scanf("%d",&buftype);

        if (command == 9)
        {
            puts("Cam on ban da su dung dich vu, bye bye");
            break;
        }
        // xoa file
        if(command==8){
            char dirname[256];
            printf("Nhap ten file muon xoa: ");
            fgets(dirname,256,stdin);
            dirname[strlen(dirname)-1]=0;
            delete_file(dirname);
        }
        // doi ten file
        if (command == 7)
        {
            char dirname[254];
            char dirnamenew[254];
            printf("Nhap ten file ban muon doi ten: ");
            fgets(dirname, 254, stdin);
            printf("Nhap ten moi: ");
            fgets(dirnamenew, 254, stdin);
            dirnamenew[strlen(dirnamenew) - 1] = 0;
            dirname[strlen(dirname) - 1] = 0;

            rename_file(dirname, dirnamenew);
        }
        // in noi dung thu muc hien tai
        if (command == 1)
        {
            print_working_dir();
        }
        // doi thu muc hien taij
        if (command == 2)
        {
            char dirname[256];
            printf("Dien ten folder ban muon chuyen den: ");
            fgets(dirname, 256, stdin);
            dirname[strlen(dirname) - 1] = 0;
            change_working_dir(dirname);
        }
        // download file
        if (command == 5)
        {
            printf("Nhap ten file de download: ");
            char filename[256];
            fgets(filename, sizeof(filename), stdin);
            filename[strlen(filename) - 1] = 0;
            download_file(filename);
        }
        // up load
        if (command == 6)
        {
            // lieu ke ten va dung luong cac file
            char buf[2048];
            int pos = 0;

            char path[256];
            getcwd(path, sizeof(path));
            strcpy(buf, path);
            pos += strlen(path) + 1;
            DIR *d = opendir(path);
            struct dirent *dir;
            struct stat st;
            if (d != NULL)
                while ((dir = readdir(d)) != NULL)
                {
                    if (dir->d_type == DT_REG)
                    {
                        stat(dir->d_name, &st);
                        printf("%s - %ld bytes\n", dir->d_name, st.st_size);
                        strcpy(buf + pos, dir->d_name);
                        pos += strlen(dir->d_name) + 1;
                        memcpy(buf + pos, &st.st_size, sizeof(st.st_size));
                        pos += sizeof(st.st_size);
                    }
                }
            char dirname[256];
            printf("Vui long nhap ten file muon upload: ");
            fgets(dirname, sizeof(dirname), stdin);
            dirname[strlen(dirname) - 1] = 0;
            upload_file(dirname);
        }
        // tao thu muc
        if (command == 3)
        {
            char dir[256];
            printf("NHap ten thu muc muon tao: ");
            fgets(dir, sizeof(dir), stdin);
            dir[strlen(dir) - 1] = 0;
            make_dir(dir);
        }
        // xoa thu muc
        if (command == 4)
        {
            char dir[256];
            printf("Nhap thu muc muon xoa: ");
            fgets(dir, sizeof(dir), stdin);
            dir[strlen(dir) - 1] = 0;
            remove_dir(dir);
        }
    }
    close(client);
}
int print_working_dir()
{
    send_list();
    return 1;
}
int change_working_dir(char *dirname)
{
    char buf[1024];
    sprintf(buf, "cwd %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);
    int ret = recv(client, buf, 1024, 0);
    buf[ret] = 0;
    puts(buf);
    return 1;
}
int upload_file(char *dir)
{
    unsigned short port = send_pasv(client);
    printf("Port: %d\n", port);

    // Mo ket noi du lieu
    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr(defaultIP);
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *)&addr_data, sizeof(addr_data));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }
    char buf[1024];
    send(client, "cwd FTPtest\r\n", strlen("swd ftptest\r\n"), 0);
    ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    sprintf(buf, "stor %s\r\n", dir);
    send(client, buf, strlen(buf), 0);
    // mo file dirname, doc va gui du lieu
    FILE *f=fopen(dir,"rb");
    fseek(f,0,SEEK_SET);
    while(1){
        int ret=fread(buf,1,1024,f);
        if(ret<=0){
            break;
        }
        send(client_data,buf,strlen(buf),0);
    }
    fclose(f);
    
    ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    printf("Go phim bat ki de tiep tuc : ");
    buftype = getchar();
    buftype = getchar();
    return 1;
}
int remove_dir(char *dir)
{
    char buf[1024];
    send(client, "cwd FTPtest\r\n", strlen("cwd ftptest\r\n"), 0);
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    sprintf(buf, "rmd %s\r\n", dir);
    send(client, buf, strlen(buf), 0);
    ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    printf("Go phim bat ki de tiep tuc: ");
    buftype = getchar();
    buftype = getchar();
}
int make_dir(char *dirname)
{
    char buf[1024];
    send(client, "CWD FTPtest\r\n", strlen("cwd FTPtest\r\n"), 0);
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    sprintf(buf, "MKD %s\r\n", dirname);
    puts(buf);
    send(client, buf, strlen(buf), 0);
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }
    buf[ret] = 0;
    puts(buf);
    printf("Go phim bat ki de tiep tuc: ");
    scanf("%c", &buftype);
    scanf("%c", &buftype);
}
unsigned short send_pasv(int client)
{
    char buf[2048];

    send(client, "PASV\r\n", 6, 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    // puts(buf);

    char *pos = strchr(buf, '(');
    int i1 = atoi(strtok(pos, "(),"));
    int i2 = atoi(strtok(NULL, "(),"));
    int i3 = atoi(strtok(NULL, "(),"));
    int i4 = atoi(strtok(NULL, "(),"));
    int p1 = atoi(strtok(NULL, "(),"));
    int p2 = atoi(strtok(NULL, "(),"));

    return p1 * 256 + p2;
}

int send_list()
{
    // Gui lenh PASV
    unsigned short port = send_pasv(client);
    printf("Port: %d\n", port);

    // Mo ket noi du lieu
    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr(defaultIP);
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *)&addr_data, sizeof(addr_data));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    // Gui lenh LIST
    send(client, "LIST\r\n", 6, 0);

    char buf[2048];

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Nhan du lieu tren kenh du lieu
    // In ra man hinh
    while (1)
    {
        ret = recv(client_data, buf, sizeof(buf) - 1, 0);
        if (ret <= 0)
        {
            close(client_data);
            break;
        }

        buf[ret] = 0;
        printf("%s", buf);
    }

    printf("\n");

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
}

int download_file(char *remote_file)
{
    unsigned short port = send_pasv(client);
    printf("Port: %d\n", port);

    // Mo ket noi du lieu
    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr(defaultIP);
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *)&addr_data, sizeof(addr_data));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Gui lenh retr
    sprintf(buf, "retr %s\r\n", remote_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Nhan noi dung file
    FILE *f = fopen(remote_file, "w");
    while (1)
    {
        ret = recv(client_data, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }
    close(client_data);
    fclose(f);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
}
int rename_file(char *dirname, char *dirnamenew)
{
    char buf[1024];
    sprintf(buf, "rnfr %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);
    int ret = recv(client, buf, 1024, 0);
    if (ret <= 0)
    {
        close(client);
        return 0;
    }
    buf[ret] = 0;
    puts(buf);
    sprintf(buf, "rnto %s\r\n", dirnamenew);
    send(client, buf, strlen(buf), 0);
    ret = recv(client, buf, 1024, 0);
    buf[ret] = 0;
    puts(buf);
    return 1;
}
int delete_file(char *dirname){
    char buf[1024];
    sprintf(buf,"dele %s\r\n",dirname);
    send(client,buf,strlen(buf),0);
    int ret=recv(client,buf,1024,0);
    if(ret<=0){
        close(client);
        return 0;
    }
    buf[ret]=0;
    puts(buf);
    return 1;
}