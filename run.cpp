#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "game.h"
#include "run.h"
#include "opseq.h"
#include "ui.h"

using namespace std;

OpSeq *read_opseq(const char *path)
{
  ifstream file(path);
  if (!file.is_open())
  {
    cout << "Error: Couldn't open file " << path << endl;
    return nullptr;
  }
  OpSeq *os = new OpSeq{};
  strcpy(os->path, path);
  int proc;
  for (proc = 0;; proc++)
  {
    int op_count = 0;
    char s[MAX_PATH_LEN * MAX_OP_LEN * 2] = "";
    file.getline(s, sizeof(s));
    // check end of file
    // using < 2 in case of get a single '\n'
    if (strlen(s) < 2)
      break;
    istringstream ss(s);
    ss >> os->procs[proc].name;
    while (1)
    {
      char cmd[16] = "";
      char param[MAX_PATH_LEN];
      ss >> cmd;
      OpType otp = string_to_optype(cmd);
      // check end of line
      if (otp == UNKNOWN_TYPE)
        break;
      os->procs[proc].ops[op_count].otp = SAVE;
      if (otp == SAVE)
      {
        ss >> param;
        strcpy(os->procs[proc].ops[op_count].param, param);
      }
      strcpy(os->procs[proc].ops[op_count].param, cmd);
      os->procs[proc].ops[op_count].otp = otp;
      op_count++;
    }
    os->procs[proc].count = op_count;
  }
  os->count = proc;
  return os;
}

struct Executor
{
  int proc = 0;
  int op_offset = 0;
};

Direction turn_left(Direction d)
{
  Direction map[4] = {LEFT, RIGHT, DOWN, UP};
  return map[d];
}

Direction turn_right(Direction d)
{
  Direction map[4] = {RIGHT, LEFT, UP, DOWN};
  return map[d];
}

Position move(Map *map)
{
  Direction d = map->dir;
  Position p = map->robot;
  int dx[4] = {0, 0, -1, 1};
  int dy[4] = {-1, 1, 0, 0};
  Position newp = {p.x + dx[d], p.y + dy[d]};
  if (newp.x < 0 || map->n <= newp.x || newp.y < 0 || map->m <= newp.y ||
      map->height[newp.y][newp.x] != map->height[p.y][p.x])
  {
    cout << "Warning: Robot can't move from (" << p.x << "," << p.y << ") to " << direction_string(d) << endl;
    return p;
  }
  return newp;
}

Position jump(Map *map)
{
  Direction d = map->dir;
  Position p = map->robot;
  int dx[4] = {0, 0, -1, 1};
  int dy[4] = {-1, 1, 0, 0};
  Position newp = {p.x + dx[d], p.y + dy[d]};
  if (newp.x < 0 || map->n <= newp.x || newp.y < 0 || map->m <= newp.y ||
      abs(map->height[newp.y][newp.x] - map->height[p.y][p.x]) != 1)
  {
    cout << "Warning: Robot can't jump from (" << p.x << "," << p.y << ") to " << direction_string(d) << endl;
    return p;
  }
  return newp;
}

bool light_up(Map *map)
{
  Position p = map->robot;
  for (int i = 0; i < map->q; i++)
  {
    if (p.x == map->lights[i].x && p.y == map->lights[i].y)
    {
      if (map->lightened[i])
      {
        cout << "Warning: The light at (" << p.x << "," << p.y << ") is already lightened." << endl;
        return false;
      }
      map->lightened[i] = true;
      return true;
    }
  }
  cout << "Warning: There is no light at (" << p.x << "," << p.y << ")." << endl;
  return false;
}

int call(OpSeq *os, const char *name)
{
  int proc;
  for (proc = 0; proc < os->count; proc++)
    if (strcmp(os->procs[proc].name, name) == 0)
      return proc;
  return -1;
}

bool done(Map *map)
{
  for (int i = 0; i < map->q; i++)
    if (!map->lightened[i])
      return false;
  return true;
}

Result robot_run(const char *path)
{
  OpSeq *os = read_opseq(path);
  if (os == NULL)
    return {0, FILE_NOT_FOUND};
  memcpy(game.map_run, game.map, sizeof(Map));
  Map *map = game.map_run;
  int steps = 0; // count of robot moves
  int count = 0; // count of operations for limit
  // call stack can't has more elements than operation limit
  Executor *call_stack = new Executor[game.limit]{};
  int stack_len = 1;
  while (!done(map) && stack_len && count < game.limit)
  {
    Executor *now = call_stack + stack_len - 1;
    if (now->op_offset == os->procs[now->proc].count)
    {
      // proc ends
      stack_len--;
      continue;
    }
    OpType otp = os->procs[now->proc].ops[now->op_offset].otp;
    switch (otp)
    {
    case TL:
      map->dir = turn_left(map->dir);
      break;
    case TR:
      map->dir = turn_right(map->dir);
      break;
    case MOV:
      map->robot = move(map);
      break;
    case JMP:
      map->robot = jump(map);
      break;
    case LIT:
      light_up(map);
      break;
    case CALL:
      call_stack[stack_len] = {call(os, os->procs[now->proc].ops[now->op_offset].param), 0};
      // it's a valid frame if and only if proc != -1
      if (call_stack[stack_len].proc >= 0)
        stack_len++;
      break;
    case SAVE:
      save(os->procs[now->proc].ops[now->op_offset].param);
      count--;
      break;
    default:
      // unreachable
      exit(-1);
    }
    now->op_offset++;
    count++;
    if (otp != SAVE && otp != CALL)
    {
      auto_save();
      steps++;
    }
  }
  delete os;
  ResultType r = done(map) ? LIGHTUP : (!stack_len ? HITEND : LIMIT);
  return {steps, r};
}
