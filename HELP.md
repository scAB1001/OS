## Setup and understanding

### xv6 root dir
- This dir contains the user space programs provided by the xv6 operating system.
- The implementation of a simple shell, and commands like ls, ln and echo.

#### programs
- kernal: implementation of the xv6 kernel
- mkfs: utility to construct a file system for xv6
- implementation of the xv6 user space programs

#### compiling the xv6 OS
** If you haven't downloaded the singularity image/docker **
 ```singularity pull docker://callaghanmt/xv6-tools:buildx-latest```  

** Otherwise **
 ```singularity shell xv6-tools_buildx-latest.sif```
 ```make```		builds the xv6 OS
 ```make qemu```	to emulate the RISC-V CPU for xv6 to run on

	You are now running in xv6 under qemu on an emulated RISC-V CPU.
If you ```> ls``` , you will see a bunch of build targets

- To quit: ```ctrl-a + x```
- xv6 shell is similar to bash
- You can write programs for xv6 OS in C/C++ (anythin GCC can compile)

#### make commands
make command	|	Purpose   
make			Compiles all dirs of build artefacts   
make clean		Cleans ^   
make qemu		COmpiles xv6 kernel, user space programs & starts qemu: emulates on xv6 OS   make qemu-gdb		  ^ but also sets up remote debugging, pausing xv6 OS as it boots.    

### vim shortcuts
```
# Turn on line numbers
set nu
# Delete words before cursor
ctrl + w
# Delete characters before the cursor
ctrl + h
# Create a new line when inserting a new mode
ctrl + j
# Undo
ctrl + u
```

### Adding a .c file to compile in Makefile
- cd users; vim name.c
- Under UPROGS: TAB $U/_name.c\ ("\" unless the last)

Recompile:    
- make clean; make   
- singularity shell xv6-tools_buildx-latest.sif; make qemu   

## Running gdb debugger
- exit xv6 OS
- make qemu-gdb
- new terminal, load toolchain
- gdb-multiarch -tui; source .gdbinit (tells gdb the arch. of the plat. being debugged 
