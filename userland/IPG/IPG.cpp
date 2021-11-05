// IPG.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <Windows.h>
#include "types.h"

double ENERGY_UNITS = 0.;
double POWER_UNITS = 0.;
double TIME_UNITS = 0.;

auto CMD_POWER_UNITS = make_power_units_request();
auto CMD_CURRENT_POWER_REQUEST = make_current_power_request();
auto CMD_POWER_LIMIT_REQUEST = make_package_power_limit_request();
auto CMD_CPU_UTILIZATION = make_cpu_utilization_request();
auto CMD_CPU_TEST = make_test_request();

static void extract_rapl_power_units( uint64_t data );
static void extract_rapl_power_limits( uint64_t data );
static void extract_rapl_current_power( uint64_t data );

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
    size_t buf_len;
	uint64_t val;

    hDevice = CreateFile(L"\\\\.\\EnergyDriver", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        puts("Failed to open device");
        return 1;
    }
    puts("Device opened!");

    buf = cmd_to_buf(CMD_POWER_UNITS, &buf_len);
    //buf_debug(buf, buf_len);

    if (DeviceIoControl(hDevice, 0x22E00A, buf, buf_len, CMD_POWER_UNITS->response->data, CMD_POWER_UNITS->response->len, &len, NULL))
    {
        /*puts("Device returned: ");
        buf_debug(CMD_POWER_UNITS->response->data, CMD_POWER_UNITS->response->len);*/
		memcpy(&val, CMD_POWER_UNITS->response->data, CMD_POWER_UNITS->response->len);
		extract_rapl_power_units(val);
		
		buf = cmd_to_buf(CMD_POWER_LIMIT_REQUEST, &buf_len);
        //buf_debug(buf, buf_len);
        if (DeviceIoControl(hDevice, 0x22E00A, buf, buf_len, CMD_POWER_LIMIT_REQUEST->response->data, CMD_POWER_LIMIT_REQUEST->response->len, &len, NULL))
        {
            /*puts("Device returned: ");
            buf_debug(CMD_POWER_LIMIT_REQUEST->response->data, CMD_POWER_LIMIT_REQUEST->response->len);*/
			memcpy(&val, CMD_POWER_LIMIT_REQUEST->response->data, CMD_POWER_LIMIT_REQUEST->response->len);
			extract_rapl_power_limits(val);
        }
        else
        {
            puts("Failed to get device CPU utilization.");
        }

		/*while (TRUE)
		{*/
			buf = cmd_to_buf(CMD_CURRENT_POWER_REQUEST, &buf_len);
			//buf_debug(buf, buf_len);
			if (DeviceIoControl(hDevice, 0x22E00A, buf, buf_len, CMD_CURRENT_POWER_REQUEST->response->data, CMD_CURRENT_POWER_REQUEST->response->len, &len, NULL))
			{
				puts("Device returned: ");
				buf_debug(CMD_CURRENT_POWER_REQUEST->response->data, CMD_CURRENT_POWER_REQUEST->response->len);
				memcpy(&val, CMD_CURRENT_POWER_REQUEST->response->data, CMD_CURRENT_POWER_REQUEST->response->len);
				extract_rapl_current_power(val);
			}
			else
			{
				puts("Failed to get device CPU utilization.");
			}
			
			Sleep(1000);
		//}
		/*
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
        }*/
    }
    else
    {
        puts("Failed to talk with device.");
    }

    CloseHandle(hDevice);

    return 0;
}

void extract_rapl_power_units( uint64_t data )
{
	// Intel documentation says high level bits are reserved, so ignore them
	uint16_t time;
	uint16_t power;
	uint32_t energy;
	double time_units;
	double power_units;
	double energy_units;
	
	data = data & 0xFFFFFFFF;
	
	// Power units are located from bits 0 to 3, extract them
	power = data & 0x0F;
	
	// Energy state units are located from bits 8 to 12, extract them
	energy = (data >> 8) & 0x1F;
	
	// Time units are located from bits 16 to 19, extract them
	time = (data >> 16) & 0x0F;
	
	// Intel documentation says: 1 / 2^power
	power_units = 1.0 / pow(2, static_cast<double>(power));
	
	// Intel documentation says: 1 / 2^energy
	energy_units = 1.0 / pow(2, static_cast<double>(energy));
	
	// Intel documentation says: 1 / 2^energy
	time_units = 1.0 / pow(2, static_cast<double>(time));
	
	printf("CPU energy unit is: %.6f microJ\n", energy_units * 1000000);
	printf("CPU power unit is: %.6f Watt(s)\n", power_units);
	printf("CPU time unit is: %.6f second(s)\n", time_units);
	ENERGY_UNITS = energy_units;
	POWER_UNITS = power_units;
	TIME_UNITS = time_units;
}

void extract_rapl_power_limits( uint64_t data )
{
	uint32_t power;
	uint32_t time_limit;
	uint8_t clamping_limitation;
	uint8_t power_limit_enabled;
	
	power = data & 0x3FFF;
	printf("CPU power limit is: %.6f Watts\n", power * POWER_UNITS);
}

void extract_rapl_current_power( uint64_t data )
{
	uint32_t energy_consumed;
	static double energy_consumed_prev = 0.;
	
	// High level bits are reserved, ignore them
	energy_consumed = data & 0xFFFFFFFF;
	
	printf("Current power usage: %.6f microJ\n", energy_consumed * ENERGY_UNITS * 1000000);
	printf("Current power usage: %.6f Watts\n", (((energy_consumed - energy_consumed_prev) * ENERGY_UNITS) / TIME_UNITS) / 1000);
	
	energy_consumed_prev = energy_consumed;
}