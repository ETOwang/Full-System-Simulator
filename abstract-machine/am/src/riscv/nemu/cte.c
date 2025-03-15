#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

extern void __am_switch(Context *c);
extern void __am_get_cur_as(Context *c);
Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case YIELD_SYSCALL:
      uint32_t sys= c->GPR1;
      if(sys==-1){
        ev.event = EVENT_YIELD;
      }else{
        ev.event = EVENT_SYSCALL;
      }
      break;
      case IRQ_TIMER:
        ev.event=EVENT_IRQ_TIMER;
        break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context* ctx=(Context*)kstack.end-sizeof(Context);
  //set the entry(to match the trap.S)
  ctx->mepc=(uintptr_t)entry;
  //to pass difftest
  ctx->mstatus=0x1800;
  ctx->GPR2=(uintptr_t)arg;
  ctx->pdir=NULL;
  return ctx;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
