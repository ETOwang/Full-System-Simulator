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

#ifndef __RISCV_REG_H__
#define __RISCV_REG_H__

#include <common.h>
#define MTVEC 0x305
#define MCAUSE 0x342
#define MEPC 0x341
#define MSTATUS 0x300
#define SATP 0x180

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
  return idx;
}

static inline word_t* get_csr(int idx) {
  switch (idx)
  {
  case MTVEC:
     return &cpu.csr.mtvec;
  case MCAUSE:
      return &cpu.csr.mcause;
  case MEPC:
      return &cpu.csr.mepc;
  case MSTATUS:
      return &cpu.csr.mstatus.val;
  case SATP:
      return &cpu.csr.satp.val;
  default:
      assert(0);
  }
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])
#define csr(idx) (*get_csr(idx))

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

#endif
