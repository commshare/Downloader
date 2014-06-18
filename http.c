#ifndef STRUCT_H_  
#define STRUCT_H_  

#endif /* STRUCT_H_ */  
#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netdb.h>  
#include <netinet/in.h>  
//create a struct to save the file part infomation  
struct filepart  
{  
int partnum;  
char pathname[128];  
long int lenth;  
long int datastar;  
long int dataend;  
struct filepart *next;  
};  
typedef struct filepart FilePart;  
struct hostent *host;  
char filepath[2048];  //save the filepath on the server  
char hostname[1024];  //save the name of the server  
char filename[256];  //save the name of the file  
char localpath[512];  
int port,part;  
long int FileLen;  //the file lenth  
void getname( char * ulr); //解析url，并从中取得主机名，ip地址 、文件路径以及端口  
void filepart( long int datalen,int part ); //对文件进行分块  
void *getfile( FilePart *pointer); //下载线程函数  
long int getFileLen( struct hostent *host  ); //获取http包头，分析文件大小  
void createthread( FilePart * head, int num );  //创建线程函数 参数为链表头指针和分块数目  
int mergefile( FilePart * head);  
int main(void)  
{  
char ulr[128];  
printf("please input the website:\n");  
scanf("%s",ulr);  
printf("the website is :%s\n",ulr);  
printf("input the direct to save file:\n");  
scanf("%s",localpath);  
getname( ulr );             //get the ipaddress of the host  
// getfile( host );  
FileLen = getFileLen( host );     //get the lenth of the file  
filepart( FileLen, 3);  

puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */  
return EXIT_SUCCESS;  
}  



long int getFileLen( struct hostent *host  )  
{  
struct sockaddr_in sock_serv;  
int sockfd;  
char Request[1024];  
long int filelen;  
int recvbytes;  
char *buff ,*buffp,*Plen;  
buff = (char *)malloc( 1024 * sizeof(char));  
memset( buff, 0, sizeof(buff));  
sockfd = socket( AF_INET, SOCK_STREAM,0);  
// printf("sockfd = %d\n",sockfd);  
if(sockfd < 0)  
{  
  perror("socket");  
  exit(1);  
}  
memset( &sock_serv, 0, sizeof( sock_serv));  
sock_serv.sin_family = AF_INET;  
sock_serv.sin_port = htons(port);  
sock_serv.sin_addr = *((struct in_addr *)host -> h_addr);  
if(0>connect(sockfd,(struct sockaddr *)&sock_serv, sizeof(struct sockaddr)))  
{  
  perror("connect");  
  exit(1);  
}  
// printf("connect success!\n");  
printf("file path is:%s\n",filepath);  
sprintf(Request, "GET %s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: en-us\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\nHost: %s:%d\r\nConnection: Keep-Alive\r\n\r\n", filepath, hostname, port);  
// printf("%s",Request);  

//send the http protocl packet  
if( 0> send(sockfd, Request, strlen(Request), 0))  
{  
  perror("send");  
  exit(1);  
}  
while( 1 )  
{  
  recvbytes = recv(sockfd, buff, 1024,0);  
//  printf("recvbytes = %d",recvbytes);  
  if(0 > recvbytes )  
  {  
   perror("recv");  
   exit(1);  
  }  
  Plen = strstr(buff, "Length:");  
  if( NULL != Plen )  
  {  
   Plen = strchr( Plen, ':');  
   Plen++;  
   filelen = atol( Plen );  
   printf("the file data is %ld\n",filelen);  
  }  
  buffp = strstr(buff, "\r\n\r\n");  
//  printf("\n%s",buff);  
  if( NULL != buffp )  
   break;  
}  
free( buff ); buff = NULL;  
close(sockfd);  
return filelen;  
}  

void getname( char * ulr)  
{  
char head[10];          //save the"http://" or "https://"  
char hostname[1024];  
char *pointer;  
char *pointer2;  
char **addrs;  
pointer = strstr(ulr,"//");   //move the pointer to the "//" of the url  
pointer += 2;          //move the pointer to the right of "//" of the url  
strncpy(head, ulr, pointer - ulr);//copy "  
http://" to head  
// printf("head = %s\n",head);  
pointer2 = strchr( pointer,':'); //find the port of the server  
if( pointer2 == NULL)       //if there is no port  
{  
  char *temp = strchr(pointer,'/');       //move pointer to the first '/'  
  port = 80;  
  if( temp != NULL )  
  {  
   pointer2 = temp++;          //save a pointer pointo the right of the first '/'  
   strncpy(hostname, pointer, pointer2 - pointer);  //save the host name  
   strcpy(filepath, pointer2);            //the right of the first '/' are the filepath and name, then save!  
   printf("filepath:%s\n",filepath);  
   while( NULL != temp )  
   {  
    temp = strchr( temp,'/');            //find the right '/'  
    if( NULL != temp )  
    {  
     pointer2 = temp;  
     temp++;  
    }  
   }  
   strcpy(filename, ++pointer2);           //the right of the right '/' is the file name!  
   printf("filename is: %s\n",filename);  
          //between the first left '/' and the right '/' is the file path  
   printf("save to %s\n",localpath);  
  /* if( *temp == 0 )  
   {  
    strcpy(filepath,"/index.html");  
    printf("filepath:%s\n",filepath);  
   }  
  */  
  }  
  else  
  {  
   strcpy(hostname,pointer);  
   strcpy(filepath,"/index.html");  
   printf("filepath = %s\n",filepath);  
  }  
}  
else  
  {  
   strncpy(hostname, pointer, pointer2 - pointer);  
   port = atoi(++pointer2);  
   printf("port = %d\n",port);  
  }  
printf("hostname = %s\n",hostname);  
host = gethostbyname(hostname);  
if( host < 0)  
{  
  perror("gethostbyname");  
  exit(1);  
}  
addrs = host -> h_addr_list;  
printf("%s",inet_ntoa(*(struct in_addr *)*addrs));  
printf("\n");  
}  

void filepart( long int datalen,int part )      //move the file lenth and the number of cutting parts  
{  
long int partlen = datalen / part;  
long int end = datalen - 1;  
int i;  
// printf("partlen = %ld\nend = %ld\n",partlen,end);  
FilePart *head ,*present;  
head = (FilePart *)malloc(sizeof(FilePart));    //create the head of the link  
head -> partnum =  1;  
head -> datastar = 0;  
head -> dataend = partlen;  
head -> lenth = partlen;  
sprintf(head -> pathname,"%s/part%d",localpath,1);  
if(part == 1)  
  head -> dataend = end;  
present = head;  
for( i = 0; i < part - 1; i++)  
{  
  present ->next = (FilePart *)malloc(sizeof(FilePart));  
  memset(present->next, 0, sizeof(FilePart));  
  present ->next -> partnum = i+2;  
  present ->next -> datastar = present -> dataend +1;  
  if( i == (part-2))  
   {  
    present ->next -> dataend = end;  
    present ->next -> lenth = datalen - (part-1)*partlen;  
   }  
  else  
   {  
    present ->next -> dataend = partlen * (i+2);  
    present ->next -> lenth = partlen;  
   }  
  sprintf(present ->next->pathname,"%s/part%d",localpath,(i+2));  
  present = present->next;  
}  
createthread( head, part );  
mergefile( head );  
}  
void createthread( FilePart * head, int num )  
{  
// printf("start to create thread\n");  
int i = 1 ,res;  
long int pres[num];  
pthread_t thread[num];  
FilePart *present = head;  
for(i = 0; i < num; i++)  
{  
  res = pthread_create(&thread, NULL, getfile, present);  
  if(res != 0)  
  {  
   perror("thread creation failed");  
   exit(1);  
  }  
//  printf("start to create thread %d\n",i+1);  
  present = present -> next;  
}  
long int all = 0;  
for( i = 0; i < num; i++)  
{  
  res = pthread_join(thread,&pres);  
  if( res != 0)  
  {  
   perror("pthread join");  
   exit(1);  
  }  
  printf("thread %d finished!\n",i+1);  
  all += pres;  
}  
printf("all download:%ld!\n",all);  
}  
void *getfile( FilePart *pointer)  
{  
char buff[1024],request[2048];  
struct sockaddr_in sock_serv;  
FILE *fp,* fplog;  
int recvbytes,psockfd;  
long int download = 0;  
psockfd = socket(AF_INET, SOCK_STREAM, 0);  
if( psockfd < 0)  
{  
  perror("socket");  
  pthread_exit(1);  
}  
// printf("psockfd = %d\n",psockfd);  
memset( &sock_serv, 0, sizeof(sock_serv));  
sock_serv.sin_family = AF_INET;  
sock_serv.sin_port = htons(port);  
sock_serv.sin_addr = *((struct in_addr *)host -> h_addr);  
if( ( connect( psockfd, (struct sockaddr *)&sock_serv, sizeof(struct sockaddr) ) )< 0 )  
{  
  perror("connect");  
  pthread_exit(1);  
}  
// printf("connect success!\n");  
// printf("filepath is :%s\n",filepath);  
sprintf(request, "GET %s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: en-us\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\nHost: %s:%d\r\nRange:bytes=%d-%d\r\nConnection: Keep-Alive\r\n\r\n",filepath, hostname, port,pointer ->datastar,pointer ->dataend);  
// printf("ready to send:\n %s",request);  
int sends = send( psockfd, request, strlen(request), 0);  
if( sends < 0)  
{  
  perror("send");  
  exit(1);  
}  
char filelog[32];  
sprintf(filelog,"/tmp/down/part%dlog",pointer-> partnum);  
fplog = fopen( filelog,"w+");  
fp = fopen( pointer->pathname,"w+");  
if(fp < 0 )  
{  
  perror("open ");  
  pthread_exit(1);  
}  
int  in = 0;  
do  
{  
  memset(buff, 0, sizeof(buff));  
  recvbytes = recv(psockfd,buff,1024,0);  
//  printf(" recv %d bytes!\n",recvbytes);  
  if( recvbytes == 0)  
   break;  
//  printf("recvbytes = %d\n",recvbytes);  
  if( recvbytes < 0)  
  {  
   perror("recv");  
   pthread_exit(1);  
  }  
  char *p;  
  p = strstr(buff,"\r\n\r\n");  

/* when recv the file stream,if there don't include http protocol head,skip,or discaerd the head,and save the other to the file */  
  if( !in )  
  {  
   if(  p != NULL )  
   {  
    p += 4;  
    in = fwrite(buff, 1, ( p - buff), fplog ); //discard the http protocol head  
//    printf("\nin = %d\n (p - buff)=%d\n",in,(p-buff));  
    recvbytes = recvbytes - ( p - buff );  
//    printf("\n recvbytes = %d\n",recvbytes);  
    in = fwrite( p, 1, recvbytes, fp);  
//    printf("in = %d\n",in);  
//    printf("%s",buff);  
    download += in;  
   }  
   else  
   {  
//    printf("%s",buff);  
//    fwrite(buff, 1, recvbytes, fp );  
   }  
  }  
  else  
  {  
   in = fwrite(buff,1, recvbytes, fp);  
//   printf("in = %d\n",in);  
   download += in;  
  }  
  //printf("\n");  
}while( recvbytes > 0 );  
printf("%d have downloaded!\n",download);  
fclose(fp);  
fclose(fplog);  
close(psockfd);  
pthread_exit(download);  
}  
/*merge all parts to one file and remove the temp file rename the file*/  
int mergefile( FilePart * head)  
{  
FilePart *present ,*release;  
char *buf = (char *)malloc(1024*sizeof(char));  
FILE *pfread,*pfwrite;  
int readnum,writenum;  
pfwrite = fopen(head->pathname,"ab");  //open the first part as append mode.  
if( pfwrite < 0)  
{  
  printf("open file");  
  exit(1);  
}  
present = head->next;  
while( present )  
{  
  pfread = fopen( present->pathname,"rb");  //open temp part's data  
  if(pfread < 0)  
  {  
   printf("fopen");  
   exit(1);  
  }  
  else  
   printf("open success!\n");  
  while( !feof(pfread))  
  {  
   readnum = fread( buf, sizeof(char), 1024, pfread); //read data from temp part's  
   writenum = fwrite( buf, sizeof(char), readnum, pfwrite);  //write to the first part  
//   printf("%ld have written!\n",writenum);  
  }  
  printf(" read %d part\n", present->partnum);  
  fclose(pfread);  //close the temp file  
  remove(present->pathname); //remove the temp file which have already read  
  release = present;  
  present = present->next;  
  free(release); //release the link  
}  
free(buf);buf=NULL;  
fclose(pfwrite);  
sprintf(localpath,"%s/%s",localpath,filename);  
if( 0 > (rename( head->pathname,localpath)))  
  perror("rename");  
free(head); //free the head of the link  
return 0;  
}  
