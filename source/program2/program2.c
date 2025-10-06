#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/kmod.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/binfmts.h>

MODULE_LICENSE("GPL");

// 声明需要使用的内核函数 - 使用正确的函数签名
extern struct filename *getname(const char __user *filename);

static struct task_struct *task;


//implement fork function
int my_fork(void *argc){
	pid_t child_pid;
	int status;
	long retval;
	struct kernel_clone_args args;
	const char *const argv[] = { "/tmp/test", NULL };
	const char *const envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/bin", NULL };
	
	//set default sigaction for current process
	int i;
	struct k_sigaction *k_action = &current->sighand->action[0];
	for(i=0;i<_NSIG;i++){
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}
	
	/* fork a process using kernel_clone */
	memset(&args, 0, sizeof(args));
	args.flags = SIGCHLD;
	args.exit_signal = SIGCHLD;
	
	child_pid = kernel_clone(&args);
	
	if (child_pid < 0) {
		printk("[program2] : Failed to fork process\n");
		return child_pid;
	}
	
	if (child_pid == 0) {
		/* This is the child process */
		printk("[program2] : child process\n");
		
		/* execute a test program in child process */
		retval = kernel_execve("/tmp/test", argv, envp);
		if (retval < 0) {
			printk("[program2] : Failed to execute test program: %ld\n", retval);
		}
		
		// execve成功后不会返回到这里，除非失败
		return retval;
	} else {
		/* This is the parent process */
		printk("[program2] : The child process has pid = %d\n", child_pid);
		printk("[program2] : This is the parent process, pid = %d\n", current->pid);
		
		/* wait until child process terminates */
		retval = kernel_wait4(child_pid, NULL, 0, NULL);
		
		if (retval < 0) {
			printk("[program2] : wait4 failed: %ld\n", retval);
		} else {
			// 获取退出状态
			status = (int)retval;
			// 检查进程退出状态
			if (status & 0x7f) {
				// 被信号终止
				int sig = status & 0x7f;
				printk("[program2] : get SIGTERM signal\n");
				printk("[program2] : child process terminated\n");
				printk("[program2] : The return signal is %d\n", sig);
			} else {
				// 正常退出
				int exit_code = (status >> 8) & 0xff;
				printk("[program2] : child process exited with code %d\n", exit_code);
			}
		}
	}
	
	return 0;
}

static int __init program2_init(void){

	printk("[program2] : Module_init\n");
	
	/* write your code here */
	
	/* create a kernel thread to run my_fork */
	printk("[program2] : module_init create kthread start\n");
	
	task = kthread_run(my_fork, NULL, "my_fork_thread");
	
	if (IS_ERR(task)) {
		printk("[program2] : Failed to create kernel thread\n");
		return PTR_ERR(task);
	}
	
	printk("[program2] : module_init kthread start\n");
	
	return 0;
}

static void __exit program2_exit(void){
	printk("[program2] : Module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);
