#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb()
{
  current = &pcb_boot;
}

void hello_fun(void *arg)
{
  int j = 1;
  while (1)
  {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
    j++;
    yield();
  }
}
void context_kload(PCB *pcb, void (*entry)(void *), void *arg)
{
  Context *ctx = kcontext((Area){pcb->stack, pcb + 1}, entry, arg);
  pcb->cp = ctx;
}
extern uintptr_t loader(PCB *pcb, const char *filename);
extern void protect(AddrSpace *as);
extern void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[])
{                         
  assert(pcb && filename);
  protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);
  printf("entry: %x\n", entry);
  Context *ctx = ucontext(&pcb->as, (Area){pcb->stack, pcb + 1}, (void *)entry);
  void* ustack=new_page(8)+8*PGSIZE;
  void* start=ustack;
  for (size_t i = 0; i < 8; i++)
  { 
    map(&pcb->as, (void*)((uintptr_t)pcb->as.area.end-(i+1)*PGSIZE), (void*)((uintptr_t)ustack-(i+1)*PGSIZE),7);
  }
  int argc = 0;
  int envc = 0;
  // TODO:number of argv and envp
  void *arg_ptr[100] = {0};
  void *env_ptr[100] = {0};
  if (argv)
  {
    for (size_t i = 0; argv[i] != NULL; i++)
    {
      argc++;
      ustack -= strlen(argv[i]) + 1;
      memcpy(ustack, argv[i], strlen(argv[i]) + 1); // include '\0'
      arg_ptr[i] = ustack;
    }
    
  }
  if (envp)
  {
    for (size_t i = 0; envp[i] != NULL; i++)
    {
      envc++;
      ustack -= strlen(envp[i]) + 1;
      memcpy(ustack, envp[i], strlen(envp[i]) + 1);
      env_ptr[i] = ustack;
    }
  }
  
  // TODO:align
  ustack -= sizeof(uintptr_t);
  *(uintptr_t *)ustack = (uintptr_t)NULL;
  for (int i = envc - 1; i >= 0; i--)
  {
    ustack -= sizeof(uintptr_t);
    *(uintptr_t *)ustack = (uintptr_t)env_ptr[i];
  }
  ustack -= sizeof(uintptr_t);
  *(uintptr_t *)ustack = (uintptr_t)NULL;
  for (int i = argc - 1; i >= 0; i--)
  {
    ustack -= sizeof(uintptr_t);
    *(uintptr_t *)ustack = (uintptr_t)arg_ptr[i];
  }
  ustack -= sizeof(int);
  *(int *)ustack = argc;
  ctx->GPRx = (uintptr_t)(pcb->as.area.end - ((uintptr_t)start-(uintptr_t)ustack));
  pcb->cp=ctx;
}
// void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
//   assert(pcb && filename);
//   protect(&pcb->as);
//   uintptr_t entry = loader(pcb, filename);
//   Context *ctx = ucontext(&pcb->as, (Area){pcb->stack, pcb + 1}, (void *)entry);

//   // 分配并映射用户栈（8页）
//   void *phys_base = new_page(8);
//   for (int i = 0; i < 8; i++) {
//       void *pa = phys_base + i * PGSIZE;
//       void *va = (void *)((uintptr_t)pcb->as.area.end - (i+1)*PGSIZE);
//       map(&pcb->as, va, pa, 3); // RW权限
//   }
//   void *ustack = (void *)((uintptr_t)pcb->as.area.end - 8 * PGSIZE);

//   // 计算参数数量
//   int argc = 0, envc = 0;
//   if (argv) while (argv[argc]) argc++;
//   if (envp) while (envp[envc]) envc++;
//   assert(argc < 100 && envc < 100);

//   // 复制字符串到栈底（低地址方向）
//   char *arg_str[100] = {0}, *env_str[100] = {0};
//   uintptr_t ustack_bottom = (uintptr_t)ustack - 8 * PGSIZE; // 栈底

//   // 复制 argv 字符串
//   uintptr_t current = ustack_bottom;
//   for (int i = argc-1; i >= 0; i--) {
//       size_t len = strlen(argv[i]) + 1;
//       current += len;
//       current = current-current%16; // 16字节对齐
//       memcpy((void *)current, argv[i], len);
//       arg_str[i] = (char *)current;
//   }

//   // 复制 envp 字符串
//   for (int i = envc-1; i >= 0; i--) {
//       size_t len = strlen(envp[i]) + 1;
//       current += len;
//       current = current-current%16; // 16字节对齐
//       memcpy((void *)current, envp[i], len);
//       env_str[i] = (char *)current;
//   }

//   // 构建指针数组（栈顶方向）
//   current = ROUNDUP(current, 16);
//   uintptr_t *argv_ptr = (uintptr_t *)current;
//   for (int i = 0; i < argc; i++) {
//       argv_ptr[i] = (uintptr_t)arg_str[i];
//   }
//   argv_ptr[argc] = 0; // NULL 终止

//   uintptr_t *envp_ptr = argv_ptr + argc + 1;
//   for (int i = 0; i < envc; i++) {
//       envp_ptr[i] = (uintptr_t)env_str[i];
//   }
//   envp_ptr[envc] = 0; // NULL 终止

//   // 设置寄存器
//   ctx->GPRx = (uintptr_t)ustack; // 栈指针

//   pcb->cp = ctx;
// }
void init_proc()
{
  Log("Initializing processes...");
  //context_uload(&pcb[0], "/bin/nslider", (char *[]){"/bin/exec-test","1", NULL}, (char *[]){"PATH=/bin", NULL});
  context_uload(&pcb[0], "/bin/nslider", (char *[]){"/bin/exec-test","1", NULL}, (char *[]){"PATH=/bin", NULL});
  //context_kload(&pcb[0], hello_fun, "world");
  //context_kload(&pcb[1], hello_fun, "hello");
  switch_boot_pcb();
}
Context *schedule(Context *prev)
{
  current->cp = prev;
  for (size_t i = 0; i < MAX_NR_PROC; i++)
  {
    PCB *cur = &pcb[i];
    if (cur->cp != NULL && cur->cp != prev)
    {
      current = cur;
      return current->cp;
    }
  }
  return prev;
}
void sys_execve(const char *filename, char *const argv[], char *const envp[])
{
  context_uload(current, filename, argv, envp);
  switch_boot_pcb();
  yield();
}
