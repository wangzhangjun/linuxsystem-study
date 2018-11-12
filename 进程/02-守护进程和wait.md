# linux系统编程-守护进程和wait

@(linux系统编程)[进程]

## 守护进程和wait,waitpid
#### 1.守护进程
* 什么是守护进程

	* 1.守护进程是在后台运行不受控端控制的进程，通常情况下守护进程在系统启动时自动运行

	* 2.守护进程的名称通常以d结尾，比如sshd、xinetd、crond等

* 创建守护进程
	* 1.调用fork(),创建新进程，它会是将来的守护进程
	* 2.在父进程中调用exit，保证子进程不是进程组组
	* 3.调用setsid创建新的会话期
	* 4.将当前目录改为根目录 （如果把当前目录作为守护进程的目录，当前目录不能被卸载，它作为守护进程的工作目录了。）
	* 5.将标准输入、标准输出、标准错误重定向到/dev/null
* man setsid
Setsid创建一个新的会话；调用者进程会是这个会话期唯一的一个进程，是唯一组的组长；调用者进程id是组id，也是会话期的id。不能用进程组组长去调用setsid函数

```
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <errno.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>
//守护进程
int main()
{
	pid_t pid;
	pid = fork();
	if (pid == -1)
	{
		perror("fork err");
		exit (0);
	}

	if (pid > 0)
	{
		exit (0);
	}
	pid = setsid();
	printf("pid :%d\n",getpid());
	if (pid == -1)
	{
		perror("fork err");
		exit (0);
	}
	//把当前目录改为守护进程的工作目录，无法卸载
	chdir("/");
	int i;
	for(i=0; i<3; i++)
	{
		close(i);
	}

	open("/dev/null",O_RDWR);
	dup(0);
	dup(0);
}
```
#### 2.父进程wait和waitpid
1.wait和waitpid出现的原因
>* 当子进程退出的时候，内核会向父进程发送SIGCHLD信号，子进程的退出是个异步事件（子进程可以在父进程运行的任何时刻终止）
>*  子进程退出时，内核将子进程置为僵尸状态，这个进程称为僵尸进程，它只保留最小的一些内核数据结构，以便父进程查询子进程的退出状态。

2.父进程查询子进程的退出状态可以用wait/waitpid函数
wait
函数功能：当我们用fork启动一个进程时，子进程就有了自己的生命，并将独立地运行。有时，我们需要知道某个子进程是否已经结束了，我们可以通过wait安排父进程在子进程结束之后。
函数原型：pid_t wait(int *status)  ：status可以获得你等待的子进程的信息。返回子进程的pid。
wait系统调用会使父进程暂停执行，直到它的一个子进程结束为止。
返回的是子进程的PID，它通常是结束的子进程
状态信息允许父进程判定子进程的退出状态，即从子进程的main函数返回的值或子进程中exit语句的退出码。
如果status不是一个空指针，状态信息将被写入它指向的位置

Wait获取status后检测处理
```
宏定义	描述
WIFEXITED(status)	如果子进程正常结束，返回一个非零值
WEXITSTATUS(status)	如果WIFEXITED非零，返回子进程退出码
WIFSIGNALED(status)	子进程因为捕获信号而终止，返回非零值
WTERMSIG(status)	如果WIFSIGNALED非零，返回信号代码
WIFSTOPPED(status)	如果子进程被暂停，返回一个非零值
WSTOPSIG(status)	如果WIFSTOPPED非零，返回一个信号代码
```
