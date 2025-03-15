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

#include "sdb.h"
#include <cpu/cpu.h>
#define NR_WP 32

typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;
  char *expr;
  word_t pre;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static word_t counter = 0;

void init_wp_pool()
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP *new_wp(char *expression)
{
  Assert(free_ != NULL, "No free watchpoint");
  WP *res = free_;
  free_ = free_->next;
  if (head == NULL)
  {
    head = res;
    res->next = NULL;
  }
  else
  {
    WP *next = head->next;
    head->next = res;
    res->next = next;
  }
  res->NO = counter++;
  res->expr = malloc((strlen(expression) + 1) * sizeof(char));
  strcpy(res->expr, expression);
  res->expr[strlen(expression)] = '\0';
  bool *success = malloc(sizeof(bool));
  *success = true;
  res->pre = expr(expression, success);
  if (!(*success))
  {
    free(success);
    return NULL;
  }
  free(success);
  return res;
}
void free_wp(WP *wp)
{
  WP *pre = NULL;
  WP *cur = head;
  while (cur != NULL && cur != wp)
  {
    pre = cur;
    cur = cur->next;
  }
  Assert(cur == wp, "Invalid watchpoint");
  free(wp->expr);
  if (pre == NULL)
  {
    head = head->next;
  }
  else
  {
    pre->next = cur->next;
  }
  if (free_ == NULL)
  {
    free_ = wp;
    free_->next = NULL;
    return;
  }
  WP *next = free_->next;
  free_ = wp;
  free_->next = next;
}

bool delete_wp(word_t index)
{
  WP *cur = head;
  while (cur != NULL && cur->NO != index)
  {
    cur = cur->next;
  }
  if (cur == NULL)
  {
    return false;
  }
  free_wp(cur);
  return true;
}

void check_watchpoint()
{
  WP *cur = head;
  while (cur != NULL)
  {
    bool *success = malloc(sizeof(bool));
    *success = true;
    word_t cur_expr = expr(cur->expr, success);
    free(success);
    if (cur_expr != cur->pre)
    {
      printf("\nWatchpoint %d: %s\n", cur->NO, cur->expr);
      printf("\nOld value = %u\n", cur->pre);
      printf("\nNew value = %u\n", cur_expr);
      cur->pre = cur_expr;
      // TODO:pc and halt_ret
      set_nemu_state(NEMU_STOP, 0, 0);
    }
    cur = cur->next;
  }
}

void print_watchpoints()
{
  WP *cur = head;
  printf("NO      Expr\n");
  while (cur != NULL)
  {
    printf("%u       %-10s\n", cur->NO, cur->expr);
    cur = cur->next;
  }
}
