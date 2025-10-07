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
#include <linux/wait.h>
#include <linux/signal.h>

MODULE_LICENSE("GPL");

struct wait_opts{
	enum pid_type wo_type;
	int wo_flags;
	struct pid *wo_pid;
	struct waitid_info *wo_info;
	int __user *wo_stat;
	struct rusage __user *wo_rusage;
	wait_queue_entry_t child_wait;
	int notask_error;
};

static struct task_struct *task;

/* Modern kernel API functions */
extern pid_t kernel_clone(struct kernel_clone_args *kargs);
extern int kernel_execve(const char *filename, const char *const *argv, const char *const *envp);
extern long kernel_wait4(pid_t pid, int __user *stat_addr, int options, struct rusage *ru);
extern int kernel_wait(pid_t pid, int *stat);

char* processTerminatedSignal[] = {
	"SIGHUP",      "SIGINT",       "SIGQUIT",      "SIGILL",      "SIGTRAP",
	"SIGABRT",     "SIGBUS",        "SIGFPE",       "SIGKILL",     NULL,
    "SIGSEGV",         NULL,       "SIGPIPE",     "SIGALRM",    "SIGTERM"
};

int my_WEXITSTATUS(int status){
	return ((status & 0xff00)>>8);
}

int my_WTERMSIG(int status){
	return (status & 0x7f);
}

int my_WSTOPSIG(int status){
	return (my_WEXITSTATUS(status));
}

int my_WIFEXITED(int status){
	return (my_WTERMSIG(status)==0);
}

signed char my_WIFSIGNALED(int status){
	return (((signed char) (((status & 0x7f) + 1) >> 1) ) > 0);
}

int my_WIFSTOPPED(int status){
	return (((status) & 0xff) == 0x7f);
}


//execute the test program
int my_exec(void){
	int result;
	const char path[] = "/tmp/test";
	const char *const argv[] = {path, NULL, NULL};
	const char *const envp[] = {"HOME=/", "PATH=/sbin:/user/sbin:/bin:/usr/bin", NULL};

	// printk("[program2] : child process\n");
	result = kernel_execve(path, argv, envp);

	if(!result){
		return 0;
	}

	do_exit(result);
}

//wait for child process using kernel_wait
int my_wait(pid_t pid){
	int status;
	int ret;
	
	/* Use kernel_wait which accepts kernel space pointer */
	ret = kernel_wait(pid, &status);
	
	if (ret < 0) {
		printk("[program2] : kernel_wait failed with error %d\n", ret);
		return ret;
	}
	
	return status;
}
	
//implement fork function
int my_fork(void *argc){
	
	//set default sigaction for current process
	int i;
	pid_t pid;
	int status;
	struct k_sigaction *k_action = &current->sighand->action[0];
	for(i=0;i<_NSIG;i++){
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}
	
	/* fork a process using kernel_clone */
	struct kernel_clone_args args = {
		.flags = SIGCHLD,
		.exit_signal = SIGCHLD,
		.stack = (unsigned long)&my_exec,
		.stack_size = 0,
	};
	
	pid = kernel_clone(&args);
	
	if (pid < 0) {
		printk("[program2] : kernel_clone failed with error %d\n", pid);
		return pid;
	}
	
	if (pid == 0) {
		/* This is the child process */
		my_exec();
		/* Should not reach here */
		return 0;
	}

	printk("[program2] : The child process has pid = %d\n", pid);
	printk("[program2] : This is the parent process, pid = %d\n", (int)current->pid);

	status = my_wait(pid);

	//checking the return status
	if(my_WIFEXITED(status)){
		printk("[program2] : child process gets normal termination\n");
		printk("[program2] : The return signal is %d\n", my_WEXITSTATUS(status));
	}
	else if(my_WIFSTOPPED(status)){
		int stopStatus = my_WSTOPSIG(status);
		printk("[program2] : CHILD PROCESS STOPPED\n");
		if(stopStatus == 19 ){
			printk("[program2] : child process get SIGSTOP signal\n");
		}
		else{
			printk("[program2] : child process get a signal not in the samples\n");
		}
		printk("[program2] : The return signal is %d\n", stopStatus);
	}
	else if(my_WIFSIGNALED(status)){
		int terminationStatus = my_WTERMSIG(status);
		printk("[program2] : child process\n");
		
		// Provide specific messages for different signals
		switch(terminationStatus) {
		case 1:  // SIGHUP
			printk("[program2] : get SIGHUP signal\n");
			printk("[program2] : child process is hung up\n");
			break;
		case 2:  // SIGINT
			printk("[program2] : get SIGINT signal\n");
			printk("[program2] : terminal interrupt\n");
			break;
		case 3:  // SIGQUIT
			printk("[program2] : get SIGQUIT signal\n");
			printk("[program2] : terminal quit\n");
			break;
		case 4:  // SIGILL
			printk("[program2] : get SIGILL signal\n");
			printk("[program2] : child process has illegal instruction error\n");
			break;
		case 5:  // SIGTRAP
			printk("[program2] : get SIGTRAP signal\n");
			printk("[program2] : child process has trap error\n");
			break;
		case 6:  // SIGABRT
			printk("[program2] : get SIGABRT signal\n");
			printk("[program2] : child process has abort error\n");
			break;
		case 7:  // SIGBUS
			printk("[program2] : get SIGBUS signal\n");
			printk("[program2] : child process has bus error\n");
			break;
		case 8:  // SIGFPE
			printk("[program2] : get SIGFPE signal\n");
			printk("[program2] : child process has float error\n");
			break;
		case 9:  // SIGKILL
			printk("[program2] : get SIGKILL signal\n");
			printk("[program2] : child process is killed\n");
			break;
		case 11: // SIGSEGV
			printk("[program2] : get SIGSEGV signal\n");
			printk("[program2] : child process has segmentation fault error\n");
			break;
		case 13: // SIGPIPE
			printk("[program2] : get SIGPIPE signal\n");
			printk("[program2] : child process has pipe error\n");
			break;
		case 14: // SIGALRM
			printk("[program2] : get SIGALRM signal\n");
			printk("[program2] : child process has alarm error\n");
			break;
		case 15: // SIGTERM
			printk("[program2] : get SIGTERM signal\n");
			printk("[program2] : child process terminated\n");
			break;
		default:
			if(terminationStatus>=1 && terminationStatus <=15 && processTerminatedSignal[terminationStatus-1]!=NULL){
				printk("[program2] : get %s signal\n", processTerminatedSignal[terminationStatus-1]);
			}
			else{
				printk("[program2] : child process get a signal not in samples\n");
			}
			printk("[program2] : child process terminated\n");
			break;
		}
		printk("[program2] : The return signal is %d\n", terminationStatus);
	}
	else{
		printk("[program2] : CHILD PROCESS CONTINUED\n");
	}
	do_exit(0);

	return 0;
}

static int __init program2_init(void){

	printk("[program2] : module_init\n");
	printk("[program2] : module_init create kthread start\n");
	
	/* create a kernel thread to run my_fork */
	task = kthread_create(&my_fork, NULL, "MyThread");

	//wake up new thread if ok
	if(!IS_ERR(task)){
		printk("[program2] : module_init kthread start\n");
		wake_up_process(task);
	}

	return 0;
}

static void __exit program2_exit(void){
	printk("[program2] : module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);
