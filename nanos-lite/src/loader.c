#include <proc.h>
#include <elf.h>
#include <fs.h>
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
static uintptr_t loader(PCB *pcb, const char *filename) {
    int fd=fs_open(filename,0,0);
    Elf_Ehdr ehdr;
    fs_read(fd,&ehdr,sizeof(ehdr));
    assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
    for (int i = 0; i < ehdr.e_phnum; i++) {
      
      Elf_Phdr phdr;
      fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
      fs_read(fd, &phdr, sizeof(phdr));
      assert(ehdr.e_phentsize == sizeof(phdr));
      if (phdr.p_type == PT_LOAD) {
        fs_lseek(fd, phdr.p_offset, SEEK_SET);
        fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
        if (phdr.p_filesz < phdr.p_memsz) {
          memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
        }
      }
    }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

