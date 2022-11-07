#include <iostream>
#include <fstream>
#include <cstring>

#include "opseq.h"
#include "game.h"

using namespace std;

bool check_proc_name(const char *s, int p)
{
  // MAIN or P<n>
  if (!strcmp(s, "MAIN"))
    return true;
  return s[0] == 'P' && '0' <= s[1] && s[1] <= '0' + p && !s[2];
}

bool check_file_exists(const char *path)
{
  ifstream ifs(path);
  if (ifs.is_open())
  {
    cout << "File " << path << " already exists, would you like to overwrite? (Input Y to continue): ";
    cout.flush();
    char buff[MAX_PATH_LEN];
    cin.getline(buff, MAX_PATH_LEN);
    if (buff[0] != 'Y')
      return false;
  }
  return true;
}

const char *optype_string(OpType op)
{
  const char *ops[] = {"TL", "TR", "MOV", "JMP", "LIT", "CALL", "SAVE"};
  return ops[op];
}

OpType string_to_optype(const char *s)
{
  if (!strcmp(s, "TL"))
    return TL;
  if (!strcmp(s, "TR"))
    return TR;
  if (!strcmp(s, "MOV"))
    return MOV;
  if (!strcmp(s, "JMP"))
    return JMP;
  if (!strcmp(s, "LIT"))
    return LIT;
  if (!strcmp(s, "SAVE"))
    return SAVE;
  if (check_proc_name(s, MAX_PROCS))
    return CALL;
  return UNKNOWN_TYPE;
}

OpSeq *read_opseq()
{
  OpSeq *os = new OpSeq{};
  int proc = 0;
  int op_count = 0;
  int op_limited_count = 0;
  int on = 1;
  Map *map = game.map;
  int proc_count = map->p;
  strcpy(os->procs[0].name, "MAIN");
  for (int i = 1; i < proc_count; i++)
    sprintf(os->procs[i].name, "P%d", i);
  while (on)
  {
    cout << "[" << os->procs[proc].name << "] ";
    cout.flush();
    char s[MAX_PATH_LEN];
    cin.getline(s, MAX_PATH_LEN);
    char cmd[16];
    char param[MAX_PATH_LEN];
    char garbage[MAX_PATH_LEN];
    int args = sscanf(s, "%16s%128s%s", cmd, param, garbage);
    if (args <= 0)
      continue;
    OpType otp = string_to_optype(cmd);
    if (otp == UNKNOWN_TYPE)
    {
      if (!strcmp(cmd, "END"))
      {
        os->procs[proc].count = op_count;
        on = 0;
      }
      else if (!strcmp(cmd, "ABORT"))
      {
        delete os;
        os = nullptr;
        on = 0;
      }
      else if (!strcmp(cmd, "PROC"))
      {
        if (args != 2)
        {
          cout << "Error: cmd " << cmd << " should have excctly one argument" << endl;
          continue;
        }
        if (!check_proc_name(param, map->p))
        {
          cout << "Error: " << param << " is not a legal proc name" << endl;
          continue;
        }
        int old_proc = proc;
        for (proc = 0; proc < proc_count; proc++)
        {
          if (!strcmp(param, os->procs[proc].name))
            break;
        }
        if (proc == proc_count)
        {
          cout << "Error: procs limit for map (" << map->p << ") is exceeded, fail to create new proc" << endl;
          continue;
        }
        os->procs[old_proc].count = op_count;
        op_limited_count = op_count = 0;
      }
      else
        cout << "Error: Unknown operation " << cmd << endl;
    }
    else if (otp == SAVE)
    {
      if (args != 2)
      {
        cout << "Error: cmd " << cmd << " should have excctly one argument" << endl;
        continue;
      }
      if (otp == SAVE && !check_path(param))
        continue;
      strcpy(os->procs[proc].ops[op_count].param, param);
      os->procs[proc].ops[op_count].otp = otp;
      op_count++;
      op_limited_count += otp == CALL;
    }
    else
    {
      if (args > 1)
      {
        cout << "Error: cmd " << cmd << " shouldn't have any argument" << endl;
        continue;
      }
      if (op_limited_count == map->op_limit[proc])
      {
        cout << "Error: op limit of proc in the map (" << map->op_limit[proc] << ") is exceeded" << endl;
        continue;
      }
      // just copy for call
      strcpy(os->procs[proc].ops[op_count].param, cmd);
      os->procs[proc].ops[op_count].otp = otp;
      op_count++, op_limited_count++;
    }
  }
  os->count = proc_count;
  return os;
}

int new_opseq(const char *path)
{
  if (!check_file_exists(path))
    return -1;
  OpSeq *os = read_opseq();
  if (os == nullptr)
    return -2;
  ofstream ofs(path);
  for (int i = 0; i < os->count; i++)
  {
    if (os->procs[i].count == 0 && i > 0)
      continue;
    ofs << os->procs[i].name;
    for (int j = 0; j < os->procs[i].count; j++)
    {
      OpType otp = os->procs[i].ops[j].otp;
      if (otp == CALL)
        ofs << ' ' << os->procs[i].ops[j].param;
      else if (otp == SAVE)
        ofs << " SAVE " << os->procs[i].ops[j].param;
      else
        ofs << ' ' << optype_string(os->procs[i].ops[j].otp);
    }
    ofs << endl;
  }
  delete os;
  return 0;
}
