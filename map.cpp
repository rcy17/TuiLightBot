#include <iostream>
#include <fstream>
#include <cstring>

#include "map.h"
#include "game.h"

using namespace std;

const char *
format_string(int height, bool isBot = false, bool isLightened = false, bool isUnlightened = false)
{
  if (height == 0)
  {
    // default color for blank cell
    return "\e[0m";
  }
  int bg_color = isLightened ? 103 : (isUnlightened ? 104 : 100);
  int fg_color = isBot ? 91 : 92;
  // using meta programing maybe cool here, but i didn't learn that
  static char color[100] = "";
  sprintf(color, "\e[%d;%d;1m", bg_color, fg_color);
  return color;
}

const char *direction_string(Direction dir)
{
  const char *dirs[] = {"up", "down", "left", "right"};
  return dirs[dir];
}

Direction int_direction(int d)
{
  switch (d)
  {
  case 0:
    return UP;
  case 1:
    return DOWN;
  case 2:
    return LEFT;
  case 3:
    return RIGHT;
  default:
    exit(-1);
  }
}

void print_map(Map *map)
{
  // print the whole map
  for (int r = 0; r < map->m; r++)
  {
    for (int c = 0; c < map->n; c++)
    {
      if (!map->height[r][c])
      {
        cout << format_string(0) << ' ';
      }
      else
      {
        bool isBot = same_position(map->robot, {c, r});
        bool isLightened = false;
        bool isUnlightened = false;
        for (int i = 0; i < map->q; i++)
        {
          if (same_position(map->lights[i], {c, r}))
          {
            (map->lightened[i] ? isLightened : isUnlightened) = true;
            break;
          }
        }
        cout << format_string(map->height[r][c], isBot, isLightened, isUnlightened) << map->height[r][c];
      }
    }
    cout << format_string(0) << endl;
  }
  // report other info
  cout << "Robot faces " << direction_string(map->dir) << endl;
  cout << "Processing limit: [";
  for (int i = 0; i < map->p; i++)
  {
    cout << (i ? ", " : "") << map->op_limit[i];
  }
  cout << "]" << endl;
}

int load_map(const char *path)
{
  ifstream infile(path);
  Map *map = game.map;
  if (!infile.is_open())
  {
    cerr << "Can't find file " << path << endl;
    return 1;
  }
  strcpy(map->path, path);
  infile >> map->m >> map->n >> map->q >> map->p;
  for (int r = 0; r < map->m; r++)
  {
    for (int c = 0; c < map->n; c++)
    {
      infile >> map->height[r][c];
    }
  }
  for (int i = 0; i < map->q; i++)
  {
    infile >> map->lights[i].x >> map->lights[i].y;
  }
  for (int i = 0; i < map->p; i++)
  {
    infile >> map->op_limit[i];
  }
  int dir;
  infile >> map->robot.x >> map->robot.y >> dir;
  map->dir = int_direction(dir);
  return 0;
}

int new_map()
{
  Map map = {};
  memcpy(&game.map, &map, sizeof(map));
  return 0;
}
