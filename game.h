#pragma once
#include "map.h"
struct Game
{
  Map *map, *map_run;          // 指向当前地图与机器人执行完后的地图的结构指针
  char autosave[MAX_PATH_LEN]; // 当前设定的自动存图路径
  int next_save_id;            // 记录在启用自动存图功能时下一张图路径中 % d 对应的值
  int limit;                   // 当前设定的RUN的限制操作数
};
extern Game game; // 全局唯一的 Game 结构变量

int run_game();
bool check_path(const char *path, bool isTemplate = false);
