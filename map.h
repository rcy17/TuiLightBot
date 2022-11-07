#pragma once
#include "const.h"
enum Direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

struct Position
{
  int x, y; // x 表示列号，y 表示行号
};

struct Map
{
  char path[MAX_PATH_LEN];  // 地图的文件路径
  int height[MAX_M][MAX_N]; // 记录地图各个格子的高度
  int m, n, q, p;           // 地图的行数、列数，灯数，过程数
  int op_limit[MAX_PROCS];  // 记录各个过程序列长度限制
  Position lights[MAX_Q];   // 记录各个灯的位置
  int lightened[MAX_Q];     // 记录各个灯是否被点亮
  Position robot;           // 机器人当前位置
  Direction dir;            // 机器人朝向是一个枚举值以增强代码可读性
};

inline bool same_position(const Position &a, const Position &b)
{
  return a.x == b.x && a.y == b.y;
}

void print_map(Map *map);
int load_map(const char *path);
const char *direction_string(Direction dir);
