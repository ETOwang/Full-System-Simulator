#include <fs.h>

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct
{
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum
{
  FD_STDIN,
  FD_STDOUT,
  FD_STDERR,
  FD_FB
};

size_t invalid_read(void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}
size_t invalid_write(const void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len) ;
extern size_t fb_write(const void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
    {"/dev/events", 0, 0, events_read, invalid_write},
    {"/dev/fb",0,0,invalid_read,fb_write},
    {"/proc/dispinfo",0,0,dispinfo_read,invalid_write}
   };


extern void __am_gpu_config(AM_GPU_CONFIG_T *cfg);
void init_fs()
{
  // TODO: initialize the size of /dev/fb
  for (int i = FD_STDERR + 1; i < sizeof(file_table) / sizeof(Finfo); i++)
  {
    if (strcmp(file_table[i].name, "/dev/fb") == 0)
    {
      AM_GPU_CONFIG_T cfg;
      __am_gpu_config(&cfg);
      file_table[i].size = cfg.width* cfg.height * 4;
    }
  }
}

int fs_open(const char *pathname, int flags, int mode)
{
  for (int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++)
  {
    if (strcmp(pathname, file_table[i].name) == 0)
    {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert(0);
}
size_t fs_read(int fd, void *buf, size_t len)
{
  if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR)
  {
    return 0;
  }
  int ret;
  if (file_table[fd].read == NULL)
  {
    if(file_table[fd].open_offset + len > file_table[fd].size)
    {
      len = file_table[fd].size - file_table[fd].open_offset;
    }
    ret = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  else
  {
    ret = file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  file_table[fd].open_offset += ret;
  return ret;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
  if (fd == FD_STDIN)
  {
    return 0;
  }
  if (fd != FD_STDOUT && fd != FD_STDERR)
  {
    if (file_table[fd].open_offset + len > file_table[fd].size)
    {
      len = file_table[fd].size - file_table[fd].open_offset;
    }
  }
  int ret;
  if (file_table[fd].write == NULL)
  {
    ret = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  else
  {
    ret = file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  file_table[fd].open_offset += ret;
  return ret;
}
size_t fs_lseek(int fd, size_t offset, int whence)
{
  if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR)
  {
    return 0;
  }
  switch (whence)
  {
  case SEEK_SET:
    file_table[fd].open_offset = offset;
    break;
  case SEEK_CUR:
    file_table[fd].open_offset += offset;
    break;
  case SEEK_END:
    file_table[fd].open_offset = file_table[fd].size + offset;
    break;
  default:
    assert(0);
  }
  assert(file_table[fd].open_offset <= file_table[fd].size);
  return file_table[fd].open_offset;
}
int fs_close(int fd)
{
  return 0;
}