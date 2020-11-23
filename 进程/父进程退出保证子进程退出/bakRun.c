#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<signal.h>
#include <sys/prctl.h>
#include<sys/wait.h>
/*
prctl(PR_SET_PDEATHSIG,SIGHUP);
设置该选项，在父进程死后，子进程会收到SIGHUP信号，子进程因此也会退出，解决了，父进程异常退出，而子进程来不及回收资源的问题。
*/
int mysystem(const char *cmd)
{

    pid_t pid;
    int status;

    if((pid=fork()) < 0)
    {
        printf("fork error\n");
    }
    else if(pid >0)
    {
        waitpid(pid,&status,0);
        printf("parent\n");

    }
    else
    {
        printf("receive msg\n");
        prctl(PR_SET_PDEATHSIG,SIGHUP);
        execl("/bin/sh","sh","-c","./bakRun.sh",(char*)0);
        printf("exec cmd\n");
        return -1;
    }
    return 0;
}
int main(int argc ,char* argv[])
{
    mysystem("./backRun.sh");
    while(1);
    return 0;
}