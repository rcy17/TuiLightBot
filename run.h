#include "game.h"

enum ResultType
{
  HITEND,         // MAIN 过程执行完毕，但灯没能全部点亮
  LIMIT,          // 到达操作数上限
  LIGHTUP,        // 点亮了全部灯，干得漂亮
  FILE_NOT_FOUND, // 指令文件不存在
};

struct Result
{
  int steps;         // 记录总步数
  ResultType result; // 用enum记录结束原因
};

Result robot_run(const char *path);
