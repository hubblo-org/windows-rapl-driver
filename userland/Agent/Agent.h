#ifndef _AGENT_H
#define _AGENT_H

#pragma once

#include <cstdint>
#include <windows.h>

#define DRIVER_DEVICE_BLOCK_NAME L"\\\\.\\RAPLDriver"
#define AGENT_POWER_UNIT_CODE 0xBEB
#define AGENT_POWER_LIMIT_CODE 0xBEC
#define AGENT_ENERGY_STATUS_CODE 0xBED

/* MSR registers are 64 bit long */
typedef uint64_t MSR_REGISTER_T;
static uint8_t msrResult[sizeof(MSR_REGISTER_T)];
static uint8_t msrRegisterBuffer[sizeof(MSR_REGISTER_T)];

static HANDLE hDevice;

void OpenDevice(void);
BOOL SendRequest(const uint16_t requestCode, const uint8_t *request, const size_t requestLength, uint8_t* reply, const size_t replyLength);
const uint8_t* msrRegisterToBuffer(MSR_REGISTER_T msrRegister);
extern "C" void __cpuid__(char *manufacturer);

#endif /* _AGENT_H */
