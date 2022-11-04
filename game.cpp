#include <iostream>
#include <cstring>

#include "game.h"
#include "opseq.h"
#include "run.h"

using namespace std;

int running = 1;

int check_params(const char *cmd, int expected, int real, bool allow_default = false)
{
  if (expected != real && expected != real + allow_default)
  {
    cout << "Error: Wrong params for " << cmd << " command! ";
    cout << "Expect " << expected << " but got " << (real > expected ? "more" : "less") << endl;
    return 0;
  }
  return true;
}

void print_status()
{
  cout << "Map: " << game.map->path << endl;
  cout << "Autosave: " << game.autosave << endl;
  cout << "Next Save ID: " << game.next_save_id << endl;
  cout << "Steps Limit: " << game.limit << endl;
  print_map(game.map);
}

bool check_path(const char *path, bool isTemplate)
{
  for (char *p = (char *)path; *p; p++)
  {
    if (('0' <= *p && *p <= '9') || ('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z') || *p == '/' || *p == '.' || *p == '_' || (isTemplate && *p == '%' && p[1] == 'd'))
      continue;
    cout << "Error: Wrong format of path " << (isTemplate ? "template " : "") << endl;
    return false;
  }
  if (isTemplate && !strstr(path, "%d"))
    // path template must contain %d
    return false;
  return true;
}

int read_int(const char *s)
{
  int p = 0;
  char c;
  if (sscanf(s, "%d%c", &p, &c) != 1)
    return -1;
  return p;
}

void parse_command(const char *s)
{
  int p;
  char cmd[16] = "";
  char param[MAX_PATH_LEN] = "";
  char garbage[MAX_PATH_LEN] = "";
  int args = sscanf(s, "%16s%128s%s", cmd, param, garbage);
  if (args <= 0)
    return;
  if (strcmp(cmd, "LOAD") == 0)
  {
    if (!check_params(cmd, 2, args) || !check_path(param))
      return;
    load_map(param);
    print_map(game.map);
  }
  else if (strcmp(cmd, "NEW") == 0)
  {
    if (!check_params(cmd, 2, args))
      return;
    if (!new_map())
    {
      print_map(game.map);
    }
  }
  else if (strcmp(cmd, "AUTOSAVE") == 0)
  {
    if (!check_params(cmd, 2, args) || (!check_path(param, true) && strcmp(param, "OFF")))
      return;
    strcpy(game.autosave, param);
  }
  else if (strcmp(cmd, "OP") == 0)
  {
    if (!check_params(cmd, 2, args, true) || !check_path(param))
      return;
    if (args == 1)
      strcpy(param, "main.ops");
    new_opseq(param);
  }
  else if (strcmp(cmd, "LIMIT") == 0)
  {
    if (!check_params(cmd, 2, args))
      return;
    int new_limit = read_int(param);
    if (new_limit > 0)
      game.limit = new_limit;
    else
      cout << "Error: failed to parse '" << s << "' as an positive int" << endl;
  }
  else if (strcmp(cmd, "RUN") == 0)
  {
    if (!check_params(cmd, 2, args, true) || !check_path(param))
      return;
    if (args == 1)
      strcpy(param, "main.ops");
    run(param);
  }
  else if (strcmp(cmd, "EXIT") == 0)
  {
    if (!check_params(cmd, 1, args))
      return;
    cout << "Goodbye!" << endl;
    running = 0;
  }
  else if (strcmp(cmd, "STATUS") == 0)
  {
    if (!check_params(cmd, 1, args))
      return;
    print_status();
  }
  else
  {
    cout << "Error: command " << cmd << " not found!" << endl;
  }
}

void read_command()
{
  cout << "TuiLightBot> ";
  cout.flush();
  char s[512];
  cin.getline(s, 512);
  parse_command(s);
}

int run_game()
{
  game.map = new Map{};
  game.map_run = new Map{};
  if (load_map("main.map"))
  {
    cerr << "Fail to load map main.map, you must create it or load another one" << endl;
  }
  print_map(game.map);
  while (running)
  {
    read_command();
  }
  delete game.map;
  delete game.map_run;
  return 0;
}
