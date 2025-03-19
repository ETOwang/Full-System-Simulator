# Navy Application Framework

## Compilation Method

In the application directory (such as `tests/hello/`), use
```bash
make ISA=xxx
```
to compile the program, and use
```bash
make ISA=xxx install
```
to compile the program and install it to the filesystem image under the `fsimg/bin/` directory.

To compile a library individually (such as `libs/libbmp`), you need to run
```bash
make ISA=xxx archive
```

Use
```bash
make clean
```
to clean the compilation results of an application or library. You can also run
```bash
make clean-all
```
in the project root directory to clean all compilation results for all applications and libraries.

## File System Image

The `fsimg/` directory contains the contents of the filesystem image:
```
bin/ -- binary files (this directory is not created by default)
share/  -- platform-independent files
  files/ -- used for file testing
  fonts/ -- font files
  music/ -- example music
  pictures/ -- example images
  games/ -- game data (this directory is not created by default)
    nes/ -- NES Roms
    pal/ -- Chinese Paladin (Xianjian Qixia Zhuan) related data files
```

The Makefile in the project root directory contains rules for building the filesystem image.
You can modify the `APPS` and `TESTS` variables to specify which applications to install to the filesystem image,
then use
```bash
make ISA=xxx fsimg
```
to compile and install the specified applications.

Additionally, you can use
```bash
make ISA=xxx ramdisk
```
to sequentially package the contents of `fsimg/` into a simple image file `build/ramdisk.img`,
and generate the corresponding file information `build/ramdisk.h`.
Operating systems that support simple file systems (like Nanos-lite) can use this image.

## Supported ISA/Operating Systems

### native (compiled to run locally on Linux)

When compiling locally, libc and libos are not linked; instead, the local glibc/glibstdc++ is used (linked with g++ without parameters).

`LD_PRELOAD` is used to implement locally simulated `/dev/events`, `/dev/fb`, and some other device files. For details, see `libs/libos/src/native.cpp`.
The code simulates a Navy-compatible runtime environment. Use the following command to run native programs:
```bash
make ISA=native run mainargs="arg1 arg2 ..."
```

### x86, mips32, riscv32, am_native (Nanos, Nanos-lite)

Where `am_native` refers to running the operating system on the `native` platform of the 