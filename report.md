# CSC3150 Assignment 1 Report

## Basic Information
**Name:** [填写你的姓名]  
**Student ID:** [填写你的学号]  
**Date:** October 7, 2025

---

## 1. Program Design (4 points)

### 1.1 Overall Architecture
- [ ] 简述你的程序整体架构设计
- [ ] 说明主要模块的功能划分

### 1.2 Program 1: Signal Handling
- [ ] 信号处理程序的设计思路
- [ ] 如何实现不同信号的处理机制
- [ ] 进程控制和信号传递的实现方法
- [ ] 代码结构和关键函数说明

### 1.3 Program 2: Process Management
- [ ] 进程创建和管理的设计策略
- [ ] 父子进程通信机制
- [ ] 进程同步和协调的实现
- [ ] 错误处理和资源管理

### 1.4 Bonus: pstree Implementation
- [ ] pstree 程序的核心算法设计
- [ ] 进程树构建的数据结构选择
- [ ] 各种选项功能的实现策略
  - [ ] 基本进程树显示
  - [ ] 线程聚合显示 (-p, 线程计数)
  - [ ] 命令行参数显示 (-a)
  - [ ] 数字排序 (-n)
  - [ ] UID变化显示 (-u)
  - [ ] ASCII模式 (-A)
  - [ ] 进程组ID显示 (-g)
  - [ ] 其他实现的选项
- [ ] 进程链压缩算法的设计思路
- [ ] 缩进对齐和格式化的实现

---

## 2. Development Environment Setup (2 points)

### 2.1 System Information
- [ ] 操作系统版本和内核信息
- [ ] 开发工具链版本 (gcc, make, etc.)

### 2.2 Compilation Environment
- [ ] 如何设置编译环境
- [ ] Makefile 的配置和使用
- [ ] 编译选项和优化设置

### 2.3 Kernel Compilation (if applicable)
- [ ] 内核编译的准备工作
- [ ] 内核源码获取和配置
- [ ] 编译内核的具体步骤
- [ ] 内核安装和测试过程
- [ ] 遇到的问题和解决方案

### 2.4 Debugging Environment
- [ ] 调试工具的配置 (gdb, valgrind, etc.)
- [ ] 如何进行程序调试和测试

---

## 3. Program Output Screenshots (2 points)

### 3.1 Program 1 Output
- [ ] 正常信号处理的运行截图
- [ ] 不同信号类型的处理结果
- [ ] 错误情况的处理演示

### 3.2 Program 2 Output
- [ ] 进程创建和管理的运行结果
- [ ] 父子进程交互的演示
- [ ] 程序执行的完整流程截图

### 3.3 Bonus Program Output
- [ ] 基本 pstree 功能演示
- [ ] 各种选项的输出对比
  - [ ] `./pstree` (基本输出)
  - [ ] `./pstree -p` (显示PID)
  - [ ] `./pstree -a` (显示参数)
  - [ ] `./pstree -A` (ASCII模式)
  - [ ] `./pstree -n` (数字排序)
  - [ ] `./pstree -u` (UID变化)
  - [ ] 其他选项组合
- [ ] 与系统 pstree 的对比截图
- [ ] 复杂进程树的显示效果 (如包含多线程的进程)

---

## 4. Learning Outcomes (2 points)

### 4.1 Technical Skills Gained
- [ ] 对 Linux 系统编程的理解加深
- [ ] 信号处理机制的掌握
- [ ] 进程管理和 IPC 的实践经验
- [ ] 文件系统操作 (/proc) 的应用

### 4.2 Programming Concepts
- [ ] 系统调用的使用经验
- [ ] 错误处理和资源管理的重要性
- [ ] 数据结构设计在系统编程中的应用
- [ ] 算法优化和性能考虑

### 4.3 Problem-Solving Experience
- [ ] 遇到的主要挑战和解决过程
- [ ] 调试技巧和经验总结
- [ ] 代码优化和改进的思路

### 4.4 Understanding of Operating Systems
- [ ] 对进程和线程概念的深入理解
- [ ] 操作系统内核机制的认识
- [ ] 系统性能和资源管理的考虑

---

## 5. Conclusion

### 5.1 Assignment Summary
- [ ] 完成情况总结
- [ ] 各部分功能的实现程度
- [ ] 达到的技术指标

### 5.2 Future Improvements
- [ ] 可能的优化方向
- [ ] 功能扩展的设想
- [ ] 代码质量改进的思路

---

## Appendix

### A. Source Code Structure
```
source/
├── program1/
│   ├── program1.c
│   ├── [其他信号处理相关文件]
│   └── Makefile
├── program2/
│   ├── program2.c
│   ├── [其他进程管理相关文件]
│   └── Makefile
└── bonus/
    ├── pstree.c
    ├── Makefile
    └── README (if applicable)
```

### B. Compilation Commands
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

### C. Test Commands
```bash
# 填写你用于测试的具体命令
```

---

## References
- [ ] 课程资料和讲义
- [ ] Linux man pages
- [ ] 相关技术文档和资源
