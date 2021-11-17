#pragma once

#include <cstdint>
#include <cstring>
#include <Windows.h>


typedef struct extra_cmd_s {
	size_t len;
	uint8_t* data;
}extra_cmd_t;

typedef struct response_cmd_s {
	size_t len;
	uint8_t* data;
}response_cmd_t;

typedef struct command_s {
	uint32_t u32_1; // always set to 0
	uint32_t id;
	extra_cmd_t *extra;
	response_cmd_t *response;
}command_t;

uint8_t* cmd_to_buf(command_t *cmd, size_t *out_len)
{
	uint8_t *res;

	if (!cmd->extra)
	{
		res = new uint8_t[sizeof(command_t) - sizeof(extra_cmd_t*) - sizeof(response_cmd_t*)];
		*out_len = sizeof(command_t) - sizeof(extra_cmd_t*) - sizeof(response_cmd_t*);
	}
	else
	{
		res = new uint8_t[sizeof(command_t) - sizeof(extra_cmd_t*) - sizeof(response_cmd_t*) + cmd->extra->len];
		*out_len = sizeof(command_t) - sizeof(extra_cmd_t*) - sizeof(response_cmd_t*) + cmd->extra->len;
	}

	memcpy(res, &cmd->u32_1, sizeof(uint32_t));
	memcpy(res + sizeof(uint32_t), &cmd->id, sizeof(uint32_t));
	if (cmd->extra)
		memcpy(res + sizeof(command_t) - sizeof(extra_cmd_t*) - sizeof(response_cmd_t*), cmd->extra->data, cmd->extra->len);

	return res;
}

command_t* make_power_units_request()
{
	command_t *cmd = new command_t{ 0, 0x606, 0, 0 };
	response_cmd_t* response_cmd = new response_cmd_t{ 0, 0 };

	response_cmd->len = sizeof(uint64_t);
	response_cmd->data = new uint8_t[response_cmd->len];
	memset(response_cmd->data, 0, response_cmd->len);

	cmd->response = response_cmd;
	return cmd;
}

command_t* make_package_power_limit_request()
{
	command_t *cmd = new command_t{ 0, 0x610, 0, 0 };
	response_cmd_t* response_cmd = new response_cmd_t{ 0, 0 };

	response_cmd->len = sizeof(uint64_t);
	response_cmd->data = new uint8_t[response_cmd->len];
	memset(response_cmd->data, 0, response_cmd->len);

	cmd->response = response_cmd;
	return cmd;
}

command_t* make_current_power_request()
{
	command_t *cmd = new command_t{ 0, 0x611, 0, 0 };
	response_cmd_t* response_cmd = new response_cmd_t{ 0, 0 };

	response_cmd->len = sizeof(uint64_t);
	response_cmd->data = new uint8_t[response_cmd->len];
	memset(response_cmd->data, 0, response_cmd->len);

	cmd->response = response_cmd;
	return cmd;
}

command_t* make_test_request()
{
	command_t *cmd = new command_t{ 0, 3, 0, 0 };
	response_cmd_t* response_cmd = new response_cmd_t{ 0, 0 };

	response_cmd->len = sizeof(uint64_t);
	response_cmd->data = new uint8_t[response_cmd->len];
	memset(response_cmd->data, 0, response_cmd->len);

	cmd->response = response_cmd;
	return cmd;
}

command_t* make_cpu_utilization_request()
{
	SYSTEM_INFO sysinfo;
	command_t *cmd = new command_t{ 0, 0xE7, 0, 0 };
	extra_cmd_t *extra = new extra_cmd_t{ sizeof(uint32_t), 0 };
	response_cmd_t *response_cmd = new response_cmd_t{ 0, 0 };

	GetSystemInfo(&sysinfo);
	extra->data = new uint8_t[extra->len];
	memcpy(extra->data, &sysinfo.dwNumberOfProcessors, extra->len);

	response_cmd->len = sysinfo.dwNumberOfProcessors * sizeof(uint64_t);
	response_cmd->data = new uint8_t[response_cmd->len];
	memset(response_cmd->data, 0, response_cmd->len);

	cmd->extra = extra;
	cmd->response = response_cmd;
	return cmd;
}
