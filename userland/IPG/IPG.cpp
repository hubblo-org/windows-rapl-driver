// IPG.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cstdio>
#include <cstdint>
#include <Windows.h>
#include "types.h"

auto CMD_INIT = make_initialization_request();
auto CMD_CPU_UTILIZATION = make_cpu_utilization_request();

void buf_debug(uint8_t* buf, size_t len)
{
    size_t i;

    printf("\tBuffer length = %i\n", len);
    for (i = 0; i < len; ++i)
    {
        if (i % 16 == 0)
            puts("");
        printf("%02x ", buf[i]);
    }
    puts("");
}

int main()
{
    HANDLE hDevice;
    uint8_t *buf;
    DWORD len;
    uint64_t seq;
    size_t buf_len;

    hDevice = CreateFile(L"\\\\.\\EnergyDriver", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        puts("Failed to open device");
        return 1;
    }
    puts("Device opened!");

    buf = cmd_to_buf(CMD_INIT, &buf_len);
    buf_debug(buf, buf_len);

    if (DeviceIoControl(hDevice, 0x22E00A, buf, buf_len, CMD_INIT->response->data, CMD_INIT->response->len, &len, NULL))
    {
        puts("Device returned: ");
        buf_debug(CMD_INIT->response->data, CMD_INIT->response->len);

        buf = cmd_to_buf(CMD_CPU_UTILIZATION, &buf_len);
        buf_debug(buf, buf_len);
        if (DeviceIoControl(hDevice, 0x22E00A, buf, buf_len, CMD_CPU_UTILIZATION->response->data, CMD_CPU_UTILIZATION->response->len, &len, NULL))
        {
            puts("Device returned: ");
            buf_debug(CMD_CPU_UTILIZATION->response->data, CMD_CPU_UTILIZATION->response->len);
        }
        else
        {
            puts("Failed to get device CPU utilization.");
        }
    }
    else
    {
        puts("Failed to talk with device.");
    }

    CloseHandle(hDevice);

    return 0;
}
