#include <iostream>
#include <intrin.h>
#include "Agent.h"

using namespace std;

int main()
{
    uint8_t i, j, count = 128;

    /* Get CPU information */
    memset(manufacturer, 0, sizeof(manufacturer));
    __cpuid(cpu_regs, 0);
    memcpy(manufacturer, &cpu_regs[1], sizeof(uint32_t));
    memcpy(manufacturer + sizeof(uint32_t), &cpu_regs[3], sizeof(uint32_t));
    memcpy(manufacturer + 2 * sizeof(uint32_t), &cpu_regs[2], sizeof(uint32_t));
    printf("CPU manufacturer: %s\n", manufacturer);

    OpenDevice();

    for (i = i ^ i; i < count; ++i)
    {
        /* These 3 calls are almost the final example to what we really want */
        if (strncmp(manufacturer, "GenuineIntel", 12) == 0)
        {
            SendRequest(AGENT_POWER_UNIT_CODE, msrRegisterToBuffer(MSR_RAPL_POWER_UNIT), sizeof(MSR_REGISTER_T), msrResult, sizeof(MSR_REGISTER_T));
            printMsrBuffer(msrResult);
            SendRequest(AGENT_POWER_LIMIT_CODE, msrRegisterToBuffer(MSR_PKG_POWER_LIMIT), sizeof(MSR_REGISTER_T), msrResult, sizeof(MSR_REGISTER_T));
            printMsrBuffer(msrResult);
            SendRequest(AGENT_ENERGY_STATUS_CODE, msrRegisterToBuffer(MSR_PKG_ENERGY_STATUS), sizeof(MSR_REGISTER_T), msrResult, sizeof(MSR_REGISTER_T));
            printMsrBuffer(msrResult);
        }
        else
        {
            /* Assume it's AMD processor */
            SendRequest(AGENT_POWER_UNIT_CODE, msrRegisterToBuffer(MSR_AMD_RAPL_POWER_UNIT), sizeof(MSR_REGISTER_T), msrResult, sizeof(MSR_REGISTER_T));
            SendRequest(AGENT_POWER_LIMIT_CODE, msrRegisterToBuffer(MSR_AMD_CORE_ENERGY_STATUS), sizeof(MSR_REGISTER_T), msrResult, sizeof(MSR_REGISTER_T)); // FIXME: is that really equal to MSR_PKG_POWER_LIMIT??
            SendRequest(AGENT_ENERGY_STATUS_CODE, msrRegisterToBuffer(MSR_AMD_PKG_ENERGY_STATUS), sizeof(MSR_REGISTER_T), msrResult, sizeof(MSR_REGISTER_T));
        }
    }

    CloseHandle(hDevice);

    return 0;
}

void OpenDevice(void)
{
    hDevice = CreateFileW(DRIVER_DEVICE_BLOCK_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        puts("Failed to open device");
        ExitProcess(-1);
    }
    puts("Device opened!");
}

BOOL SendRequest(const uint16_t requestCode, const uint8_t *request, const size_t requestLength, uint8_t* reply, const size_t replyLength)
{
    size_t i;
    DWORD len;

    /* Zero memory result buffer */
    memset(reply, 0, replyLength);
    if (DeviceIoControl(hDevice, CTL_CODE(FILE_DEVICE_UNKNOWN, requestCode, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA),
        (LPVOID)request, requestLength,
        msrResult, replyLength, /* Set MSR buffer to store answer from driver */
        &len, NULL))
    {
        //puts("Device answered!");
        if (len != replyLength)
        {
            printf("Uh oh, got invalid length answer. Expected %i, got %i\n", replyLength, len);
            return FALSE;
        }

        /*
        puts("Got answer:");
        for (i = 0; i < replyLength; ++i)
        {
            if ((i % 16) == 0)
                puts("");
            printf("%02x ", reply[i]);
        }
        puts("");
        */
    }
    else
    {
        printf("Failed to talk with device. Error code: %u\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

const uint8_t* msrRegisterToBuffer(MSR_REGISTER_T msrRegister)
{
    memcpy(msrRegisterBuffer, &msrRegister, sizeof(MSR_REGISTER_T));
    return msrRegisterBuffer;
}

void printMsrBuffer(const uint8_t *buffer)
{
    MSR_REGISTER_T value;

    memcpy(&value, buffer, sizeof(MSR_REGISTER_T));
    printf("Read: 0x%llx\n", value);
}