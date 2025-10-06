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
#include <linux/delay.h>

MODULE_LICENSE("GPL");

// 声明需要使用的内核函数 - 使用正确的函数签名
extern struct filename *getname(const char __user *filename);

static struct task_struct *task;

// 信号名称映射表
static const char *signal_names[] = {
    "UNKNOWN",   // 0: No signal 0
    "SIGHUP",    // 1: Hangup
    "SIGINT",    // 2: Interrupt from keyboard
    "SIGQUIT",   // 3: Quit from keyboard
    "SIGILL",    // 4: Illegal instruction
    "SIGTRAP",   // 5: Trace/breakpoint trap
    "SIGABRT",   // 6: Abort signal
    "SIGBUS",    // 7: Bus error
    "SIGFPE",    // 8: Floating-point exception
    "SIGKILL",   // 9: Kill signal
    "SIGUSR1",   // 10: User-defined signal 1
    "SIGSEGV",   // 11: Segmentation fault
    "SIGUSR2",   // 12: User-defined signal 2
    "SIGPIPE",   // 13: Broken pipe
    "SIGALRM",   // 14: Timer signal from alarm
    "SIGTERM",   // 15: Termination signal
    "SIGSTKFLT", // 16: Stack fault
    "SIGCHLD",   // 17: Child stopped or terminated
    "SIGCONT",   // 18: Continue if stopped
    "SIGSTOP",   // 19: Stop process
    "SIGTSTP",   // 20: Stop typed at terminal
    "SIGTTIN",   // 21: Terminal input for background process
    "SIGTTOU",   // 22: Terminal output for background process
    "SIGURG",    // 23: Urgent condition on socket
    "SIGXCPU",   // 24: CPU time limit exceeded
    "SIGXFSZ",   // 25: File size limit exceeded
    "SIGVTALRM", // 26: Virtual alarm clock
    "SIGPROF",   // 27: Profiling timer expired
    "SIGWINCH",  // 28: Window resize signal
    "SIGIO",     // 29: I/O now possible
    "SIGPWR",    // 30: Power failure
    "SIGSYS"     // 31: Bad system call
};

// 获取信号名称的辅助函数
static const char* get_signal_name(int sig) {
    if (sig >= 0 && sig <= 31) {
        return signal_names[sig];
    }
    return "UNKNOWN";
}


//implement fork function
int my_fork(void *argc){
	pid_t child_pid;
	int status = 0;
	int __user *status_ptr;
	long retval;
	struct kernel_clone_args args;
	const char *const argv[] = { "/tmp/test", NULL };
	const char *const envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/bin", NULL };
	int termsig, coredump, sig, exit_code;
	const char *sig_name;
	
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
		
		/* wait for child process */
		// 在内核空间中，我们需要使用 __user 指针
		// 最简单的方法是声明一个用户空间指针并使用它
		status_ptr = (int __user *)&status;
		retval = kernel_wait4(child_pid, status_ptr, 0, NULL);
		if (retval < 0) {
			printk("[program2] : kernel_wait4 failed with error %ld\n", retval);
			return retval;
		}
		
		// 添加原始状态值的调试信息
		printk("[program2] : Raw status value: 0x%x (%d)\n", status, status);
		
		// 检查进程退出状态
		// WTERMSIG(status) 等同于 (status & 0x7f)
		// WIFEXITED(status) 检查是否正常退出
		// WIFSIGNALED(status) 检查是否被信号终止
		
		termsig = status & 0x7f;  // 终止信号
		coredump = (status & 0x80) ? 1 : 0;  // core dump 标志
		
		printk("[program2] : termsig = %d, coredump = %d\n", termsig, coredump);
		
		// 如果 termsig 为 0，说明正常退出
		// 如果 termsig 为 0x7f (127)，说明进程停止而非终止
		if (termsig == 0) {
			// 正常退出
			exit_code = (status >> 8) & 0xff;
			printk("[program2] : child process exited normally\n");
			printk("[program2] : The exit code is %d\n", exit_code);
		} else if (termsig == 0x7f) {
			// 进程停止（不是终止）
			printk("[program2] : child process stopped\n");
		} else {
			// 被信号终止
			sig = termsig;
			sig_name = get_signal_name(sig);
			printk("[program2] : get %s signal\n", sig_name);
			printk("[program2] : child process terminated by signal\n");
			printk("[program2] : The return signal is %d\n", sig);
			if (coredump) {
				printk("[program2] : Core dumped\n");
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

