/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>
bool check_cross_page(vaddr_t vaddr,int len) {
  vaddr_t vaddr_end = (vaddr&(~PAGE_MASK))+PAGE_SIZE;
  return vaddr+len<=vaddr_end;
}
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  Assert(check_cross_page(vaddr,len),"Cross page error at vaddr:0x%x len:%d\n",vaddr,len);
  uint64_t ppn=cpu.csr.satp.ppn;
  uint64_t pde_addr=(ppn<<PAGE_SHIFT)+(vaddr>>VPN_SHIFT<<2);
  word_t pde=paddr_read(pde_addr,4);
  Assert(pde&V_MASK,"vaddr:0x%x pde:0x%x\n",vaddr,pde);
  //pay attention to the priority
  uint64_t pte_addr=(pde>>PPN_SHIFT<<PAGE_SHIFT)+((vaddr&VPN0_MASK)>>PAGE_SHIFT<<2);
  word_t pte=paddr_read(pte_addr,4);
  Assert(pte&V_MASK,"vaddr:0x%x pte:0x%x\n",vaddr,pte);
  paddr_t paddr=(pte>>PPN_SHIFT<<PAGE_SHIFT)+(vaddr&PAGE_MASK);
  return paddr;
}
int isa_mmu_check(vaddr_t vaddr, int len, int type){
  if (cpu.csr.satp.mode == 0) {
    return MMU_DIRECT;
  }
  return MMU_TRANSLATE;
}