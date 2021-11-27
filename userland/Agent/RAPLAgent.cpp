#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
    HANDLE hDevice;
    uint8_t buf[8];
    DWORD len;
    uint64_t seq;
    size_t buf_len;

    hDevice = CreateFile(L"\\\\.\\RAPLDriver", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        puts("Failed to open device");
        return 1;
    }
    puts("Device opened!");

    memset(buf, 0, 8);
    if (DeviceIoControl(hDevice, CTL_CODE(FILE_DEVICE_UNKNOWN, 0xBEB, METHOD_OUT_DIRECT, FILE_READ_DATA), buf, 8, buf, 8, &len, NULL))
    {
        puts("Device answered!");
    }
    else
    {
        puts("Failed to talk with device.");
    }

    CloseHandle(hDevice);

    return 0;
}