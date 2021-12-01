#include <iostream>
#include "Agent.h"

using namespace std;

int main()
{
    OpenDevice();

    SendRequest(AGENT_POWER_UNIT_CODE, NULL, 0, msrResult, sizeof(MSR_REGISTER_T));
    SendRequest(AGENT_POWER_LIMIT_CODE, NULL, 0, msrResult, sizeof(MSR_REGISTER_T));
    SendRequest(AGENT_ENERGY_STATUS_CODE, NULL, 0, msrResult, sizeof(MSR_REGISTER_T));

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
    if (DeviceIoControl(hDevice, CTL_CODE(FILE_DEVICE_UNKNOWN, requestCode, METHOD_OUT_DIRECT, FILE_READ_DATA),
        (LPVOID)request, requestLength, /* Driver does not need input data so set it to NULL */
        msrResult, sizeof(MSR_REGISTER_T), /* Set MSR buffer to store answer from driver */
        &len, NULL))
    {
        puts("Device answered!");
        if (len != replyLength)
        {
            printf("Uh oh, got invalid length answer. Expected %i, got %i\n", replyLength, len);
            return FALSE;
        }

        puts("Got answer:");
        for (i = 0; i < replyLength; ++i)
        {
            if ((i % 16) == 0)
                puts("");
            printf("%02x ", reply[i]);
        }
        puts("");
    }
    else
    {
        printf("Failed to talk with device. Error code: %u\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}
