
struct Pixel
{
  // notice: bmp use color order (b,g,r)
  unsigned char b, g, r;
};

struct Image
{
  int height, width;
  Pixel *data;
};

void init_assets();
void release_assets();
void save(const char *path);
void auto_save();
