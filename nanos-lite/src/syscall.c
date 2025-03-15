#include <common.h>
#include <fs.h>
#include <sys/time.h>
#include "syscall.h"
#include <proc.h>
extern int mm_brk(uintptr_t brk);
static int sys_brk(uintptr_t addr) {
  //return -1;
  return mm_brk(addr);
}
extern void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime);
static int sys_gettimeofday(struct timeval *tv, struct timezone *tz){
  AM_TIMER_UPTIME_T uptime;
  __am_timer_uptime(&uptime);
  if (tv != NULL) {
    tv->tv_sec = uptime.us / 1000000;
    tv->tv_usec = uptime.us;
  }
  if (tz != NULL) {
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
  }
  return 0;
}
extern Context* schedule(Context *prev);
extern void sys_execve(const char *filename, char *const argv[], char *const envp[]);
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  printf("syscall ID = %d\n", a[0]);
  switch (a[0]) {
    case SYS_yield:c->GPRx=(uintptr_t)schedule(c);break;
    case SYS_exit: sys_execve("/bin/menu",NULL,NULL); break;
    case SYS_write:
    c->GPRx = fs_write(a[1], (void *)a[2], a[3]); break;
    case SYS_brk:
    c->GPRx = sys_brk(a[1]);
    break;
    case SYS_read:
    c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
    break;
    case SYS_open:
    c->GPRx = fs_open((char *)a[1], a[2], a[3]); break;
    case SYS_close:
    c->GPRx = fs_close(a[1]); break;
    case SYS_lseek:
    c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_gettimeofday:
    c->GPRx = sys_gettimeofday((void *)a[1], (void *)a[2]); break;
    case SYS_execve:
    sys_execve((char*)a[1],(char**)a[2],(char**)a[3]); c->GPRx=-1;break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
