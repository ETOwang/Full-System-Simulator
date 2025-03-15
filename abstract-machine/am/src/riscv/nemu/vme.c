#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void *(*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void *) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
    NEMU_PADDR_SPACE};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir)
{
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> PG_SHIFT)));
}

static inline uintptr_t get_satp()
{
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void *(*pgalloc_f)(int), void (*pgfree_f)(void *))
{
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  int i;
  for (i = 0; i < LENGTH(segments); i++)
  {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE)
    {
      map(&kas, va, va, 0);
    }
  }
  set_satp(kas.ptr);
  vme_enable = 1;
  return true;
}

void protect(AddrSpace *as)
{
  PTE *updir = (PTE *)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as)
{
  pgfree_usr(as->ptr);
}

void __am_get_cur_as(Context *c)
{
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c)
{
  if (vme_enable && c->pdir != NULL)
  {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot)
{
  uintptr_t vpn1 = (uintptr_t)va >> VPN_SHIFT;
  uintptr_t vpn0 = ((uintptr_t)va & VPN0_MASK) >> PG_SHIFT;
  uintptr_t *pgdir = (uintptr_t *)as->ptr;
  uintptr_t pgtab = pgdir[vpn1]>>10<<PG_SHIFT;
  if (!(pgdir[vpn1]& PTE_V))
  {
    pgtab=(uintptr_t)pgalloc_usr(PGSIZE);
    pgdir[vpn1] = (pgtab>>PG_SHIFT<<10)| PTE_V;
  }
  assert((*((uintptr_t*)pgtab + vpn0)&PTE_V)==0);
  *((uintptr_t *)pgtab + vpn0) = (uintptr_t)pa >> PG_SHIFT << 10 |prot << 1| PTE_V;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry)
{
  Context *ctx = (Context *)kstack.end - sizeof(Context);
  ctx->mepc = (uintptr_t)entry;
  ctx->mstatus = 0x1800;
  ctx->pdir = as->ptr;
  return ctx;
}
