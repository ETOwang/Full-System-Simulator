#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)};

size_t serial_write(const void *buf, size_t offset, size_t len)
{
  int count = 0;
  for (int i = 0; i < len; i++)
  {
    if (*(char *)(buf + i) == '\0')
    {
      break;
    }
    putch(*(char *)(buf + i));
    count++;
  }
  return count;
}
extern void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd);
size_t events_read(void *buf, size_t offset, size_t len)
{
  AM_INPUT_KEYBRD_T kbd;
  __am_input_keybrd(&kbd);
  if (kbd.keycode == AM_KEY_NONE)
  {
    return 0;
  }
  if (kbd.keydown)
  {
    return snprintf(buf, len, "kd %s\n", keyname[kbd.keycode]);
  }
  return snprintf(buf, len, "ku %s\n", keyname[kbd.keycode]);
}

extern void __am_gpu_config(AM_GPU_CONFIG_T *cfg);
size_t dispinfo_read(void *buf, size_t offset, size_t len)
{
  AM_GPU_CONFIG_T cfg;
  __am_gpu_config(&cfg);
  char temp[128];
  snprintf(temp, 128, "WIDTH : %d\nHEIGHT:%d\n", cfg.width, cfg.height);
  return snprintf(buf, len, "%s", temp);
}
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl);
size_t fb_write(const void *buf, size_t offset, size_t len)
{
  //TODO: FASTER
  AM_GPU_FBDRAW_T ctl;
  AM_GPU_CONFIG_T cfg;
  __am_gpu_config(&cfg);
  int start_x = offset/4 % cfg.width;
  int start_y = offset/4 / cfg.width;
  if(len/4 + start_x <=cfg.width)
  {
    ctl.pixels = (uint32_t *)buf;
    ctl.x = start_x;
    ctl.y = start_y;
    ctl.w = len/4;
    ctl.h = 1;
    ctl.sync = 1;
    __am_gpu_fbdraw(&ctl);
  }else{
    for (size_t i = 0; i < len / 4; i++)
    {
      ctl.pixels = (uint32_t *)(buf+4*i);
      ctl.x = (offset+4*i)/ 4 % cfg.width;
      ctl.y = (offset+4*i)/ 4 / cfg.width;
      ctl.w = 1;
      ctl.h = 1;
      ctl.sync = 1;
      __am_gpu_fbdraw(&ctl);
    }
  }
  
  return len;
}

void init_device()
{
  Log("Initializing devices...");
  ioe_init();
}
