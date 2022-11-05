
struct Pixel
{
  unsigned char r, g, b;
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
