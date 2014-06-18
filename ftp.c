#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#define SERV_PORT 21
#define MAXSIZE 1024
#define SA struct sockaddr
static int control_sockfd;
int npsupport;
int login_yes;
int f;//f=0时为默认文件结构
int login();
void ftp_list(int control_sockfd);
void zeromery(char *a,int len);
void ftp_pwd(int control_sockfd);
void ftp_changdir(char dir[],int control_sockfd);
void ftp_quit(int control_sockfd);
void ftp_creat_mkd(char *path,int control_sockfd);
void ftp_back(int control_sockfd);
void ftp_stru(int control_sockfd);
void ftp_rest(int control_sockfd);
int ftp_download(int control_sockfd);
char *itoa(int value, char *string, int radix);
int  main(int argc,char **argv)
{
    printf("ftp>");
    char command[MAXSIZE];
    char*cmd;
    scanf("%s",command);
    cmd=command;
    while(*(cmd)==' ')
        cmd++;

    if(strncmp(cmd,"login",5)==0)
    {
        login();
        if(login_yes==1)
        {
            while(1)
            {
            comm:            
                sleep(1);
                printf("ftp>");
                zeromery(command,1024);
                scanf("%s",command);
                cmd=command;
                while(*(cmd)==' ')
                cmd++;
                if(strncmp(cmd,"pasv",4)==0)
                {
                    ftp_list(control_sockfd);
                }
                if(strncmp(cmd,"port",4)==0)
                {
                    ftp_list(control_sockfd);
                }
                if(strncmp(cmd,"list",4)==0)
                {
                    ftp_pwd(control_sockfd);
                    ftp_list(control_sockfd);
                }
                if(strncmp(cmd,"pwd",3)==0)
                {
                    ftp_pwd(control_sockfd);
                }
                if(strncmp(cmd,"mkdir",5)==0)
                {
                    char path[60];
                    zeromery(path,60);
                    printf("创建的路径名: ");
                    scanf("%s",path);
                    printf("s/n",path);
                    ftp_creat_mkd(path,control_sockfd);
                }
                if(strncmp(cmd,"back",4)==0)
                {
                    ftp_back(control_sockfd);

                    ftp_pwd(control_sockfd);
                }
                if(strncmp(cmd,"cd",2)==0)
                {
                    int i;
                    char path[60];
                    zeromery(path,60);
                    printf("要到的路径：");
                    scanf("%s",path);
                    printf("%s/n",path);
                    ftp_changdir(path,control_sockfd);
                }
                if(strncmp(cmd,"get",3)==0)
                {
                    ftp_pwd(control_sockfd);    
                    ftp_download(control_sockfd);
                }
                if(strncmp(cmd,"up",3)==0)
                {
                    ftp_pwd(control_sockfd);
                    ftp_up(control_sockfd);        
                }
                if(strncmp(cmd,"quit",4)==0)
                {
                    printf("bye^_^/n");
                    close(control_sockfd);
                    break;
                }
                printf("支持 list,pwd,mkdir,back,cd,up,get/n");    
        }
        
    }
    else if(login_yes==0)
    { 
        int i;//不成功登录下最多还有两次机会，如果不能在两次登录，则，关闭链接。
        printf("Can not login vsftpd");
        for(i=2;i>0;i--)
        {
            printf("你还有 %d 登录机会/n",i);            
            login();
            if(login_yes==1)
            {
                goto comm;
            }
        }
        if(i==0)
        {
            printf("你不能在登录！/n");
            close(control_sockfd);
        }
            
    }
    else if (strncmp(cmd,"quit",4)==0)
    {
        ftp_quit(control_sockfd);
        close(control_sockfd);
        
    }
    }
    return 0;
}
int login()
{
    //初始化端口信息
    struct sockaddr_in serv_addr;
    char senddate,recvdate;
    char sendline[MAXSIZE],recvline[MAXSIZE];
    struct hostent *host;

    //获取hostent中相关参数
    char name[MAXSIZE],password[MAXSIZE];
    printf("please enter the hostname/n");
    printf("ftp-> ");
    scanf("%s",name);
    host=gethostbyname(name);
    if(host==NULL)
    {
        printf("get host by name is error!/n");
        login_yes=0;
    }
    else
    {
        //创建socket
        control_sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(control_sockfd<0)
        {
            printf("socket is error/n");
            login_yes=0;
        }
    

        //设置sockaddr_in 结构体中的相关参数
        bzero(&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_port=htons(SERV_PORT);
        serv_addr.sin_addr.s_addr=INADDR_ANY;

        //调用connect函数发起连接
        char addr[MAXSIZE];
        if((connect(control_sockfd,(SA*)&serv_addr,sizeof(serv_addr)))<0)
        {
            printf("connect is error/n");
            login_yes=0;
        }
        printf("connect to %s/n",inet_ntop(AF_INET,host->h_addr,addr,1024));
        recvdate=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(recvdate==-1)
        {
            printf("recvdate is connect error/n");
            login_yes=0;
        }
        else if(strncmp(recvline,"220",3)==0)
        {
            printf("connect success,pelase enter username/n");
            login_yes=1;
        }
        else 
        {
            printf("220 connect is error!");
            login_yes=0;    
        }

        //ftp用户登录主体部分
        int sendbytes,recvbytes;
        zeromery(name,1024);
        zeromery(password,1024);
        zeromery(recvline,1024);
        zeromery(sendline,1024);
        printf("ftp-> ");
        scanf("%s",name);//可以支持匿名登录vsftpd
        strcat(sendline,"USER ");
        strcat(sendline,name);
        strcat(sendline,"/r/n");
        printf("--->%s/n",sendline);
        sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
        if(sendbytes==-1)
        {
            printf("send is wrong/n");
            login_yes=0;
        }
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(strncmp(recvline,"331",3)==0)
        {
            printf("331 please specify the password./n");
        }
        else
        {
            printf("recv date is error./n");
            login_yes=0;
        }
        zeromery(sendline,1024);
        zeromery(recvline,1024);
        printf("ftp-> ");
        scanf("%s",password);
        strcat(sendline,"PASS ");
        strcat(sendline,password);
        strcat(sendline,"/r/n");
        printf("--->%s/n",sendline);
        sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
        if(sendbytes==-1)
        {
            printf("pass send is error/n");
            login_yes=0;
        }
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(strncmp(recvline,"230",3)==0)
        {
            printf("login success!/n");
            login_yes=1;
        }
        else 
        {
            printf("pass recv is error/n");
            login_yes=0;
        }


        //支持断点续传
        zeromery(sendline,1024);
        zeromery(recvline,1024);
        strcat(sendline,"REST ");
        strcat(sendline,"0");
        strcat(sendline,"/r/n");
        printf("--->%s/n",sendline);
        sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
        if(sendbytes==-1)
        {
            printf("rest send is error!/n");
            login_yes=0;
        }
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(recvbytes==-1)
        {
            printf("rest recv date is error./n");    
            login_yes=0;    
        }
        if(strncmp(recvline,"350 Restart position accepted (0).",34)==0)
        {
            npsupport=1;
            printf("support 断点续传/n");
            login_yes=1;
        }
        else
        {
            npsupport=0;
            printf("not support 断点续传/n");
            login_yes=0;    
        }
    

        //获取服务器版本信息
        zeromery(recvline,1024);
        zeromery(sendline,1024);
        strcat(sendline,"SYST");
        strcat(sendline,"/r/n");
        printf("--->%s/n",sendline);
        sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
        if(sendbytes==-1)
        {
            printf("syst send is error/n");
            login_yes=0;
        }
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(recvbytes==-1)
        {
            printf("syst recv is error/n");
            login_yes=0;
        }
        if(strncmp(recvline,"215 UNIX Type: L8",17)==0)
        {
            printf("%s",recvline);
            login_yes=1;
        }
        else 
        {
            printf("syst recv connectin is error/n");
            login_yes=0;
        }    
    }
    
    return login_yes;
}

//数组初始化
void zeromery(char *a,int len)
{
    int i;
    len=sizeof(a);
    for(i=0;i<len;i++)
    {
        a[i]=0;
    }
}

//quit函数,control_sockfd,通过实参传递
void ftp_quit(int control_sockfd )
{
    char sendline[1024];
    char recvline[1024];
    int recvbytes;
    int sendbytes;
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"QUIT");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("quit send is error!/n");
        exit(1);
    }
    recvbytes=recv(control_sockfd,recvline,strlen(recvline),0);
    if(strncmp(recvline,"221",3)==0)
    {
        printf("221 bye!^_^");
        exit(1);
    }
    else
    {
        printf("quit recv is error!/n");
        exit(1);
    }
}


//mkd,在所在路径中创建目录 函数
void ftp_creat_mkd(char *path,int control_sockfd)
{
    char sendline[1024];
    char recvline[1024];
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    int recvbytes,sendbytes;
    int issuccess;
    strcat(sendline,"MKD ");
    strcat(sendline,path);
    strcat(sendline,"/r/n");
    printf("%s/n",sendline);
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("mkd send is error!");
        exit(1);
    }
    recvbytes=recv(control_sockfd,recvline,strlen(recvline),0);
    if(strncmp(recvline,"257",3)==0)
    {
        issuccess=1;
    }
    else
    {
        issuccess=0;
    }
}


//改变目录函数chdir
void  ftp_changdir(char *dir,int control_sockfd)
{
    
    char sendline[1024];
    char recvline[1024];
    int recvbytes,sendbytes;
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"CWD ");
    strcat(sendline,dir);
    strcat(sendline,"/r/n");
    printf("%s/n",sendline);
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("cwd send is error!/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(recvbytes<0)
    {
        printf("cwd recv is error!/n");
    }
    if(strncmp(recvline,"250",3)==0)
    {
                char buf[55];
                snprintf(buf,39,">>> %s/n",recvline);
                printf("%s/n",buf);
    }
    else
    {
        printf("cwd chdir is error!/n");
        exit(1);
    }
}


//pwd 命令函数
//在应答中返回当前工作目录，“pwd”+/r/n
void ftp_pwd(int control_sockfd)
{
    int recvbytes,sendbytes;
    char sendline[1024],recvline[1024];
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"PWD");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("pwd,send is error/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(strncmp(recvline,"257",3)==0)
    {
        int i=0;
        char *ptr;
        char currendir[1024];
        zeromery(currendir,1024);
        ptr=recvline+5;
        while(*(ptr)!='"')
        {
            currendir[i++]=*(ptr);
            ptr++;
        }
        currendir[i]='/0';
        printf("current directory is:%s/n",currendir);

    }
    else
    {
        printf("pwd,recv is error!/n");
    }
}



//获取服务器文件列表
//list命令，是数据通道，通过的是21端口。the function 's struct is "the data //transport mode"(ascii or b) puls "the data mode to transport"(pasv or port//) puls "the list command"
void ftp_list(int control_sockfd)
{
    int pasv_or_port;// 定义the ftp协议的两种不同工作mode
    int recvbytes,sendbytes;
    char sendline[1024],recvline[1024];
    struct sockaddr_in serv_addr;
    int i,j;
    int flag=0;
    int data_sockfd;

    //用户来选择pasv 或者是 port mode(默认的是pasv模式)
    char selectdata_mode_tran[1024];
    zeromery(selectdata_mode_tran,1024);
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    //printf("ftp->ftp协议工作方式选择（pasv or port）/n");
    //printf("ftp->");
//    scanf("%s",selectdata_mode_tran);
    //if(strncmp(selectdata_mode_tran,"pasv",4)==0)
    //{
        pasv_or_port=0;
//    }
//    if(strncmp(selectdata_mode_tran,"port",4)==0)
//    {
//        pasv_or_port=1;
//    }
    //pasv mode
    if(pasv_or_port==0)
    {
        strcat(sendline,"PASV");
        strcat(sendline,"/r/n");
        sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
        if(sendbytes<0)
        {
            printf("pasv send is error!/n");
        }
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(recvbytes<0)
        {
            printf("pasv recv is error!/n");
        }
        if(strncmp(recvline,"227",3)==0)
        {
            printf("%s/n",recvline);
        }
        else
        {
            printf("pasv recv is error!/n");
        }    
        //处理ftp server 端口
        char *ptr1,*ptr2;
        char num[1024];
        zeromery(num,1024);
        //取低位字节
        ptr1=recvline+strlen(recvline);
        while(*(ptr1)!=')')
        {
            ptr1--;
        }
        ptr2=ptr1;
        while(*(ptr2)!=',')
            ptr2--;
        strncpy(num,ptr2+1,ptr1-ptr2-1);
        i=atoi(num);//将字符串转换成整数
        //取高位字节
        zeromery(num,1024);
        ptr1=ptr2;
        ptr2--;
        while(*(ptr2)!=',')
            ptr2--;
        strncpy(num,ptr2+1,ptr1-ptr2-1);
        j=atoi(num);
        //初始化服务器数据连接时的端口信息
        int data_serviceport;
        data_serviceport=j*256+i;
        data_sockfd=socket(AF_INET,SOCK_STREAM,0);
                
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=INADDR_ANY;
        serv_addr.sin_port=htons(data_serviceport);
        if(connect(data_sockfd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))==-1)
        {
            printf("pasv data connect is error!/n");
        }
    }
    //port mode
    if(pasv_or_port==1)
    {
        data_sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(data_sockfd<0)
        {
            printf("创建数据端口连接失败！/n");
        }
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=INADDR_ANY;
        serv_addr.sin_port=htons(SERV_PORT);
        
        int ret;
        int addrlen;
        ret=sizeof(struct sockaddr_in);
        getsockname(data_sockfd,(SA*)&serv_addr,&ret);
        
        //处理port 后面要带的参数
        char ip[1024];
        int i,j;
        char data[1024];
        zeromery(ip,1024);
        zeromery(data,1024);
        inet_ntop(AF_INET,&(serv_addr.sin_addr),ip,sizeof(ip));
        printf("%s/n",ip);
        i=data_sockfd/256;
        j=data_sockfd%256;

        //将点分十进制的点转换为逗号。
        char *ptr1;
        ptr1=ip;
        while(*(ptr1)!='/0')
        {
            if(*(ptr1)=='.')
            {
                *(ptr1)=',';
            }
            ptr1++;
        }

        strcat(sendline,"PORT ");
        strcat(sendline,ip);
        strcat(sendline,",");
        strcat(sendline,itoa(i,data,10));
        strcat(sendline,",");
        strcat(sendline,itoa(j,data,10));
        strcat(sendline,"/r/n");
        printf("--->%s/n",sendline);
        sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
        if(sendbytes<0)
        {
            printf("port send is error!/n");
            exit(1);
        }
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(strncmp(recvline,"200",3)==0)
        {
            printf("%s/n",recvline);
        }
        else
        {
            printf("port recv is error!/n");
        }

    }

    //type
    zeromery(recvline,1024);
    zeromery(sendline,1024);
    strcat(sendline,"TYPE ");
    strcat(sendline,"I");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf(" type send is error!/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(strncmp(recvline,"200",3)==0)
    {
        printf("使用二进制传输数据/n");
    }
    else
    {
        printf("type recv is error!/n");
    }
        

    //list
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"LIST");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("list send is error!/n");
    }
recvdata:
    sleep(1);
    recvbytes=recv(data_sockfd,recvline,sizeof(recvline),0);
    if(recvbytes<0)
    {
        close(data_sockfd);
        goto ending;
    }
    printf("%s",recvline);
    if(flag==0)
    {
        zeromery(recvline,1024);
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(strncmp(recvline,"226",3)!=0)
        {
            flag=1;
            goto recvdata;
        }
    }
ending:
    if(flag!=1)
    {
        zeromery(recvline,1024);
    }
    close(data_sockfd);    
}


//itoa 函数的实现(整数转换成字符串)
char *itoa(int value, char *string, int radix)
{
    char tmp[33];
    char *tp = tmp;
    int i;
    unsigned v;
    int sign;
    char *sp;

    sign = (radix == 10 && value < 0);
    if (sign)
        v = -value;
    else
        v = (unsigned)value;
    while (v || tp == tmp)
    {
        i = v % radix;
        v = v / radix;
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    if (string == 0)
        string = (char *)malloc((tp-tmp)+sign+1);
    sp = string;

    if (sign)
        *sp++ = '-';
    while (tp > tmp)
        *sp++ = *--tp;
    *sp = 0;
    return string;
}


//back 返回上一级函数，相当于cd /;
void ftp_back(int control_sockfd)
{
    char sendline[1024],recvline[1024];
    int recvbytes,sendbytes;
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"CDUP");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("cdup send is error !/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(recvbytes<0)
    {
        printf("cdup recv is error !/n");
    }
    if(strncmp(recvline,"250",3)==0)
    {
        printf("请求的文件操作已经成功/n");
    }
}
//stru命令的实现
void ftp_stru(int control_sockfd)
{
    int recvbytes,sendbytes;
    char sendline[1024],recvline[1024];
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"STRU");
    strcat(sendline,"F");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("stru send is error!/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(recvbytes<0)
    {
        printf("stru recv is error!/n");
    }
    if(strncmp(recvline,"200",3)==0)
    {
        f=0;
    }

}

//断点函数的支持
void ftp_rest(int control_sockfd)
{

    int recvbytes,sendbytes;
    char sendline[1024],recvline[1024];
    zeromery(sendline,1024);
    zeromery(recvline,1024);
    strcat(sendline,"REST ");
    strcat(sendline,"500");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
        printf("stru send is error!/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(recvbytes<0)
    {
        printf("stru recv is error!/n");
    }
    if(strncmp(recvline,"350",3)==0)
    {
        printf("%s/n",recvline);
    }
}

//下载的实现函数
int ftp_download(int control_sockfd)
{
    
    int pasv_or_port;// 定义the ftp协议的两种不同工作mode
    int recvbytes,sendbytes;
    char sendline[1024],recvline[1024];
    struct sockaddr_in serv_addr;
    FILE *fd;
    int i,j;
    int data_sockfd;
    //rest
    ftp_rest(control_sockfd);
    //type
    zeromery(recvline,1024);
    zeromery(sendline,1024);
    strcat(sendline,"TYPE ");
    strcat(sendline,"I");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
            printf(" type send is error!/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(strncmp(recvline,"200",3)==0)
    {
        printf("使用二进制传输数据/n");
    }
    else
    {
        printf("type recv is error!/n");
    }

    if(npsupport==1)
    {
        //open the file
        int size;
        char localpathname[60];//预打开的文件路径字符串
        int flags;
        char pathname[60];
        unsigned int mode;
        //用户来选择pasv 或者是 port mode
        char selectdata_mode_tran[1024];
        zeromery(selectdata_mode_tran,1024);
        zeromery(sendline,1024);
        zeromery(recvline,1024);
        pasv_or_port=0;//(默认是pasv模式)
        //pasv mode
        if(pasv_or_port==0)
        {
            strcat(sendline,"PASV");
            strcat(sendline,"/r/n");
            sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
            if(sendbytes<0)
            {
                printf("pasv send is error!/n");
            }
            zeromery(recvline,1024);
            recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
            if(recvbytes<0)
            {
                printf("pasv recv is error!/n");
            }
            if(strncmp(recvline,"227",3)==0)
            {
                char buf[55];
                snprintf(buf,51,">>> %s/n",recvline);
                printf("%s/n",buf);
            }
            else
            {
                printf("pasv recv is error!/n");
            }    
            //处理ftp server 端口
            char *ptr1,*ptr2;
            char num[1024];
            zeromery(num,1024);
            //取低位字节
            ptr1=recvline+strlen(recvline);
            while(*(ptr1)!=')')
            {
                ptr1--;
            }
            ptr2=ptr1;
            while(*(ptr2)!=',')
                ptr2--;
            strncpy(num,ptr2+1,ptr1-ptr2-1);
            i=atoi(num);//将字符串转换成整数
            //取高位字节
            zeromery(num,1024);
            ptr1=ptr2;
            ptr2--;
            while(*(ptr2)!=',')
                ptr2--;
            strncpy(num,ptr2+1,ptr1-ptr2-1);
            j=atoi(num);
            //初始化服务器数据连接时的端口信息
            int data_serviceport;
            data_serviceport=j*256+i;
            data_sockfd=socket(AF_INET,SOCK_STREAM,0);
                
            serv_addr.sin_family=AF_INET;
            serv_addr.sin_addr.s_addr=INADDR_ANY;
            serv_addr.sin_port=htons(data_serviceport);
            if(connect(data_sockfd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))==-1)
            {
                printf("pasv data connect is error!/n");
            }
            printf("remote-file-pathname=");
            scanf("%s",pathname);
            printf("local-file-pathname=");
            scanf("%s",localpathname);
            printf("local:%s remore:%s/n",localpathname,pathname);
            fd=fopen(localpathname,"w+");
            if(fd==NULL)
            {
                printf("cannot open file/n");
                exit(1);
            }

    
            //send the command retr;
            zeromery(sendline,1024);
            zeromery(recvline,1024);
            strcat(sendline,"RETR ");
            strcat(sendline,pathname);
            strcat(sendline,"/r/n");
            printf("%s/n",sendline);
            sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
            if(sendbytes<0)
            {
                printf("retr send is error!/n");
            }
            recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
            if(recvbytes<0)
            {
                printf("retr recv is error!/n");
            }
            if(strncmp(recvline,"400",3)>0)
            {
                printf("return is error!/n");
            }
        }
        //port mode
        /*if(pasv_or_port==1)
        {
            data_sockfd=socket(AF_INET,SOCK_STREAM,0);
            if(data_sockfd<0)
            {
                printf("创建数据端口连接失败！/n");
            }
            serv_addr.sin_family=AF_INET;
            serv_addr.sin_addr.s_addr=INADDR_ANY;
            serv_addr.sin_port=htons(SERV_PORT);
        
            int ret;
            int addrlen;
            ret=sizeof(struct sockaddr_in);
            getsockname(data_sockfd,(SA*)&serv_addr,&ret);
        
            //处理port 后面要带的参数
            char ip[1024];
            int i,j;
            char data[1024];
            zeromery(ip,1024);
            zeromery(data,1024);
            inet_ntop(AF_INET,&(serv_addr.sin_addr),ip,sizeof(ip));
            printf("%s/n",ip);
            i=data_sockfd/256;
            j=data_sockfd%256;

            //将点分十进制的点转换为逗号。
            char *ptr1;
            ptr1=ip;
            while(*(ptr1)!='/0')
            {
                if(*(ptr1)=='.')
                {
                    *(ptr1)=',';
                }
                ptr1++;
            }

            strcat(sendline,"PORT ");
            strcat(sendline,ip);
            strcat(sendline,",");
            strcat(sendline,itoa(i,data,10));
            strcat(sendline,",");
            strcat(sendline,itoa(j,data,10));
            strcat(sendline,"/r/n");
            printf("--->%s/n",sendline);
            sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
            if(sendbytes<0)
            {
                printf("port send is error!/n");
                exit(1);
            }
            recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
            if(strncmp(recvline,"200",3)==0)
            {
                printf("%s/n",recv);
            }
            else
            {
                printf("port recv is error!/n");
            }

        }*/
        //begin to transpotr data
        sleep(1);
        int flag=0;
        char buffer[65536];
recvdata:
        zeromery(buffer,1024);
        recvbytes=recv(data_sockfd,buffer,sizeof(buffer),0);
        if(recvbytes<0)
        {
            close(data_sockfd);
            goto end;
        }
        fwrite(buffer,1,recvbytes,fd);
        zeromery(recvline,1024);
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(flag==0)
        {
            if(strncmp(recvline,"226",3)!=0)
            {
                    flag=1;
                    goto recvdata;
            }
        }
end:
        if(flag!=1)
        {
            zeromery(recvline,1024);
            shutdown(data_sockfd,SHUT_WR);
            close(data_sockfd);
        }
        close(data_sockfd);
/*    int err;
    char buffer[65535];
    err=read(data_sockfd,buffer,sizeof(buffer));
    sleep(5);
    fwrite(buffer,1,err,fd);
    sleep(5);
    zeromery(recvline,1024);
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(strncmp(recvline,"226",3)==0)
    {
    end:    printf("226 transfer complete/n");
        close(data_sockfd);
    }*/
    return 0;
    }
}


//up 函数
int ftp_up(int control_sockfd)
{
        
    int pasv_or_port;// 定义the ftp协议的两种不同工作mode
    int recvbytes,sendbytes;
    char sendline[1024],recvline[1024];
    struct sockaddr_in serv_addr;
    FILE *fd;
    int i,j;
    int data_sockfd;
    //type
    zeromery(recvline,1024);
    zeromery(sendline,1024);
    strcat(sendline,"TYPE ");
    strcat(sendline,"I");
    strcat(sendline,"/r/n");
    sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
    if(sendbytes<0)
    {
            printf(" type send is error!/n");
    }
    recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
    if(strncmp(recvline,"200",3)==0)
    {
        printf("使用二进制传输数据/n");
    }
    else
    {
        printf("type recv is error!/n");
    }

    if(npsupport==1)
    {
        //open the file
        int size;
        char localpathname[60];//预打开的文件路径字符串
        int flags;
        char pathname[60];
        unsigned int mode;
        //用户来选择pasv 或者是 port mode
        char selectdata_mode_tran[1024];
        zeromery(selectdata_mode_tran,1024);
        zeromery(sendline,1024);
        zeromery(recvline,1024);
        pasv_or_port=0;//(默认是pasv模式)
        //pasv mode
        if(pasv_or_port==0)
        {
            strcat(sendline,"PASV");
            strcat(sendline,"/r/n");
            sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
            if(sendbytes<0)
            {
                printf("pasv send is error!/n");
            }
            recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
            if(recvbytes<0)
            {
                printf("pasv recv is error!/n");
            }
            if(strncmp(recvline,"227",3)==0)
            {
                char buf[55];
                snprintf(buf,51,">>> %s/n",recvline);
                printf("%s/n",buf);
            }
            else
            {
                printf("pasv recv is error!/n");
            }    
            //处理ftp server 端口
            char *ptr1,*ptr2;
            char num[1024];
            zeromery(num,1024);
            //取低位字节
            ptr1=recvline+strlen(recvline);
            while(*(ptr1)!=')')
            {
                ptr1--;
            }
            ptr2=ptr1;
            while(*(ptr2)!=',')
                ptr2--;
            strncpy(num,ptr2+1,ptr1-ptr2-1);
            i=atoi(num);//将字符串转换成整数
            //取高位字节
            zeromery(num,1024);
            ptr1=ptr2;
            ptr2--;
            while(*(ptr2)!=',')
                ptr2--;
            strncpy(num,ptr2+1,ptr1-ptr2-1);
            j=atoi(num);
            //初始化服务器数据连接时的端口信息
            int data_serviceport;
            data_serviceport=j*256+i;
            data_sockfd=socket(AF_INET,SOCK_STREAM,0);
                
            serv_addr.sin_family=AF_INET;
            serv_addr.sin_addr.s_addr=INADDR_ANY;
            serv_addr.sin_port=htons(data_serviceport);
            if(connect(data_sockfd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))==-1)
            {
                printf("pasv data connect is error!/n");
            }
                printf("local-file-pathname=");
                scanf("%s",pathname);
                printf("remote-file-pathname=");
                scanf("%s",localpathname);
                printf("local:%s remore:%s/n",localpathname,pathname);
                fd=fopen(pathname,"r");
                if(fd==NULL)
                {
                    printf("cannot open file,请重新输入！/n");
                }

            //send the command retr;
            zeromery(sendline,1024);
            zeromery(recvline,1024);
            strcat(sendline,"STOR ");
            strcat(sendline,localpathname);
            strcat(sendline,"/r/n");
            printf("%s/n",sendline);
            sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
            if(sendbytes<0)
            {
                printf("stor send is error!/n");
            }
            recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
            if(recvbytes<0)
            {
                printf("retr recv is error!/n");
            }
            if(strncmp(recvline,"150",3)==0)
            {
                char buf[55];
                snprintf(buf,25,">>> %s/n",recvline);
                printf("%s/n",buf);
            }
        }
        //port mode
        /*if(pasv_or_port==1)
        {
            data_sockfd=socket(AF_INET,SOCK_STREAM,0);
            if(data_sockfd<0)
            {
                printf("创建数据端口连接失败！/n");
            }
            serv_addr.sin_family=AF_INET;
            serv_addr.sin_addr.s_addr=INADDR_ANY;
            serv_addr.sin_port=htons(SERV_PORT);
        
            int ret;
            int addrlen;
            ret=sizeof(struct sockaddr_in);
            getsockname(data_sockfd,(SA*)&serv_addr,&ret);
        
            //处理port 后面要带的参数
            char ip[1024];
            int i,j;
            char data[1024];
            zeromery(ip,1024);
            zeromery(data,1024);
            inet_ntop(AF_INET,&(serv_addr.sin_addr),ip,sizeof(ip));
            printf("%s/n",ip);
            i=data_sockfd/256;
            j=data_sockfd%256;

            //将点分十进制的点转换为逗号。
            char *ptr1;
            ptr1=ip;
            while(*(ptr1)!='/0')
            {
                if(*(ptr1)=='.')
                {
                    *(ptr1)=',';
                }
                ptr1++;
            }

            strcat(sendline,"PORT ");
            strcat(sendline,ip);
            strcat(sendline,",");
            strcat(sendline,itoa(i,data,10));
            strcat(sendline,",");
            strcat(sendline,itoa(j,data,10));
            strcat(sendline,"/r/n");
            printf("--->%s/n",sendline);
            sendbytes=send(control_sockfd,sendline,strlen(sendline),0);
            if(sendbytes<0)
            {
                printf("port send is error!/n");
                exit(1);
            }
            recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
            if(strncmp(recvline,"200",3)==0)
            {
                printf("%s/n",recv);
            }
            else
            {
                printf("port recv is error!/n");
            }

        }*/
        //begin to transpotr data
    while(!feof(fd))
    {        
        char buffer[65536];
        zeromery(buffer,sizeof(buffer));
        int size;
        size=fread(buffer,1,sizeof(buffer),fd);
        if(ferror(fd))
        {
            printf("read file data is error!/n");
            break;
        }
        else
        {
            zeromery(sendline,1024);
            sendbytes=send(data_sockfd,buffer,size,0);
            printf("传输了 %d 个字节/n",sendbytes);
        }
        close(data_sockfd);
        recvbytes=recv(control_sockfd,recvline,sizeof(recvline),0);
        if(strncmp(recvline,"226",3)==0)
        {
            printf("226 transfer complete");
            break;
        }
    }
    return 0;
    }
}