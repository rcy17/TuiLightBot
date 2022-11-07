#define uint16 unsigned short
#define uint32 unsigned int
#define int32 int

#pragma pack(1)

struct BMPHeader
{                          // Total: 54 bytes
  uint16 type;             // Magic identifier: 0x4d42
  uint32 size;             // File size in bytes
  uint16 reserved1;        // Not used
  uint16 reserved2;        // Not used
  uint32 offset;           // Offset to image data in bytes from beginning of file (54 bytes)
  uint32 dib_header_size;  // DIB Header size in bytes (40 bytes)
  uint32 width_px;         // Width of the image
  uint32 height_px;        // Height of image
  uint16 num_planes;       // Number of color planes
  uint16 bits_per_pixel;   // Bits per pixel
  uint32 compression;      // Compression type
  uint32 image_size_bytes; // Image size in bytes
  int32 x_resolution_ppm;  // Pixels per meter
  int32 y_resolution_ppm;  // Pixels per meter
  uint32 num_colors;       // Number of colors
  uint32 important_colors; // Important colors
};

struct BMPFile
{
  BMPHeader header;
  unsigned char *data;
};

BMPFile *read_bmp(const char *path);
bool write_bmp(const char *path, BMPFile *file);
void release_bmp(BMPFile *bmp);
