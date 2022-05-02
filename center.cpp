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
    int type;//0:connect request
            //1:inter users comunicate
            //2:disconnect note
            //3:failed sending note
    user source;
    user dist;
    char data[100];
};


int main(){
    int fd,ret;
    int ID_Alocator=1;//ID Alocator
    int fds[10];//users fd list,0 refers to offline or unexit
    memset(fds,0,sizeof(fds));
    char data[100];
    unlink("FIFO/shared");
    ret=mkfifo("FIFO/shared",0777);
    if(ret==-1){
        printf("Makefifo error\n");
        return -1;
    }
    fd=open("FIFO/shared",O_RDONLY);
    if(fd==-1){
        printf("Open shared error\n");
        return -1;
    }

    msg m1;
    while(1){
        ret=read(fd,&m1,sizeof(m1));
        if(ret==-1){
            printf("Read error\n");
            return -1;
        }else if(ret==0){
            printf("Users all offline,quit\n");
            unlink("FIFO/shared");
            return 0;
        }
        //deal with user request
        if(m1.type==0){//connect
            pid_t pidon=m1.source.pid;
            printf("Connect request from process %d\n",pidon);
            char userfifo[20];
            sprintf(userfifo,"FIFO/fifo%d",pidon);
            fds[ID_Alocator]=open(userfifo,O_WRONLY);
            m1.source.ID=ID_Alocator;
            printf("ID %d allocated\n",ID_Alocator);
            write(fds[ID_Alocator],&m1,sizeof(m1));
            ID_Alocator++;
        }else if(m1.type==1){//comunicate
            int sourceID=m1.source.ID;
            int distID=m1.dist.ID;
            printf("ID %d -> ID %d : ",sourceID,distID);
            if(fds[distID]==0){
                m1.type=3;
                ret=write(fds[sourceID],&m1,sizeof(m1));
                if(ret<0)printf("Write error\n");
                printf("ID %d not exit or online\n",distID);
            }
            else{
                ret=write(fds[distID],&m1,sizeof(m1));
                if(ret<0)printf("Write error\n");
                else printf("Success\n");
            }
        }else if(m1.type==2){//disconnect
            int IDoff=m1.source.ID;
            pid_t pidoff=m1.source.pid;
            printf("ID %d offline\n",IDoff);
            fds[IDoff]=0;
            char userfifo[20];
            sprintf(userfifo,"FIFO/fifo%d",pidoff);
            unlink(userfifo);
        }else{
            printf("Unclear protocol\n");
        }
    }
}
