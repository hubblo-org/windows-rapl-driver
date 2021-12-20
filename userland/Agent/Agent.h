#ifndef _AGENT_H
#define _AGENT_H

#pragma once

#include <cstdint>
#include <windows.h>

#define DRIVER_DEVICE_BLOCK_NAME L"\\\\.\\RAPLDriver"
#define AGENT_POWER_UNIT_CODE 0xBEB
#define AGENT_POWER_LIMIT_CODE 0xBEC
#define AGENT_ENERGY_STATUS_CODE 0xBED

/* Run Time Average Power Limiting (RAPL) Interface */

#define MSR_RAPL_POWER_UNIT		0x00000606

#define MSR_PKG_POWER_LIMIT		0x00000610
#define MSR_PKG_ENERGY_STATUS		0x00000611
#define MSR_PKG_PERF_STATUS		0x00000613
#define MSR_PKG_POWER_INFO		0x00000614

#define MSR_DRAM_POWER_LIMIT		0x00000618
#define MSR_DRAM_ENERGY_STATUS		0x00000619
#define MSR_DRAM_PERF_STATUS		0x0000061b
#define MSR_DRAM_POWER_INFO		0x0000061c

#define MSR_PP0_POWER_LIMIT		0x00000638
#define MSR_PP0_ENERGY_STATUS		0x00000639
#define MSR_PP0_POLICY			0x0000063a
#define MSR_PP0_PERF_STATUS		0x0000063b

#define MSR_PP1_POWER_LIMIT		0x00000640
#define MSR_PP1_ENERGY_STATUS		0x00000641
#define MSR_PP1_POLICY			0x00000642

#define MSR_AMD_RAPL_POWER_UNIT		0xc0010299
#define MSR_AMD_CORE_ENERGY_STATUS		0xc001029a
#define MSR_AMD_PKG_ENERGY_STATUS	0xc001029b

/* MSR registers are 64 bit long */
typedef uint64_t MSR_REGISTER_T;
static uint8_t msrResult[sizeof(MSR_REGISTER_T)];
static uint8_t msrRegisterBuffer[sizeof(MSR_REGISTER_T)];

static HANDLE hDevice;
static char manufacturer[13];
static int cpu_regs[4];

void OpenDevice(void);
BOOL SendRequest(const uint16_t requestCode, const uint8_t *request, const size_t requestLength, uint8_t* reply, const size_t replyLength);
const uint8_t* msrRegisterToBuffer(MSR_REGISTER_T msrRegister);

/*
#ifdef WIN64
extern "C" void __cpuid__(char *manufacturer);
#else
void __cpuid__(void);
#endif
*/

#endif /* _AGENT_H */
