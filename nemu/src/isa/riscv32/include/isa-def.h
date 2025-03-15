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

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef struct
{
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  struct
  {
    union
    {
      word_t val;
      struct
      {
        word_t padding1 : 3;
        word_t mie:1;
        word_t padding2:3;
        word_t mpie:1;
        word_t padding3:3;
        word_t mpp : 2;
        word_t padding4 : 5;
        word_t sum : 1;
        word_t mxr : 1;
        word_t padding5 : 12;
      };
    } mstatus;
    word_t mepc;
    word_t mcause;
    word_t mtvec;
    union
    {
      word_t val;
      struct
      {
        word_t ppn : 22;
        word_t asid : 8;
        word_t mode : 2;
      };
    } satp;
  } csr;
  vaddr_t pc;
  bool INTR;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);
// decode
typedef struct
{
  uint32_t inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

#endif
