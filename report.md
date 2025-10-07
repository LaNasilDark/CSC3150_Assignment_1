# CSC3150 Assignment 1 Report

## Basic Information

**Name:** Xing Qiliang
**Student ID:** 123090669
**Date:** October 7, 2025

---

## 1. Program Design

### 1.1 Program 1: Signal Handling

#### 1.1.1 Design Philosophy of Signal Handler

Program 1 adopts a parent-child process model to monitor and handle signals. The core design philosophy is:

- Parent process as monitor: responsible for creating child processes, waiting for their completion, and monitoring child process termination status
- Child process as executor: executes test programs through `execvp()` and may terminate due to various signals
- Status analyzer: parent process obtains child process status through `waitpid()` and parses different termination reasons

#### 1.1.2 Implementation of Different Signal Processing Mechanisms

The program uses the status parameter of `waitpid()` system call to identify signal types:

- **Normal termination detection**: Use `WIFEXITED(status)` to check if the process exited normally
- **Signal termination detection**: Use `WIFSIGNALED(status)` to check if the process was terminated by a signal
- **Stop signal detection**: Use `WIFSTOPPED(status)` to check if the process was paused by a stop signal
- **Signal type extraction**: Obtain specific signal numbers through `WTERMSIG(status)` and `WSTOPSIG(status)`

#### 1.1.3 Implementation of Process Control and Signal Transmission

Process control flow:

1. **Process creation**: `fork()` creates child process, return value distinguishes parent and child processes
2. **Program execution**: Child process uses `execvp(argv[1], &argv[1])` to execute target test program
3. **Status monitoring**: Parent process uses `waitpid(pid, &status, WUNTRACED)` to wait for child process status changes
4. **Signal analysis**: Parse child process termination reasons and signal types through status macros

#### 1.1.4 Code Structure and Key Function Description

**Main function structure**:

- `main()`: Program entry point, implements complete process control flow
- `fork()`: Creates child process, returns 0 for child process, >0 for child process PID in parent
- `execvp()`: Executes target program in child process, replaces current process image
- `waitpid()`: Parent process waits for child process status changes, obtains termination status

**Key signal handling logic**:

```c
// Supported signal types include:
SIGABRT, SIGFPE, SIGILL, SIGINT, SIGKILL, SIGPIPE, 
SIGQUIT, SIGSEGV, SIGTERM, SIGTRAP, SIGBUS, SIGALRM, 
SIGHUP, SIGSTOP, SIGTSTP
```

**Test program design**:

- Each test file (such as `alarm.c`, `interrupt.c`, `segment_fault.c`) specifically triggers a particular signal
- Uses `raise()` or system calls (such as `alarm()`) to actively generate signals
- Verifies the completeness of signal handling mechanism through different test programs

### 1.2 Program 2: Process Management

#### 1.2.1 Design Strategy for Process Creation and Management

Program 2 is a kernel module that implements process management strategies in kernel space:

- **Kernel module architecture**: Implements process creation and management in kernel space through Linux kernel module mechanism
- **Modern kernel API**: Uses `kernel_clone()` to replace traditional `do_fork()`, conforming to modern kernel development standards
- **Kernel thread model**: Creates kernel threads through `kthread_create()` to execute process management tasks
- **Signal handler reset**: Resets all signal handlers to default state before creating child processes

#### 1.2.2 Parent-Child Process Communication Mechanism

**Process creation flow**:

1. **Kernel thread creation**: `kthread_create(&my_fork, NULL, "MyThread")` creates kernel thread for executing process management
2. **Child process creation**: Uses `kernel_clone()` to create child process in kernel space
3. **Program execution**: Child process executes user space program through `kernel_execve()`
4. **Status monitoring**: Parent process uses `kernel_wait()` to wait for child process completion and obtain status

**Kernel space characteristics**:

- All operations are performed in kernel space, avoiding user space to kernel space switching overhead
- Uses kernel-specific system call interfaces (`kernel_*` series functions)
- Outputs log information to kernel log buffer through `printk()`

#### 1.2.3 Implementation of Process Synchronization and Coordination

**Synchronization mechanism**:

- **Kernel wait**: `kernel_wait(pid, &status)` implements synchronous waiting of parent process for child process
- **Status analyzer design**: Implements object-oriented style status analyzer, encapsulating status parsing logic
- **Signal handler reset**: Resets all signal handlers by traversing `current->sighand->action[]`

**Status analyzer architecture**:

```c
struct process_status_analyzer {
    int status;
    int (*get_exit_status)(struct process_status_analyzer *self);
    int (*get_term_signal)(struct process_status_analyzer *self);
    int (*is_exited)(struct process_status_analyzer *self);
    // ... other methods
};
```

#### 1.2.4 Error Handling and Resource Management

**Error handling strategy**:

- **System call error checking**: Check return values of `kernel_clone()`, `kernel_execve()`, `kernel_wait()`
- **Resource cleanup**: Perform resource cleanup through `module_exit()` when module exits
- **Memory management**: Properly terminate kernel threads and child processes through `do_exit()`

**Key function implementation**:

- `my_fork()`: Core process management function, handles signal reset, process creation, status monitoring
- `my_exec()`: Child process execution function, calls `kernel_execve()` to execute target program
- `my_wait()`: Wait function, encapsulates `kernel_wait()` and handles error conditions

**Kernel module lifecycle**:

- **Initialization**: `program2_init()` creates and starts kernel thread
- **Execution**: Kernel thread executes process management tasks
- **Cleanup**: `program2_exit()` performs module cleanup work

### 1.3 Bonus: pstree Implementation

#### 1.3.1 Core Algorithm Design of pstree Program

pstree program adopts a multi-stage processing algorithm to build and display process trees:

**Algorithm flow**:

1. **Process scanning phase**: Traverse `/proc` directory, read all process information
2. **Data structure construction**: Create `process_t` structure for each process, store process attributes
3. **Relationship establishment phase**: Build process tree based on PPID to establish parent-child process relationships
4. **Sorting optimization**: Sort processes and child processes according to options
5. **Tree display**: Recursively traverse process tree, generate tree visualization output

#### 1.3.2 Data Structure Selection for Process Tree Construction

**Core data structure**:

```c
typedef struct process {
    int pid, ppid, uid, pgid;           // Basic process information
    char comm[MAX_COMM];                // Process name
    char cmdline[MAX_CMDLINE];          // Command line arguments
    struct process **children;          // Child process pointer array
    int child_count, child_capacity;    // Child process count and capacity
    int thread_count;                   // Thread count
    int is_thread;                      // Thread identification flag
} process_t;
```

**Storage strategy**:

- **Global process table**: `process_t *processes[MAX_PROCESSES]` stores all processes
- **Dynamic child process array**: Each process maintains a dynamically expandable child process pointer array
- **Hash lookup optimization**: Quickly locate processes through PID to establish parent-child relationships

#### 1.3.3 Implementation Strategy for Various Option Functions

**Basic process tree display**:

- Uses recursive algorithm `print_tree()` to traverse process tree
- Manages indentation levels and tree connectors through prefix strings
- Supports both Unicode (`└─├─│`) and ASCII (`|-|`) display modes

**Thread aggregation display (-p, thread count)**:

- Scans `/proc/[pid]/task/` directory to count thread numbers
- Displays thread information in `───N*[{process_name}]` format after process name
- Implements association and counting between threads and main process

**Command line argument display (-a)**:

- Reads `/proc/[pid]/cmdline` file to get complete command line
- Processes null character separators, converts to space-separated readable format
- Prioritizes displaying complete command line, degrades to displaying process name

**Numeric sorting (-n)**:

- Implements `process_compare()` comparison function
- Supports sorting by PID numbers and alphabetical sorting by process names
- Sorts both the entire process table and child processes of each process

**UID change display (-u)**:

- Compares UID between process and parent process to detect permission changes
- Gets username through `getpwuid()`, displays in `(user: username)` format
- Parses UID information from `/proc/[pid]/status` file

**ASCII mode (-A)**:

- Provides ASCII character alternatives: `|-` and `|` replace Unicode characters
- Compatible with terminal environments that don't support Unicode

**Process group ID display (-g)**:

- Reads PGID information from `/proc/[pid]/stat` file
- Displays process group identifier in `[pgid]` format after process name
- Automatically enables compact mode to avoid displaying redundancy

#### 1.3.4 Design Philosophy of Process Chain Compression Algorithm

**Compression strategy**:

- **Single child process chain detection**: Identifies process nodes with only one child process
- **Chain compression**: Merges single child process chains for display on the same line, connected with `───`
- **Recursive compression**: Continues compression until encountering processes with multiple child processes or leaf nodes
- **Alignment calculation**: Precisely calculates the character length of compression chains to ensure correct alignment of child processes

**Algorithm implementation**:

```c
// Core logic of compression algorithm
if (non_thread_children == 1 && !options.compact_not) {
    // Identify single child process and start compression
    printf("───%s", single_child->comm);
    // Continue checking if further compression is possible
    while (child_non_thread_count == 1) {
        // Recursively compress single child process chain
    }
}
```

#### 1.3.5 Implementation of Indentation Alignment and Formatting

**Indentation management**:

- **Prefix string**: Uses recursively passed prefix strings to manage indentation at each level
- **Branch character selection**: Selects different branch characters based on whether it's the last child process
- **Compression alignment**: Calculates precise length of compression chains, adjusts indentation positions of subsequent child processes

**Formatting features**:

- **Highlight display**: Supports `-H PID` option to highlight specified process and its ancestors
- **Information integration**: Integrates PID, PGID, user information, thread count into unified format
- **Long line handling**: Supports `-l` option to control long line truncation behavior

**Display mode selection**:

- **Compact mode**: Uses `print_compact_tree()` by default to implement chain compression
- **Complete mode**: Uses `-c` option to disable compression, displays complete tree structure
- **Compatibility**: Mimics system pstree display behavior and formatting as much as possible

---

## 2. Development Environment Setup

### 2.1 System Information

#### 2.1.1 Operating System Version and Kernel Information

**Operating system environment**:

- **OS**: [Ubuntu 20.04.6 LTS (focal)](https://cuhko365-my.sharepoint.com/:f:/g/personal/220019037_link_cuhk_edu_cn/EqVOz8XWtOpCkxF9MaZm9JUBWsGuCmmDWjibWHyUWnqJpQ?e=ov2zR2)
- **Kernel**: [Linux 5.15.10 (Custom compiled on Oct 6, 2025)](https://mirror.tuna.tsinghua.edu.cn/kernel/v5.x/linux-5.15.10.tar.xz)
- **Virtualization Platform**: VMware® Workstation 17 Pro 17.5.2 build-23775571
- **System Architecture**: x86_64

#### 2.1.2 Development Toolchain Versions (gcc, make, etc.)

**Compilation toolchain**:

- **GCC Version**: 9.4.0 (Ubuntu 9.4.0-1ubuntu1~20.04.2)
- **Make Version**: GNU Make 4.2.1
- **VScode Version**: 1.104.3 (user setup)

### 2.2 Compilation Environment

#### 2.2.1 How to Set Up Compilation Environment

**Environment preparation**:

```bash
# Install necessary development packages
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
sudo apt install git make gcc libc6-dev
sudo apt-get install libncurses-dev gawk flex bison openssl libssl-dev dkms libelf-dev libudev-  dev libpci-dev libiberty-dev autoconf llvm dwarves
```

#### 2.2.2 Makefile Configuration and Usage

**Program 1 Makefile**:

- Compiles all signal test programs and main program
- Supports `make all`, `make clean` commands
- Uses standard gcc compilation options

**Program 2 Makefile**:

- Kernel module compilation configuration
- Uses `$(MAKE) -C /lib/modules/$(shell uname -r)/build` for kernel module compilation
- Supports module installation and uninstallation

**Bonus pstree Makefile**:

- **Compiler setting**: Uses `gcc` as compiler
- **Compilation flags**: `-Wall -Wextra -std=c99` enables all warnings and C99 standard
- **Target file**: Generates executable file named `pstree`
- **Source file**: Compiles from single source file `pstree.c`
- **Clean function**: `make clean` deletes generated executable file
- **Test function**: `make test` automatically tests multiple pstree options
  - Basic function test: `./pstree | head -10`
  - PID display test: `./pstree -p | head -5`
  - Argument display test: `./pstree -a | head -5`
  - ASCII mode test: `./pstree -A | head -5`
  - Numeric sorting test: `./pstree -n -p | head -5`
- **Compilation command**: `$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)`

#### 2.2.3 Compilation Options and Optimization Settings

**Compilation options**:

- **Program 1**: `-Wall -Wextra -std=c99` enables warnings and C99 standard
- **Program 2**: Uses kernel module standard compilation options
- **Bonus**: `-O2 -Wall` enables optimization and warning checks

### 2.3 Kernel Compilation (if applicable)

#### 2.3.1 Preparation Work for Kernel Compilation

Program 2 needs to use three kernel functions: `kernel_clone`, `kernel_execve`, `kernel_wait`. These functions are not exported to modules by default, so custom kernel compilation is required.

#### 2.3.2 Kernel Source Code Acquisition and Configuration

**Kernel source code download**:

```bash
# Download Linux 5.15.10 kernel source code
wget https://mirror.tuna.tsinghua.edu.cn/kernel/v5.x/linux-5.15.10.tar.xz
tar -xf linux-5.15.10.tar.xz
cd linux-5.15.10
```

**Kernel configuration modification**:
Need to modify the following files to export necessary kernel symbols:

- `kernel/fork.c`: Add `EXPORT_SYMBOL(kernel_clone);`
- `kernel/exit.c`: Add `EXPORT_SYMBOL(kernel_wait);`
- `fs/exec.c`: Add `EXPORT_SYMBOL(kernel_execve);`

#### 2.3.3 Specific Steps for Kernel Compilation

**Kernel compilation process**:

```bash
# Clean previous compilation results
make clean
make mrproper

# Configure kernel (use default configuration and save)
make menuconfig

# Compile kernel image and modules
make bzImage -j$(nproc)
make modules -j$(nproc)

# Install modules and kernel
sudo make modules_install
sudo make install

# Reboot to load new kernel
sudo reboot
```

#### 2.3.4 Kernel Installation and Testing Process

**Installation verification**:

```bash
# Check kernel version after reboot
uname -r
# Should display: 5.15.10

# Check exported symbols
cat /proc/kallsyms | grep -E "(kernel_clone|kernel_execve|kernel_wait)"

# Verify kernel module compilation environment
ls /lib/modules/$(uname -r)/build
```

---

## 3. Program Output Screenshots

### 3.1 Program 1 Output

#### 3.1.1 Normal Signal Processing Runtime Screenshots

![1759821258498](image/report/1759821258498.png)

#### 3.1.2 Processing Results of Different Signal Types

![1759822000525](image/report/1759822000525.png "program1 abort result")

![1759822052813](image/report/1759822052813.png "program1 floating result")

![1759822085442](image/report/1759822085442.png "program1 kill result")

![1759822282710](image/report/1759822282710.png "program1 stop result")

To save space, only partial program runtime result screenshots are shown here.

### 3.2 Program 2 Output

```bash
sudo insmod program2.ko
sudo rmmod program2
dmesg | grep program2
```

The result is as follows:

![1759822646703](image/report/1759822646703.png)

### 3.3 Bonus Program Output

#### 3.3.1 Basic pstree Function Demonstration

![1759823002862](image/report/1759823002862.png)

#### 3.3.2 Output Comparison of Various Options

**`./pstree -p` (Display PID)**:

![1759823042127](image/report/1759823042127.png)

**`./pstree -a` (Display Arguments)**:

![1759823090124](image/report/1759823090124.png)

**`./pstree -A` (ASCII Mode)**:

![1759823127024](image/report/1759823127024.png)

**`./pstree -n` (Numeric Sorting)**:

![1759823197932](image/report/1759823197932.png)

**`./pstree -u` (UID Changes)**:

![1759823265555](image/report/1759823265555.png)

Only partial functions are shown here. Other related functions can be queried through `./pstree -h`:

![1759823419173](image/report/1759823419173.png)

---

## 4. Learning Outcomes

### 4.1 Program 1

#### 4.1.1 Deepened Understanding of Linux System Programming

Through the implementation of Program 1, gained deep understanding of the following core concepts:

- **Process lifecycle management**: Mastered the collaborative working mechanism of `fork()`, `execvp()`, `waitpid()`
- **Signal handling mechanism**: Learned to use status macros like `WIFEXITED()`, `WIFSIGNALED()`, `WIFSTOPPED()` to parse process termination reasons
- **Parent-child process synchronization**: Understood dependency relationships and synchronous waiting mechanisms between processes
- **Error handling**: Learned error checking and exception handling best practices for system calls

#### 4.1.2 Mastery of Signal Handling Mechanisms

**Signal types with deep understanding**:

- **Exception signals**: SIGSEGV (segmentation fault), SIGBUS (bus error), SIGFPE (floating point exception)
- **Termination signals**: SIGTERM (terminate), SIGKILL (force kill), SIGINT (interrupt)
- **Timing signals**: SIGALRM (alarm)
- **Stop signals**: SIGSTOP (stop), SIGTSTP (terminal stop)

**Technical points mastered**:

- Signal generation mechanisms (`raise()`, `alarm()`, etc.)
- Signal status parsing methods (bit operations and status macros)
- Differences in how different signals affect process behavior

### 4.2 Program 2

#### 4.2.1 Kernel Modification and Compilation Operations

**Kernel development experience**:

- **Kernel module programming**: Learned basic structure of kernel modules, including usage of `module_init()`, `module_exit()`
- **Kernel API usage**: Mastered modern kernel APIs such as `kernel_clone()`, `kernel_execve()`, `kernel_wait()`
- **Symbol export**: Understood the role of `EXPORT_SYMBOL()` and kernel symbol table management
- **Kernel space programming**: Learned to use `printk()` for kernel log output, understood the difference between kernel space and user space

#### 4.2.2 First Time Learning that Make Can Display a UI in Command Line to Generate Config Information

**Kernel configuration and compilation process**:

- **Using menuconfig**: Learned to use `make menuconfig` for graphical interface operations in kernel configuration
- **Compilation optimization**: Understood efficiency improvements of parallel compilation with `-j$(nproc)`
- **Module installation**: Mastered the difference and roles of `make modules_install` and `make install`
- **Version management**: Learned how to manage multiple kernel versions and switch boot options

### 4.3 Bonus

#### 4.3.1 About pstree Usage

**Advanced system programming skills**:

- **File system operations**: Proficiently used `/proc` file system to obtain process information
- **Dynamic memory management**: Implemented dynamically expandable child process arrays, mastered advanced usage of `malloc()`, `realloc()`, `free()`
- **String processing**: Handled null character separators and format conversion in `/proc/[pid]/cmdline`
- **Data structure design**: Implemented efficient process tree data structures and traversal algorithms
- **Algorithm optimization**: Designed process chain compression algorithm, improved display effects and performance

---

## Appendix

### A. Compilation Commands

```bash
# Program 1
cd source/program1
make

# Program 2  
cd source/program2
make

# Bonus Program
cd source/bonus
make
```
