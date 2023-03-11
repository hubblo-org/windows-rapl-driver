<h1 align="center">
  windows-rapl-driver
</h1>

<p align="center">
    <img src="https://repository-images.githubusercontent.com/421079628/f695abc0-c8e6-46a3-a6f4-6c7c0f617b87" width="800">
</p>

# Windows driver for RAPL metrics gathering

## Installation (for testing/troubleshooting purposes, in test mode)

### Enable test mode

For now, the driver is unsigned, so you need to put windows in test mode (allowing custom drivers to run).

To do this, open a command prompt **as an administrator** and run :

	bcdedit.exe -set TESTSIGNING ON

Then restart the computer or server.

Once restarted, if on a desktop, you should have some text written on the bottom right corner of the desktop view with **"Test Mode"** displayed.

To disable test mode just open another command prompt with adminstrator access and run :

	bcdedit.exe -set TESTSIGNING OFF

### Installing the driver

First download or [compile](#Compilation) :
- DriverLoader.exe
- ScaphandreDrv.sys

You need those files in the same folder.

Then run, in an adminstrator command prompt :

	DriverLoader.exe install
	
Start the service :

	DriverLoader.exe start

At any time you could check for the state of the service giving access to the driver on your system, with this command :

	driverquery /v | grep -i scaph

If running properly it should show a line like :

	Scaphandre Dr Scaphandre Driver Serv Scaphandre Driver Serv File System    System            Running    OK         TRUE             FALSE                  0                 4□096       0          14/01/2022 16:01:37    C:\WINDOWS\system32\DRIVERS\ScaphandreDrv.sys    4□096


## Compilation

### Windows 10

Install Visual Studio 2019

### Windows 11

Install Visual Studio 2022

According to [this documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk):

- [Install the Windows 11 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
- [Install the Windows 11 WDK (Windows Driver Kit)](https://go.microsoft.com/fwlink/?linkid=2196230)

Optionnal : [install the EWDK](https://learn.microsoft.com/en-us/legal/windows/hardware/enterprise-wdk-license-2022)

Right clik on ScaphandreDrv in the right panel, in General Properties, look for the "Platform Toolset" field. It should say "WindowsKernelModeDriver10.0".

If the WDK doesn't appear in ScaphandreDrv Properties as a Platform Toolset choice, look for the vsix runner that should have a path like :

```
 C:\Program Files (x86)\Windows Kits\10\Vsix\VS2022\10.0.22621.382\WDK.vsix.
```

Close Visual Studio 2022, run the vsix runner.

On the top of the VS Window, select "Release" as a target and "x64" as a target platform.

Then "Build" > "Build Solution".

To build DriverLoader :

Right Click on the Solution > Add > New Projet > Empty Project

On the new Project "DriverLoader" inside the solution, right click > add > existing item, look for DriverLoader.cpp.

Right click on DriverLoader in the right panel > Properties > Advanced, then ensure "Character Set" is set as "Use Multi-Byte Character Set".

Build > Build Solution ( same target and platform as ScaphandreDrv ).

### Compile from GNU/Linux (Ubuntu 22.04), run on windows (not recommended, as this is static compilation)

Compile DriverLoader.exe in `userland/Service`:

```
cd userland/Service

sudo apt install g++-mingw-w64 g++-mingw-w64-x86-64-win32 g++-mingw-w64-x86-64

x86_64-w64-mingw32-g++ DriverLoader.cpp -I/usr/x86_64-w64-mingw32/include/ddk/ -I/usr/x86_64-w64-mingw32/include/ --sysroot=. -o DriverLoader.exe -Wall -pedantic
```

### Build an MSI package

#### Setup

In Visual Studio 2022 or 2019, open Extensions > Manage Extensions, then search for "Microsoft Visual Studio Installer Projects 2019|2022", then Download, close Visual Studtio.

Then a VSIX installer should start. Restart Visual Studio.

If the ScaphandreDrvInstaller project is displayed as incompatible, right click, then "Reload Project". Then Build.

### Test-sign the driver (reference documentation is [here](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/how-to-test-sign-a-driver-package))

In C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64

	.\MakeCert.exe -r -pe -ss PrivateCertStore -n CN=hubblo.org -eku 1.3.6.1.5.5.7.3.3 ScaphandreDrvTest.cer
	
In C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86, when you already have an Inf file

	.\stampinf.exe -f C:\Users\bpeti\source\repos\windows-rapl-driver\ScaphandreDrv\ScaphandreDrv.inf -d 03/11/2023 -v 0.0.1
	
Then (need to change inf file to succeed with x64, TODO document this part)

	.\Inf2Cat.exe /driver:C:\Users\bpeti\source\repos\windows-rapl-driver\ScaphandreDrv\ /os:10_X64

Then

	.\signtool.exe sign /v /fd sha256 /s PrivateCertStore /n hubblo.org "C:\Users\bpeti\source\repos\windows-rapl-driver\ScaphandreDrv\scaphandredrv.cat"
	
Then (as Administrator)

	.\certmgr.exe /add "C:\Users\bpeti\source\repos\windows-rapl-driver\ScaphandreDrv\ScaphandreDrvTest.cer" /s /r localMachine root

## Context

This driver has been developped for a specific use case : enabling [Scaphandre](https://github.com/hubblo-org/scaphandre) on Windows.

Please have a look at [those slides](https://github.com/hubblo-org/scaphandre/files/8601923/Boavizta.-.Scaphandre_RAPL_Windows.pdf) for a better understanding of how and why this driver has been developped.

## Cross compilation MinGW (OUTDATED)

Dependencies:
- x86_64-w64-mingw32-g++

Compilation:
x86_64-w64-mingw32-g++ DriverLoader.cpp -o DriverLoader.exe

## Compilation CPUID (OUTDATED)

Code getting CPU manufacturer's informations (through CPUID) uses inlined assembly.

To compile on Linux, we need to tell GCC to use the Intel syntax and not the At&T one. We also need to specify we don't want an axecutable with independant position.

Example : gcc -masm=intel -no-pie cpuid.c -o cpuid

To compile on Windows, we can inline 32 bit assembly but not 64 bit (MSVC compiler doesn't support it).

To fix this, the __cpuid__ function is written in pure assembly matching the Windows ABI (x86 and x86_64). To compile, use those commands :
 - x86 : nasm -f win32 cpuid_x86.nasm -o cpuid_x86.obj
 - x86_64 : nasm -f win64 cpuid_x86_64.nasm -o cpuid_x86_64.obj

To compile the agent, including the __cpuid__ function, do it according to the architecture :
 - x86_64 : x86_64-w64-mingw32-g++ RAPLAgent.cpp -s cpuid_x86_64.obj -o RAPLAgent.exe

## Q & A

## External documentation we used

- https://github.com/MicrosoftDocs/windows-driver-docs/blob/staging/windows-driver-docs-pr/install/signature-categories-and-driver-installation.md
- https://github.com/intel/powergov
- https://community.intel.com/t5/Software-Tuning-Performance/RAPL-for-energy-measurement/td-p/919723
- www.eecs.berkeley.edu/Pubs/TechRpts/2012/EECS-2012-168.pdf
- https://github.com/kentcz/rapl-tools
- https://01.org/blogs/2014/running-average-power-limit-%E2%80%93-rapl
- https://github.com/energymon/energymon/blob/master/msr/energymon-msr.c
- https://github.com/gz/rust-x86
- https://docs.rs/x86/0.43.0/x86/
- https://github.com/gz/rust-x86/tree/master/x86test
- https://docs.rs/x86/0.43.0/x86/msr/constant.MSR_RAPL_POWER_UNIT.html
- https://github.com/insula-rs/libwhp/tree/master/src
- https://crates.io/crates/register/1.0.2
- https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/rdmsr--read-msr-
- https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.html
- https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
- https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-software-developers-manual-volume-1-basic-architecture.html
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Register/Intel/ArchitecturalMsr.h
- https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Register/Intel/Msr
- https://github.com/lizhuohua/linux-kernel-module-rust
- https://docs.rs/x86test-types/0.0.3/x86test_types/macro.kassert.html
- https://doc.rust-lang.org/unstable-book/library-features/llvm-asm.html
- https://elixir.bootlin.com/linux/latest/source/drivers/powercap
- http://icl.cs.utk.edu/papi/docs/d4/d46/linux-powercap_8c_source.html
- https://www.osr.com/getting-started-writing-windows-drivers/
- http://woshub.com/how-to-sign-an-unsigned-driver-for-windows-7-x64/
- https://www.electronicdesign.com/technologies/windows/article/21802026/how-to-write-windows-drivers
- https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
- https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
- https://docs.microsoft.com/en-us/windows/win32/sync/synchronization-and-overlapped-input-and-output
- https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
- https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver
- https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-your-first-driver
- https://www.intel.com/content/www/us/en/developer/articles/tool/power-gadget.html#inpage-nav-12
- https://github.com/Microsoft/Windows-driver-samples
- https://www.osronline.com/article.cfm%5Earticle=229.htm
- https://fr.adasbcc.org/driver-blocked-from-loading-fix-3085
- https://www.codeproject.com/articles/13090/building-and-deploying-a-basic-wdf-kernel-mode-dri
- https://www.digicert.com/kb/code-signing/signcode-signtool-command-line.htm
- https://www.techpout.com/update-drivers-using-command-prompt-in-windows-10/

Fichiers intéressants dans le noyau linux :
- drivers/powercap/intel_rapl_msr.c
- arch/x86:lib/msr_smp.c

## Q & A

### The code execution cannot proceed because MSVCP140D.dll was not found. Reinstalling the program may fix the problem.

build in release mode then retry !

### How to build ?

"F:\Windows Kits\10\bin\x86\Inf2Cat.exe" /driver:c:\Users\nulse\Documents\GitHub\test\ /os:10_19H1_X64,10_RS5_X64,ServerRS5_X64,10_RS4_X64

see https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/inf2cat
