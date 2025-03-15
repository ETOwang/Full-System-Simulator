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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[655360] = {};
static char code_buf[655360 + 1280] = {}; // a little larger than `buf`
const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
static char *code_format =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned $$0=0;"
    "  unsigned $ra=0;"
    "  unsigned $sp=0;"
    "  unsigned $gp=0;"
    "  unsigned $tp=0;"
    "  unsigned $a0=0;"
    "  unsigned $a1=0;"
    "  unsigned $a2=0;"
    "  unsigned $a3=0;"
    "  unsigned $a4=0;"
    "  unsigned $a5=0;"
    "  unsigned $a6=0;"
    "  unsigned $a7=0;"
    "  unsigned $s0=0;"
    "  unsigned $s1=0;"
    "  unsigned $s2=0;"
    "  unsigned $s3=0;"
    "  unsigned $s4=0;"
    "  unsigned $s5=0;"
    "  unsigned $s6=0;"
    "  unsigned $s7=0;"
    "  unsigned $s8=0;"
    "  unsigned $s9=0;"
    "  unsigned $s10=0;"
    "  unsigned $s11=0;"
    "  unsigned $t0=0;"
    "  unsigned $t1=0;"
    "  unsigned $t2=0;"
    "  unsigned $t3=0;"
    "  unsigned $t4=0;"
    "  unsigned $t5=0;"
    "  unsigned $t6=0;"
    "  unsigned result = %s; "
    "  printf(\"%%u\", result); "
    "  return 0; "
    "}";
static int count = 0;
static void gen_num()
{
  if (rand() % 2)
  {
    int n = sprintf(buf + count, "%d", rand() % 2000000000);
    count += n;
    buf[count++] = 'u';
  }else{
    buf[count++]='$';
    int n = sprintf(buf + count, "%s", regs[rand()%32]);
    count += n;
  }
}
static void gen(char c)
{
  buf[count++] = c;
}

static int choose(int len)
{
  return rand() % 3;
}
static void gen_rand_op()
{
  char ops[] = {'+', '-', '*', '/'};
  // char *oprerator[] = {"==", "!=", "&&"};
  int index = rand() % 3;
  if (index <= 3)
  {
    buf[count++] = ops[index];
  }
  // else
  // {
  //   buf[count++] = oprerator[index - 4][0]; // 随机选择一个运算符并存储到 buf 中
  //   buf[count++] = oprerator[index - 4][1];
  // }
}
static void gen_rand_expr()
{
  int choice = choose(3);
  if (count >= 500)
  {
    choice = 0;
  }
  switch (choice)
  {
  case 0:
    gen_num();
    break;
  case 1:
    gen('(');
    gen_rand_expr();
    gen(')');
    break;
  default:
    gen_rand_expr();
    gen_rand_op();
    gen_rand_expr();
    break;
  }
}

int main(int argc, char *argv[])
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1)
  {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i++)
  {
    count = 0;
    gen_rand_expr();
    if (buf[count] != '\0')
    {
      buf[count] = '\0';
    }
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    fp = popen("gcc -Werror /tmp/.code.c -o /tmp/.expr", "r");
    int ret = pclose(fp);
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    ret = pclose(fp);
    if (ret != 0)
    {
      continue;
    }
    ret = system("rm /tmp/.code.c ");
    printf("%u %s\n", result, buf);
  }
  return 0;
}
