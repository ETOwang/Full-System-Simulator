#include <memory.h>
#include <common.h>
#include <proc.h>
static void *pf = NULL;

void *new_page(size_t nr_page)
{
  void *ret = pf;
  pf = (char *)pf + nr_page * PGSIZE;
  return ret;
}

#ifdef HAS_VME
static void *pg_alloc(int n)
{
  assert(n % PGSIZE == 0);
  void *p = new_page(n / PGSIZE);
  memset(p, 0, n);
  return p;
}
#endif

void free_page(void *p)
{
  panic("not implement yet");
}

/* The brk() system call handler. */
//may cause bug
int mm_brk(uintptr_t brk)
{  
  if(current->max_brk==0){
       current->max_brk=brk;
       return 0;
  }
  if (brk > current->max_brk)
  {
    uintptr_t start = current->max_brk;
    uintptr_t end = ROUNDUP(brk, PGSIZE);
    for (uintptr_t i = start; i < end; i += PGSIZE)
    {
      void *p = new_page(1);
      map(&current->as, (void *)i, p, 7);
    }
    current->max_brk = end;
  }
  return 0;
}

void init_mm()
{
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);
#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
