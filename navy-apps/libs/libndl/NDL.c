#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static uint32_t start_time=0;
uint32_t NDL_GetTicks()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000-start_time;
}

int NDL_PollEvent(char *buf, int len)
{
  int fd = open("/dev/events", 1);
  int nread = read(fd, buf, len);
  //in libc,close fd may cause problem 
  close(fd);
  if (nread == 0)
  {
    return 0;
  }
  return 1;
}

void NDL_OpenCanvas(int *w, int *h)
{
  if (getenv("NWM_APP"))
  {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1)
    {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  }
  else
  {
    if (*w == 0 && *h == 0)
    {
      char buf[64];
      int fd = open("/proc/dispinfo", 1);
      read(fd, buf, 64);
      sscanf(buf, "WIDTH : %d\nHEIGHT:%d\n", w, h);
      close(fd);
    }
    screen_w = *w;
    screen_h = *h;
  }
}
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h)
{
  //TODO:may cause problems
  if (getenv("NWM_APP"))
  {
    int len = 4 * screen_w;
    for (int i = 0; i < h; i++)
    {
      write(fbdev, pixels + (y + i) * screen_w + x, len);
    }
  }
  else
  {
    int width, height;
    char buf[64];
    int fd = open("/proc/dispinfo", 1);
    read(fd, buf, 64);
    sscanf(buf, "WIDTH : %d\nHEIGHT:%d\n", &width, &height);
    fd=open("/dev/fb",1);
    int bias_w = (width - screen_w) / 2;
    int bias_h = (height - screen_h) / 2;
    for (size_t i = y; i < y + h; i++)
    {
      lseek(fd, 4 * (bias_h * width + bias_w + x + i * width), SEEK_SET);
      write(fd,pixels + (i - y) * w, 4 * w);
    }
  }
}

void NDL_OpenAudio(int freq, int channels, int samples)
{
}

void NDL_CloseAudio()
{
}

int NDL_PlayAudio(void *buf, int len)
{
  return 0;
}

int NDL_QueryAudio()
{
  return 0;
}

int NDL_Init(uint32_t flags)
{
  if (getenv("NWM_APP"))
  {
    evtdev = 3;
  }
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start_time=tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return 0;
}

void NDL_Quit()
{
}
