#include <common.h>
#include <fs.h>
#include <sys/time.h>
#include "syscall.h"
#include <proc.h>
static int sys_brk(int increment) {
  return 0;
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
extern void naive_uload(PCB *pcb, const char *filename);
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_yield: yield(); c->GPRx=0;break;
    case SYS_exit: naive_uload(NULL,"/bin/menu"); break;
    case SYS_write:
    c->GPRx = fs_write(a[1], (void *)a[2], a[3]); break;
    case SYS_brk:
    c->GPRx = sys_brk(a[1]); break;
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
    naive_uload(NULL, (char *)a[1]); c->GPRx=-1;break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
