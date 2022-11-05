#include <iostream>
#include <cstring>

#include "ui.h"
#include "bmp.h"
#include "game.h"

using namespace std;

Image robots[4];
void write_into_file(const char *path, Image *image);
Image *draw();

void release_image(Image *img)
{
  if (img)
  {
    if (img->data)
      delete[] img->data;
    delete img;
  }
}

Image *read_from_file(const char *path)
{
  BMPFile *bmp = read_bmp(path);
  if (!bmp)
  {
    cout << "Error: Couldn't open file" << path << endl;
    return nullptr;
  }
  Image *img = new Image{};
  int h = bmp->header.height_px, w = bmp->header.width_px;
  img->height = h, img->width = w;
  if (w * 3 % 4)
  {
    int line_length = w * 3 + 4 - w * 3 % 4;
    img->data = new Pixel[h * w];
    for (int r = 0; r < h; r++)
      memcpy(img->data + r * w, bmp->data + r * line_length, w * sizeof(Pixel));
  }
  else
  {
    img->data = (Pixel *)bmp->data;
    bmp->data = nullptr;
  }
  release_bmp(bmp);
  return img;
}

void init_assets()
{
  Image *img = read_from_file(ASSET_PATH);
  if (!img)
  {
    cout << "Fatal: Couldn't open file" << ASSET_PATH << endl;
    exit(-1);
  }
  // test
  write_into_file("assets/aaa.bmp", img);
  int H = img->height, W = img->width;
  int h = H / 2, w = W / 2;
  Pixel *temp = new Pixel[h * w];
  for (int i = 0; i < 4; i++)
  {
    int pr = i / 2, pc = i % 2;
    for (int r = 0; r < h; r++)
      memcpy(&temp[r * w], &img->data[(pr * h + r) * W + w * pc], sizeof(Pixel) * w);
    // assuming that we need convert 350*700 into 35*70
    robots[i].width = w / 10;
    robots[i].height = h / 10;
    robots[i].data = new Pixel[h * w / 100];
    for (int row = 0; row < h / 10; row++)
      for (int col = 0; col < w / 10; col++)
      {
        int r = 0, g = 0, b = 0;
        for (int i = 0; i < 10; i++)
          for (int j = 0; j < 10; j++)
          {
            r += temp[(row * 10 + i) * w + col * 10 + j].r;
            g += temp[(row * 10 + i) * w + col * 10 + j].g;
            b += temp[(row * 10 + i) * w + col * 10 + j].b;
          }
        robots[i].data[row * w / 10 + col] = {(unsigned char)(r / 100), (unsigned char)(g / 100), (unsigned char)(b / 100)};
      }
    // test
    char s[MAX_PATH_LEN];
    sprintf(s, "assets/%d.bmp", i);
    write_into_file(s, &robots[i]);
  }
}

void release_assets()
{
  for (int i = 0; i < 4; i++)
    delete[] robots[i].data;
}

void write_into_file(const char *path, Image *image)
{
  BMPFile file{};
  file.data = (unsigned char *)image->data;
  unsigned h = image->height;
  unsigned w = image->width;
  file.header = {
      0x4d42, (uint32)(h * w * sizeof(Pixel) + sizeof(BMPHeader)), 0, 0, sizeof(BMPHeader),
      0x28, w, h, 1, sizeof(Pixel) * 8, 0, (uint32)(h * w * sizeof(Pixel)), 0, 0, 0, 0};

  int line_length = w * 3;
  if (line_length % 4)
  {
    // fix padding issue is real annoying
    line_length += 4 - w * 3 % 4;
    const char padding[4] = {};
    file.data = new unsigned char[line_length * h]{};
    for (int r = 0; r < h; r++)
      memcpy((void *)(file.data + r * line_length), image->data + r * w, w * sizeof(Pixel));
    file.header.image_size_bytes = h * line_length;
    file.header.size = h * line_length + sizeof(BMPHeader);
    write_bmp(path, &file);
    delete[] file.data;
  }
  else
    write_bmp(path, &file);
}

Image *draw()
{
  // each cell is 100p*50p
  // each height is 25p
  Map *map = game.map_run;
  int h = 50 * MAX_M + 100;
  int w = 100 * MAX_N + 25 * MAX_H;
  Image *img = new Image{h, w};
  img->data = new Pixel[h * w];
  memset(img->data, 0x7f, h * w * 3);
  return img;
}

void save(const char *path)
{
  Image *img = draw();
  write_into_file(path, img);
  delete img->data;
}

void auto_save()
{
  if (!strcmp(game.autosave, "OFF"))
    return;
  char path[MAX_PATH_LEN] = "";
  char *p = game.autosave;
  char number[16];
  sprintf(number, "%d", game.next_save_id);
  game.next_save_id++;
  // assuming only one %d in path
  p = strstr(game.autosave, "%d");
  strncpy(path, game.autosave, p - game.autosave);
  strcat(path, number);
  strcat(path, p + 2);
  save(path);
}