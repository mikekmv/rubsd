/* $RuOBSD$ */

#ifndef _A386_UNIX_OS_LINUX_I386_UNISTD_H
#define _A386_UNIX_OS_LINUX_I386_UNISTD_H

extern int unix_errno;

/* user-visible error numbers are in the range -1 - -122: see <asm-i386/errno.h> */

#define _a386__syscall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
		unix_errno = -(res); \
		res = -1; \
	} \
	return (type) (res); \
} while (0)

/* XXX - _foo needs to be __foo, while __NR_bar could be _NR_bar. */
#define _a386_syscall0(type,name) \
type name(void) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name)); \
_a386__syscall_return(type,__res); \
}

#define _a386_syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(arg1))); \
_a386__syscall_return(type,__res); \
}

#define _a386_syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2))); \
_a386__syscall_return(type,__res); \
}

#define _a386_syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
		  "d" ((long)(arg3))); \
_a386__syscall_return(type,__res); \
}

#define _a386_syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
	  "d" ((long)(arg3)),"S" ((long)(arg4))); \
_a386__syscall_return(type,__res); \
} 

#define _a386_syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
	  type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
	  "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5))); \
_a386__syscall_return(type,__res); \
}

#define __NR_exit		  1
#ifndef __NR_fork
#  define __NR_fork		  2
#endif
#ifndef __NR_read
#  define __NR_read		  3
#endif
#ifndef __NR_write
#  define __NR_write		  4
#endif
#ifndef __NR_open
#  define __NR_open		  5
#endif
#ifndef __NR_close
#  define __NR_close		  6
#endif
#ifndef __NR_waitpid
#  define __NR_waitpid		  7
#endif
#ifndef __NR_creat
#  define __NR_creat		  8
#endif
#ifndef __NR_link
#  define __NR_link		  9
#endif
#ifndef __NR_unlink
#  define __NR_unlink		 10
#endif
#ifndef __NR_execve
#  define __NR_execve		 11
#endif
#ifndef __NR_chdir
#  define __NR_chdir		 12
#endif
#ifndef __NR_time
#  define __NR_time		 13
#endif
#ifndef __NR_mknod
#  define __NR_mknod		 14
#endif
#ifndef __NR_chmod
#  define __NR_chmod		 15
#endif
#ifndef __NR_lchown
#  define __NR_lchown		 16
#endif
#ifndef __NR_break
#  define __NR_break		 17
#endif
#ifndef __NR_oldstat
#  define __NR_oldstat		 18
#endif
#ifndef __NR_lseek
#  define __NR_lseek		 19
#endif
#ifndef __NR_getpid
#  define __NR_getpid		 20
#endif
#ifndef __NR_mount
#  define __NR_mount		 21
#endif
#ifndef __NR_umount
#  define __NR_umount		 22
#endif
#ifndef __NR_setuid
#  define __NR_setuid		 23
#endif
#ifndef __NR_getuid
#  define __NR_getuid		 24
#endif
#ifndef __NR_stime
#  define __NR_stime		 25
#endif
#ifndef __NR_ptrace
#  define __NR_ptrace		 26
#endif
#ifndef __NR_alarm
#  define __NR_alarm		 27
#endif
#ifndef __NR_oldfstat
#  define __NR_oldfstat		 28
#endif
#ifndef __NR_pause
#  define __NR_pause		 29
#endif
#ifndef __NR_utime
#  define __NR_utime		 30
#endif
#ifndef __NR_stty
#  define __NR_stty		 31
#endif
#ifndef __NR_gtty
#  define __NR_gtty		 32
#endif
#ifndef __NR_access
#  define __NR_access		 33
#endif
#ifndef __NR_nice
#  define __NR_nice		 34
#endif
#ifndef __NR_ftime
#  define __NR_ftime		 35
#endif
#ifndef __NR_sync
#  define __NR_sync		 36
#endif
#ifndef __NR_kill
#  define __NR_kill		 37
#endif
#ifndef __NR_rename
#  define __NR_rename		 38
#endif
#ifndef __NR_mkdir
#  define __NR_mkdir		 39
#endif
#ifndef __NR_rmdir
#  define __NR_rmdir		 40
#endif
#ifndef __NR_dup
#  define __NR_dup		 41
#endif
#ifndef __NR_pipe
#  define __NR_pipe		 42
#endif
#ifndef __NR_times
#  define __NR_times		 43
#endif
#ifndef __NR_prof
#  define __NR_prof		 44
#endif
#ifndef __NR_brk
#  define __NR_brk		 45
#endif
#ifndef __NR_setgid
#  define __NR_setgid		 46
#endif
#ifndef __NR_getgid
#  define __NR_getgid		 47
#endif
#ifndef __NR_signal
#  define __NR_signal		 48
#endif
#ifndef __NR_geteuid
#  define __NR_geteuid		 49
#endif
#ifndef __NR_getegid
#  define __NR_getegid		 50
#endif
#ifndef __NR_acct
#  define __NR_acct		 51
#endif
#ifndef __NR_umount2
#  define __NR_umount2		 52
#endif
#ifndef __NR_lock
#  define __NR_lock		 53
#endif
#ifndef __NR_ioctl
#  define __NR_ioctl		 54
#endif
#ifndef __NR_fcntl
#  define __NR_fcntl		 55
#endif
#ifndef __NR_mpx
#  define __NR_mpx		 56
#endif
#ifndef __NR_setpgid
#  define __NR_setpgid		 57
#endif
#ifndef __NR_ulimit
#  define __NR_ulimit		 58
#endif
#ifndef __NR_oldolduname
#  define __NR_oldolduname	 59
#endif
#ifndef __NR_umask
#  define __NR_umask		 60
#endif
#ifndef __NR_chroot
#  define __NR_chroot		 61
#endif
#ifndef __NR_ustat
#  define __NR_ustat		 62
#endif
#ifndef __NR_dup2
#  define __NR_dup2		 63
#endif
#ifndef __NR_getppid
#  define __NR_getppid		 64
#endif
#ifndef __NR_getpgrp
#  define __NR_getpgrp		 65
#endif
#ifndef __NR_setsid
#  define __NR_setsid		 66
#endif
#ifndef __NR_sigaction
#  define __NR_sigaction		 67
#endif
#ifndef __NR_sgetmask
#  define __NR_sgetmask		 68
#endif
#ifndef __NR_ssetmask
#  define __NR_ssetmask		 69
#endif
#ifndef __NR_setreuid
#  define __NR_setreuid		 70
#endif
#ifndef __NR_setregid
#  define __NR_setregid		 71
#endif
#ifndef __NR_sigsuspend
#  define __NR_sigsuspend		 72
#endif
#ifndef __NR_sigpending
#  define __NR_sigpending		 73
#endif
#ifndef __NR_sethostname
#  define __NR_sethostname	 74
#endif
#ifndef __NR_setrlimit
#  define __NR_setrlimit		 75
#endif
#ifndef __NR_getrlimit
#  define __NR_getrlimit		 76
#endif
#ifndef __NR_getrusage
#  define __NR_getrusage		 77
#endif
#ifndef __NR_gettimeofday
#  define __NR_gettimeofday	 78
#endif
#ifndef __NR_settimeofday
#  define __NR_settimeofday	 79
#endif
#ifndef __NR_getgroups
#  define __NR_getgroups		 80
#endif
#ifndef __NR_setgroups
#  define __NR_setgroups		 81
#endif
#ifndef __NR_select
#  define __NR_select		 82
#endif
#ifndef __NR_symlink
#  define __NR_symlink		 83
#endif
#ifndef __NR_oldlstat
#  define __NR_oldlstat		 84
#endif
#ifndef __NR_readlink
#  define __NR_readlink		 85
#endif
#ifndef __NR_uselib
#  define __NR_uselib		 86
#endif
#ifndef __NR_swapon
#  define __NR_swapon		 87
#endif
#ifndef __NR_reboot
#  define __NR_reboot		 88
#endif
#ifndef __NR_readdir
#  define __NR_readdir		 89
#endif
#ifndef __NR_mmap
#  define __NR_mmap		 90
#endif
#ifndef __NR_munmap
#  define __NR_munmap		 91
#endif
#ifndef __NR_truncate
#  define __NR_truncate		 92
#endif
#ifndef __NR_ftruncate
#  define __NR_ftruncate		 93
#endif
#ifndef __NR_fchmod
#  define __NR_fchmod		 94
#endif
#ifndef __NR_fchown
#  define __NR_fchown		 95
#endif
#ifndef __NR_getpriority
#  define __NR_getpriority	 96
#endif
#ifndef __NR_setpriority
#  define __NR_setpriority	 97
#endif
#ifndef __NR_profil
#  define __NR_profil		 98
#endif
#ifndef __NR_statfs
#  define __NR_statfs		 99
#endif
#ifndef __NR_fstatfs
#  define __NR_fstatfs		100
#endif
#ifndef __NR_ioperm
#  define __NR_ioperm		101
#endif
#ifndef __NR_socketcall
#  define __NR_socketcall		102
#endif
#ifndef __NR_syslog
#  define __NR_syslog		103
#endif
#ifndef __NR_setitimer
#  define __NR_setitimer		104
#endif
#ifndef __NR_getitimer
#  define __NR_getitimer		105
#endif
#ifndef __NR_stat
#  define __NR_stat		106
#endif
#ifndef __NR_lstat
#  define __NR_lstat		107
#endif
#ifndef __NR_fstat
#  define __NR_fstat		108
#endif
#ifndef __NR_olduname
#  define __NR_olduname		109
#endif
#ifndef __NR_iopl
#  define __NR_iopl		110
#endif
#ifndef __NR_vhangup
#  define __NR_vhangup		111
#endif
#ifndef __NR_idle
#  define __NR_idle		112
#endif
#ifndef __NR_vm86old
#  define __NR_vm86old		113
#endif
#ifndef __NR_wait4
#  define __NR_wait4		114
#endif
#ifndef __NR_swapoff
#  define __NR_swapoff		115
#endif
#ifndef __NR_sysinfo
#  define __NR_sysinfo		116
#endif
#ifndef __NR_ipc
#  define __NR_ipc		117
#endif
#ifndef __NR_fsync
#  define __NR_fsync		118
#endif
#ifndef __NR_sigreturn
#  define __NR_sigreturn		119
#endif
#ifndef __NR_clone
#  define __NR_clone		120
#endif
#ifndef __NR_setdomainname
#  define __NR_setdomainname	121
#endif
#ifndef __NR_uname
#  define __NR_uname		122
#endif
#ifndef __NR_modify_ldt
#  define __NR_modify_ldt		123
#endif
#ifndef __NR_adjtimex
#  define __NR_adjtimex		124
#endif
#ifndef __NR_mprotect
#  define __NR_mprotect		125
#endif
#ifndef __NR_sigprocmask
#  define __NR_sigprocmask	126
#endif
#ifndef __NR_create_module
#  define __NR_create_module	127
#endif
#ifndef __NR_init_module
#  define __NR_init_module	128
#endif
#ifndef __NR_delete_module
#  define __NR_delete_module	129
#endif
#ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms	130
#endif
#ifndef __NR_quotactl
#  define __NR_quotactl		131
#endif
#ifndef __NR_getpgid
#  define __NR_getpgid		132
#endif
#ifndef __NR_fchdir
#  define __NR_fchdir		133
#endif
#ifndef __NR_bdflush
#  define __NR_bdflush		134
#endif
#ifndef __NR_sysfs
#  define __NR_sysfs		135
#endif
#ifndef __NR_personality
#  define __NR_personality	136
#endif
#ifndef __NR_afs_syscall
#  define __NR_afs_syscall	137
#endif
#ifndef __NR_setfsuid
#  define __NR_setfsuid		138
#endif
#ifndef __NR_setfsgid
#  define __NR_setfsgid		139
#endif
#ifndef __NR__llseek
#  define __NR__llseek		140
#endif
#ifndef __NR_getdents
#  define __NR_getdents		141
#endif
#ifndef __NR__newselect
#  define __NR__newselect		142
#endif
#ifndef __NR_flock
#  define __NR_flock		143
#endif
#ifndef __NR_msync
#  define __NR_msync		144
#endif
#ifndef __NR_readv
#  define __NR_readv		145
#endif
#ifndef __NR_writev
#  define __NR_writev		146
#endif
#ifndef __NR_getsid
#  define __NR_getsid		147
#endif
#ifndef __NR_fdatasync
#  define __NR_fdatasync		148
#endif
#ifndef __NR__sysctl
#  define __NR__sysctl		149
#endif
#ifndef __NR_mlock
#  define __NR_mlock		150
#endif
#ifndef __NR_munlock
#  define __NR_munlock		151
#endif
#ifndef __NR_mlockall
#  define __NR_mlockall		152
#endif
#ifndef __NR_munlockall
#  define __NR_munlockall		153
#endif
#ifndef __NR_sched_setparam
#  define __NR_sched_setparam		154
#endif
#ifndef __NR_sched_getparam
#  define __NR_sched_getparam		155
#endif
#ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler		156
#endif
#ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler		157
#endif
#ifndef __NR_sched_yield
#  define __NR_sched_yield		158
#endif
#ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max	159
#endif
#ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min	160
#endif
#ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval	161
#endif
#ifndef __NR_nanosleep
#  define __NR_nanosleep		162
#endif
#ifndef __NR_mremap
#  define __NR_mremap		163
#endif
#ifndef __NR_setresuid
#  define __NR_setresuid		164
#endif
#ifndef __NR_getresuid
#  define __NR_getresuid		165
#endif
#ifndef __NR_vm86
#  define __NR_vm86		166
#endif
#ifndef __NR_query_module
#  define __NR_query_module	167
#endif
#ifndef __NR_poll
#  define __NR_poll		168
#endif
#ifndef __NR_nfsservctl
#  define __NR_nfsservctl		169
#endif
#ifndef __NR_setresgid
#  define __NR_setresgid		170
#endif
#ifndef __NR_getresgid
#  define __NR_getresgid		171
#endif
#ifndef __NR_prctl
#  define __NR_prctl              172
#endif
#ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn	173
#endif
#ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction	174
#endif
#ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask	175
#endif
#ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending	176
#endif
#ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait	177
#endif
#ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo	178
#endif
#ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend	179
#endif
#ifndef __NR_pread
#  define __NR_pread		180
#endif
#ifndef __NR_pwrite
#  define __NR_pwrite		181
#endif
#ifndef __NR_chown
#  define __NR_chown		182
#endif
#ifndef __NR_getcwd
#  define __NR_getcwd		183
#endif
#ifndef __NR_capget
#  define __NR_capget		184
#endif
#ifndef __NR_capset
#  define __NR_capset		185
#endif
#ifndef __NR_sigaltstack
#  define __NR_sigaltstack	186
#endif
#ifndef __NR_sendfile
#  define __NR_sendfile		187
#endif
#ifndef __NR_getpmsg
#  define __NR_getpmsg		188
#endif
#ifndef __NR_putpmsg
#  define __NR_putpmsg		189
#endif
#ifndef __NR_vfork
#  define __NR_vfork		190
#endif

#endif /* _A386_UNIX_OS_LINUX_I386_UNISTD_H */
