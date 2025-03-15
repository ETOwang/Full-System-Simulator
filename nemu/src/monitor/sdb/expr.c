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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include "memory/vaddr.h"

enum
{
  TK_NOTYPE = 256,
  TK_EQ,
  TK_DECIMAL,
  TK_HEX,
  TK_NEQ,
  TK_AND,
  TK_DEREF,
  TK_REG,
  TK_NEG

  /* TODO: Add more token types */

};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    // stupid error!{"0x[0-9]+", TK_HEX},
    {"0x[0-9a-fA-F]+", TK_HEX}, // hex
    {" +", TK_NOTYPE},          // spaces
    {"\\+", '+'},               // plus
    {"-", '-'},                 // minus
    {"\\*", '*'},               // multiply
    {"/", '/'},                 // div
    {"\\(", '('},               // left parenthesis
    {"\\)", ')'},               // right parenthesis
    {"[0-9]+", TK_DECIMAL},     // decimal
    {"==", TK_EQ},              // equal
    {"!=", TK_NEQ},             // not equal
    {"&&", TK_AND},             // and
    {"\\$[\\$a-z0-9]+", TK_REG} // reg
};
static int priority[] = {
    [TK_DEREF] = 4,
    [TK_NEG] = 4,
    ['+'] = 2,
    ['-'] = 2,
    ['*'] = 3,
    ['/'] = 3,
    [TK_EQ] = 1,
    [TK_NEQ] = 1,
    [TK_AND] = 0,
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

static Token tokens[32000] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //  i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
          break;
        case TK_NEQ:
        case TK_AND:
        case TK_EQ:
        case TK_NEG:
        case '(':
        case ')':
        case '*':
        case '-':
        case '/':
        case '+':
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;
        case TK_HEX:
        case TK_DECIMAL:
        case TK_REG:
          tokens[nr_token].type = rules[i].token_type;
          Assert(substr_len <= 31, "%s", "too long numbers!");
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          Assert(nr_token < 32000, "%s", "too many tokens!");
          break;
        default:
          Assert(0, "Invalid Token %s", substr_start);
        }
        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
bool check_valid(word_t p, word_t q)
{
  sword_t sum = 0;
  for (word_t i = p; i <= q; i++)
  {
    if (tokens[i].type == '(')
    {
      sum++;
    }
    else if (tokens[i].type == ')')
    {
      sum--;
      if (sum < 0)
      {
        return false;
      }
    }
  }
  return sum == 0;
}
bool check_parentheses(word_t p, word_t q, bool *success)
{
  if (tokens[p].type == '(' && tokens[q].type == ')')
  {
    if (!check_valid(p, q))
    {
      if (success)
      {
        Assert(0, "Invalid expression");
        *success = false;
      }

      return false;
    }
    sword_t sum = 0;
    for (word_t i = p + 1; i <= q - 1; i++)
    {
      if (tokens[i].type == '(')
      {
        sum++;
      }
      else if (tokens[i].type == ')')
      {
        sum--;
        if (sum < 0)
        {
          if (success)
          {
            *success = true;
          }

          return false;
        }
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}
word_t find_right_parenthese(word_t p, word_t q)
{
  int sum = 1;
  for (word_t i = p + 1; i <= q; i++)
  {
    if (tokens[i].type == ')')
    {
      sum--;
      if (sum == 0)
      {
        return i + 1;
      }
    }
    else if (tokens[i].type == '(')
    {
      sum++;
    }
  }
  return -1;
}
word_t find_main_op(word_t p, word_t q)
{
  word_t i = p;
  word_t pos = -1;
  for (; i <= q;)
  {
    if (tokens[i].type == '(')
    {
      i = find_right_parenthese(i, q);
    }
    else
    {
      if (tokens[i].type == TK_DECIMAL || tokens[i].type == TK_HEX || tokens[i].type == TK_REG)
      {
        i++;
      }
      else
      {
        if (pos == -1)
        {
          pos = i;
        }
        else
        {
          if (priority[tokens[i].type] <= priority[tokens[pos].type])
          {
            pos = i;
          }
        }
        i++;
      }
    }
  }
  Assert(pos != -1, "Error main operator");
  return pos;
}
word_t eval(word_t p, word_t q, bool *success)
{
  if (p > q)
  {
    *success = false;
    return -1;
  }
  else if (p == q)
  {

    char *endpoint = NULL;
    word_t res;
    if (tokens[p].type == TK_DECIMAL)
    {
      res = strtoul(tokens[p].str, &endpoint, 10);
    }
    else if (tokens[p].type == TK_HEX)
    {
      res = strtoul(tokens[p].str, &endpoint, 16);
    }
    else
    {
      res = isa_reg_str2val(tokens[p].str + 1, success);
    }
    if (endpoint != NULL && *endpoint != '\0')
    {
      *success = false;
    }
    return res;
  }
  else if (check_parentheses(p, q, success))
  {
    if (!(*success))
    {
      return -1;
    }
    return eval(p + 1, q - 1, success);
  }
  else
  {
    word_t pos = find_main_op(p, q);
    switch (tokens[pos].type)
    {
    case '+':
      return eval(p, pos - 1, success) + eval(pos + 1, q, success);
    case '-':
      return eval(p, pos - 1, success) - eval(pos + 1, q, success);
    case '*':
      return eval(p, pos - 1, success) * eval(pos + 1, q, success);
    case '/':
      return eval(p, pos - 1, success) / eval(pos + 1, q, success);
    case TK_DEREF:
      word_t addr = eval(p + 1, q, success);
      word_t cur = vaddr_read(addr, 4);
      return cur;
    case TK_NEG:
      return -eval(p + 1, q, success);
    case TK_EQ:
      return eval(p, pos - 1, success) == eval(pos + 1, q, success);
    case TK_NEQ:
      return eval(p, pos - 1, success) != eval(pos + 1, q, success);
    case TK_AND:
      return eval(p, pos - 1, success) && eval(pos + 1, q, success);
    default:
      Assert(0, "Error main operator %s", tokens[pos].str);
    }
  }
}

word_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }
  for (word_t i = 0; i < nr_token; i++)
  {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '(' || tokens[i - 1].type == '-' || tokens[i - 1].type == '+' || tokens[i - 1].type == '*' || tokens[i - 1].type == '/' || tokens[i - 1].type == TK_EQ || tokens[i - 1].type == TK_AND || tokens[i - 1].type == TK_NEQ))
    {
      tokens[i].type = TK_DEREF;
    }
  }
  for (word_t i = 0; i < nr_token; i++)
  {
    if (tokens[i].type == '-' && (i == 0 || tokens[i - 1].type == '(' || tokens[i - 1].type == '-' || tokens[i - 1].type == '+' ||
                                  tokens[i - 1].type == '*' || tokens[i - 1].type == '/' ||
                                  tokens[i - 1].type == TK_EQ || tokens[i - 1].type == TK_AND || tokens[i - 1].type == TK_NEQ || tokens[i - 1].type == TK_NEG || tokens[i - 1].type == TK_DEREF))
    {
      tokens[i].type = TK_NEG;
    }
  }
  return eval(0, nr_token - 1, success);
}
