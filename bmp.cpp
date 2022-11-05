#include <iostream>
#include <fstream>
#include "bmp.h"

using namespace std;

BMPFile *read_bmp(const char *path)
{
  ifstream file(path);
  if (!file.is_open())
  {
    cout << "Error: Couldn't open file " << path << endl;
    return nullptr;
  }
  BMPFile *bmp = new BMPFile{};
  file.read((char *)bmp, sizeof(BMPHeader));
  bmp->data = new unsigned char[bmp->header.image_size_bytes];
  file.read((char *)bmp->data, bmp->header.image_size_bytes);
  file.close();
  return bmp;
}

bool write_bmp(const char *path, BMPFile *bmp)
{
  ofstream file(path);
  if (!file.is_open())
  {
    cout << "Error: Fail to open file " << path << endl;
    return false;
  }
  file.write((const char *)bmp, sizeof(BMPHeader));
  file.write((const char *)bmp->data, bmp->header.image_size_bytes);
  file.close();
  return true;
}

void release_bmp(BMPFile *bmp)
{
  if (bmp)
  {
    if (bmp->data)
      delete[] bmp->data;
    delete bmp;
  }
}
