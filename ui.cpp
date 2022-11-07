#include <iostream>
#include <cstring>
#include <cassert>
#include <cmath>

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
    // notice: bmp +y direction is different with image
    for (int r = 0; r < h; r++)
      memcpy(img->data + (h - 1 - r) * w, bmp->data + r * line_length, w * sizeof(Pixel));
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
  int H = img->height, W = img->width;
  int h = H / 2, w = W / 2;
  Pixel *temp = new Pixel[h * w];
  for (int i = 0; i < 4; i++)
  {
    int pr = i / 2, pc = i % 2;
    for (int r = 0; r < h; r++)
      memcpy(&temp[r * w], &img->data[(pr * h + r) * W + w * pc], sizeof(Pixel) * w);
    // assuming that we need convert 350*700 into 70*140
    robots[i].width = w / 5;
    robots[i].height = h / 5;
    robots[i].data = new Pixel[h * w / 25];
    for (int row = 0; row < h / 5; row++)
      for (int col = 0; col < w / 5; col++)
      {
        int r = 0, g = 0, b = 0;
        for (int i = 0; i < 5; i++)
          for (int j = 0; j < 5; j++)
          {
            r += temp[(row * 5 + i) * w + col * 5 + j].r;
            g += temp[(row * 5 + i) * w + col * 5 + j].g;
            b += temp[(row * 5 + i) * w + col * 5 + j].b;
          }
        robots[i].data[row * w / 5 + col] = {(unsigned char)(r / 25), (unsigned char)(g / 25), (unsigned char)(b / 25)};
      }
  }
  // before: [down, right, left, up]
  swap(robots[0], robots[1]);
  swap(robots[0], robots[3]);
  // after: [up, down, left, right]
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
    // notice: bmp +y direction is different with image
    for (int r = 0; r < h; r++)
      memcpy((void *)(file.data + r * line_length), image->data + (h - 1 - r) * w, w * sizeof(Pixel));
    file.header.image_size_bytes = h * line_length;
    file.header.size = h * line_length + sizeof(BMPHeader);
    write_bmp(path, &file);
    delete[] file.data;
  }
  else
    write_bmp(path, &file);
}

void draw_pixel(Image *img, Position p, Pixel color)
{
  img->data[p.y * img->width + p.x] = color;
}

void draw_line(Image *img, Position start, Position stop, Pixel color)
{
  // assert(start.x < stop.x || (start.x == start.y && start.y < stop.y));
  // (x1, y1) -> (x2, y2)
  //
  if (start.x == stop.x)
  {
    if (start.y > stop.y)
      swap(start, stop);
    for (int y = start.y; y <= stop.y; y++)
    {
      draw_pixel(img, {start.x, y}, color);
    }
  }
  else
  {
    // k = tan(pi/6)
    double k = 1 / 1.732;
    if (start.x > stop.x)
      swap(start, stop);
    if (start.y > stop.y)
      k = -k;
    for (int x = start.x; x <= stop.x; x++)
    {
      double y = start.y + (x - start.x) * k;
      draw_pixel(img, {x, int(ceil(y))}, color);
      draw_pixel(img, {x, int(floor(y))}, color);
    }
  }
}

void pad_cell(Image *img, Position center, int h, int w, Pixel color)
{
  // line cross (0, h) and (w, 0):
  // x/w + y/h = 1
  // => hx + wy = hw
  // => area: hx + wy <= hw
  for (int i = 0; i < 4; i++)
  {
    int px[4] = {1, 1, -1, -1};
    int py[4] = {1, -1, 1, -1};
    for (int dx = 0; dx < w; dx++)
    {
      for (int dy = 0; dy < h; dy++)
      {
        if (h * dx + w * dy > h * w)
          break;
        draw_pixel(img, {center.x + dx * px[i], center.y + dy * py[i]}, color);
      }
    }
  }
}

void pad_parallelogram(Image *img, Position start, int h, int w, int height)
{
  /* |\   /|
     | \ / |
     |  |  |
      \ | /
       \|/
  */
  for (int dy = 1; dy < height; dy++)
  {
    draw_line(img, {start.x, start.y + dy}, {start.x + w, start.y + dy - h}, CELL_RIGHT_COLOR);
    draw_line(img, {start.x, start.y + dy}, {start.x - w, start.y + dy - h}, CELL_LEFT_COLOR);
  }
}

void draw_cell(Image *img, Position center, int h, int w, Pixel color)
{
  pad_cell(img, center, h, w, color);
  draw_line(img, {center.x - w, center.y}, {center.x, center.y + h}, LINE_COLOR);
  draw_line(img, {center.x - w, center.y}, {center.x, center.y - h}, LINE_COLOR);
  draw_line(img, {center.x + w, center.y}, {center.x, center.y - h}, LINE_COLOR);
  draw_line(img, {center.x + w, center.y}, {center.x, center.y + h}, LINE_COLOR);
}

void draw_parallelogram(Image *img, Position start, int h, int w, int height)
{
  pad_parallelogram(img, start, h, w, height);
  draw_line(img, start, {start.x, start.y + height}, LINE_COLOR);
  draw_line(img, start, {start.x - w, start.y - h}, LINE_COLOR);
  draw_line(img, start, {start.x + w, start.y - h}, LINE_COLOR);
  draw_line(img, {start.x - w, start.y - h}, {start.x - w, start.y - h + height}, LINE_COLOR);
  draw_line(img, {start.x + w, start.y - h}, {start.x + w, start.y - h + height}, LINE_COLOR);
  draw_line(img, {start.x - w, start.y - h + height}, {start.x, start.y + height}, LINE_COLOR);
  draw_line(img, {start.x + w, start.y - h + height}, {start.x, start.y + height}, LINE_COLOR);
}

bool same_color(const Pixel &a, const Pixel &b)
{
  return a.r == b.r && a.g == b.g && a.b == b.b;
}

void copy_asset(Image *img, Position center, Direction d)
{
  Image *asset = robots + d;
  for (int y = 0; y < asset->height; y++)
  {
    for (int x = 0; x < asset->width; x++)
    {
      Pixel src = asset->data[y * asset->width + x];
      if (same_color(src, BG_COLOR))
        continue;
      img->data[(center.y - y) * img->width + center.x - asset->width / 2 + x] = src;
    }
  }
}

void light_status(Map *map, Position p, bool *is_light, bool *is_lightened)
{
  *is_light = false, *is_lightened = false;
  for (int i = 0; i < map->q; i++)
    if (same_position(p, map->lights[i]))
    {
      *is_light = true;
      if (map->lightened[i])
        *is_lightened = true;
    }
}

Image *draw()
{
  // each cell is 200p*116p
  // cell height is 25p
  Map *map = game.map_run;
  const int w = 200;
  const int h = 116;
  int H = h * MAX_M + 400;
  int W = w * MAX_N + 25 * MAX_H + 400;
  Image *img = new Image{H, W};
  img->data = new Pixel[H * W];
  for (int i = 0; i < H; i++)
    for (int j = 0; j < W; j++)
      img->data[i * W + j] = {20, 128, 20};
  int bias_x = W / 2;
  int bias_y = (MAX_M - h * map->m) / 2 + 600 + MAX_H * 25;
  for (int r = 0; r < map->m; r++)
  {
    for (int c = 0; c < map->n; c++)
    {
      for (int _h = 0; _h < map->height[r][c]; _h++)
      {
        // draw each cell without height
        // calculate position is boring and meaningless, just try to adjust these parameters
        Position center = {(c - r) * w / 2 + bias_x, (r + c) * h / 2 + bias_y - (CELL_HEIGHT - 1) * _h};
        bool is_light, is_lightened;
        light_status(map, {c, r}, &is_light, &is_lightened);
        Pixel color = CELL_COLOR;
        if (is_light)
          color = LIGHT_COLOR;
        if (is_lightened)
          color = LIGHTEN_COLOR;
        draw_cell(img, center, h / 2, w / 2, color);
        if (_h)
          draw_parallelogram(img, {center.x, center.y + h / 2}, h / 2, w / 2, CELL_HEIGHT);
        else
        {
          draw_line(img, {center.x - w / 2, center.y}, {center.x - w / 2, center.y + CELL_HEIGHT / 2}, LINE_COLOR);
          draw_line(img, {center.x + w / 2, center.y}, {center.x + w / 2, center.y + CELL_HEIGHT / 2}, LINE_COLOR);
          draw_line(img, {center.x, center.y + h / 2}, {center.x, center.y + h / 2 + CELL_HEIGHT / 2}, LINE_COLOR);
        }
      }
    }
  }
  // Then draw the robot
  int r = map->robot.y, c = map->robot.x;
  Position center = {(c - r) * w / 2 + bias_x, (r + c) * h / 2 + bias_y - CELL_HEIGHT * map->height[r][c] + 30};
  copy_asset(img, center, map->dir);
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