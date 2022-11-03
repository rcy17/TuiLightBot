#pragma once
#include "const.h"

enum OpType
{
  TL,
  TR,
  MOV,
  JMP,
  LIT,
  CALL,
  SAVE,
};

struct Op
{
  OpType otp;               // 操作类型是一个枚举值以增强可读性
  char param[MAX_PATH_LEN]; // 对于存图操作包含一个地址信息，对于过程调用操作包含过程名
};

struct Proc
{
  char name[MAX_NAME_LEN]; // 记录过程名，由于数据量不大，寻找过程时直接根据过程名匹配即可
  Op ops[MAX_OP_LEN];      // 过程包含一系列操作
};

struct OpSeq
{
  char path[MAX_PATH_LEN]; // 操作序列的文件路径
  Proc procs[MAX_PROCS];   // 一个操作序列由一系列过程组成
};
