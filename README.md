# Full System Simulator

A comprehensive computer system simulator for educational purposes, featuring a full-stack implementation from hardware emulation to application execution.

## Features

- Complete CPU emulation (NEMU)
- Hardware abstraction layer (Abstract-Machine)
- Simple operating system (Nanos-lite)
- Application framework (Navy-apps)
- Integrated debugging and tracing capabilities

## Prerequisites

Before setting up this project, you need to install the following dependencies:

```bash
# Essential development tools
sudo apt-get install build-essential    # Build tools including gcc, make, etc
sudo apt-get install man                # Reference manual
sudo apt-get install gcc-doc            # GCC documentation
sudo apt-get install gdb                # GNU debugger
sudo apt-get install git                # Version control system

# Required libraries
sudo apt-get install libreadline-dev    # Command line editing library
sudo apt-get install libsdl2-dev        # Simple DirectMedia Layer
```

## Installation

1. Clone the repository:
   ```bash
   git clone git@github.com:ETOwang/Full-System-Simulator.git
   cd Full-System-Simulator
   ```

2. Initialize the required subprojects:
   ```bash
   bash init.sh                # Initialize the project
   ```

3. Source your environment variables:
   ```bash
   source ~/.bashrc
   ```

## Quick Start Guide

### NEMU (NJU EMUlator)

NEMU is the hardware emulation component that simulates CPU execution.

```bash
cd nemu
make menuconfig    # Configure build options
make               # Build NEMU
make run           # Run NEMU with default settings
make gdb           # Debug NEMU with GDB
```

### Abstract Machine

The abstract machine provides a hardware abstraction layer for applications.

```bash
cd abstract-machine
make                                     # Build the abstract machine library
cd am-kernels/tests/cpu-tests            # Navigate to test directory
make ARCH=native mainargs=hello-str run  # Run a simple test
```

### Nanos-lite

Nanos-lite is a simplified operating system running on the abstract machine.

```bash
cd nanos-lite
make              # Build nanos-lite
make run          # Run nanos-lite
make gdb          # Debug nanos-lite
```

### Navy-apps

Navy-apps contains applications that run on the Nanos-lite OS.

```bash
cd navy-apps
make              # Build all Navy applications
make run          # Run Navy applications within NEMU

# Build and run specific application
cd apps/hello
make
make ISA=native run
```

## Project Structure

- **NEMU**: CPU emulator supporting multiple ISAs
- **Abstract-Machine**: Hardware abstraction layer
- **Nanos-lite**: Simple operating system
- **Navy-apps**: Application framework and example applications

## Troubleshooting

If you encounter issues with environment variables, run:
```bash
source ~/.bashrc
```

For compilation errors, ensure all dependencies are properly installed.

## Acknowledgements

This project originated from the programming assignment of the Introduction to Computer Systems (ICS) course in the Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment, refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2024/

The following subprojects/components are included:
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)
