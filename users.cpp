#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

struct user{
    pid_t pid;//user information
    int ID;//get from center 
};

struct msg
{
    int type;
    user source;
    user dist;
    char data[100];
};


int main(){
    int ret,fd1,fd2;//fd1:write to center;fd2:read from center
    char selffifo[20];
    char data[100];
    fd1=open("FIFO/shared",O_WRONLY);
    if(fd1==-1){
        printf("open shared error\n");
        return -1;
    }
    pid_t pid;
    pid=getpid();
    sprintf(selffifo,"FIFO/fifo%d",pid);
    unlink(selffifo);
    ret=mkfifo(selffifo,0777);
    if(ret==-1){
        printf("mkfifo error\n");
        return -1;
    }
    user myself;myself.pid=pid;
    msg m1;m1.type=0;m1.source=myself;
    ret=write(fd1,&m1,sizeof(m1));
    if(ret==-1){
        printf("write error\n");
        return -1;
    }
    fd2=open(selffifo,O_RDONLY);
    if(fd2==-1){
        printf("make selffifo error\n");
        return -1;
    }
    msg m2;
    ret=read(fd2,&m2,sizeof(m2));
    if(ret==-1){
        printf("read error\n");
        return -1;
    }
    else printf("get linked to center\n");
    myself.ID=m2.source.ID;
    m1.source=myself;
    printf("And your ID is %d\n",myself.ID);

    int status;//change read block
    status=fcntl(fd2,F_GETFL,0);
    status|=O_NONBLOCK;
	fcntl(fd2,F_SETFL,status);

    status=fcntl(STDIN_FILENO,F_GETFL,0);
    status|=O_NONBLOCK;
	fcntl(STDIN_FILENO,F_SETFL,status);

    while(1){
	    void *p=gets(data);
		if(p!=NULL){
			if(data[0]=='q'){
                m1.type=2;
                ret=write(fd1,&m1,sizeof(m1));
                break;
            }
            else if(data[0]>='0'&&data[0]<='9'&&data[1]==':'){
                int distID=data[0]-'0';
                user distu;distu.ID=distID;
                m1.dist=distu;
                m1.type=1;
                memcpy(m1.data,data,sizeof(data));
                ret=write(fd1,&m1,sizeof(m1));
                if(ret==-1){
                    printf("write error\n");
                    return -1;
                }
            }
            else{
                printf("Invalid input\n");
            }
		}
        int size=read(fd2,&m2,sizeof(m2));
        if(size>0){
            if(m2.type==1){
                printf("[Receive]");
                m2.data[0]=m2.source.ID+'0';
                printf("%s\n",m2.data);
            }else{
                printf("[failed]ID %d is not online\n",m2.dist.ID);
            }
        }
	}
}