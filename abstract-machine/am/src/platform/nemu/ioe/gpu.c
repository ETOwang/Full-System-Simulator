#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define HEIGHT_MASK 0x0000ffff
#define WIDTH_MASK 0xffff0000
void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t size=inl(VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = (size&WIDTH_MASK)>>16, .height = size&HEIGHT_MASK,
    .vmemsz = 0
  };
  cfg->vmemsz=cfg->height*cfg->width*sizeof(uint32_t);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
   uint32_t size=inl(VGACTL_ADDR);
   uint32_t width=(size&WIDTH_MASK)>>16;
   uint32_t *fb = (uint32_t *)FB_ADDR;
   for (uint32_t i = ctl->y; i < ctl->y+ctl->h; i++)
   {
      for (uint32_t j = ctl->x; j < ctl->x+ctl->w; j++)
      {  
         fb[i*width+j]=((uint32_t*)ctl->pixels)[(i-ctl->y)*ctl->w+j-ctl->x];
      }
   }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
