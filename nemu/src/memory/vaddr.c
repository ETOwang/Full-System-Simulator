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
#include <memory/paddr.h>
word_t vaddr_ifetch(vaddr_t addr, int len) {
  if(isa_mmu_check(addr, len, MEM_TYPE_IFETCH) == MMU_DIRECT){
    return paddr_read(addr, len);
  }
  return paddr_read(isa_mmu_translate(addr,len,MEM_TYPE_IFETCH), len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  if(isa_mmu_check(addr, len, MEM_TYPE_READ) == MMU_DIRECT){
    return paddr_read(addr, len);
  }
  return paddr_read(isa_mmu_translate(addr,len,MEM_TYPE_READ), len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  if(isa_mmu_check(addr, len, MEM_TYPE_WRITE) == MMU_DIRECT){
    paddr_write(addr, len, data);
    return;
  }
  paddr_write(isa_mmu_translate(addr,len,MEM_TYPE_WRITE), len, data);
}
