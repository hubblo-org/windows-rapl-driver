#include <iostream>
#include <windows.h>

using namespace std;

#define SERVICE_NAME "RAPLDrvService"
#define SERVICE_DESC "RAPL Driver Service"
#define DRIVER_PATH "C:\\Users\\Vico\\source\\repos\\RAPLDrv\\x64\\Debug\\RAPLDrv.sys"

typedef enum {
    ERROR_SERVICE_OK,
    ERROR_SERVICE_FAILED_TO_OPEN_MANAGER,
    ERROR_SERVICE_CREATE_FAILED,
    ERROR_SERVICE_START_FAILED,
    ERROR_SERVICE_STOP_FAILED,
    ERROR_SERVICE_REMOVE_FAILED,
    ERROR_SERVICE_UNAVAILABLE,
    ERROR_SERVICE_ALREADY_EXISTS
}e_service_error;

void ShowHelp(void);
void EnsureServiceExists(SC_HANDLE manager, SC_HANDLE* srvHandle);
SC_HANDLE GetOrCreateService(SC_HANDLE manager, BOOL create = FALSE);

int main(int argc, char **argv)
{
	SC_HANDLE scHandle, srvHandle;
    SERVICE_STATUS srvStatus;

    if (argc < 2)
        ShowHelp();

    /* Open Service Manager */
	scHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (scHandle == NULL)
    {
        cout << "Failed to open Service Manager. Run as admin?" << endl;
        ExitProcess(ERROR_SERVICE_FAILED_TO_OPEN_MANAGER);
    }

    if (strcmp(argv[1], "install") == 0)
    {
        srvHandle = GetOrCreateService(scHandle);
        if (srvHandle)
        {
            cout << "Service already exists!" << endl;
            ExitProcess(ERROR_SERVICE_ALREADY_EXISTS);
        }

        srvHandle = GetOrCreateService(scHandle, TRUE);
        if (!srvHandle)
        {
            cout << "Failed to create " << SERVICE_DESC << ". Error code: " << GetLastError() << endl;
            ExitProcess(ERROR_SERVICE_CREATE_FAILED);
        }
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        EnsureServiceExists(scHandle, &srvHandle);
        if (!StartService(srvHandle, 0, NULL))
        {
            cout << "Failed to start " << SERVICE_DESC << ". As the driver is unsigned, you might need to disable Windows signature enforcement and restart your computer." << endl;
            ExitProcess(ERROR_SERVICE_START_FAILED);
        }

        cout << SERVICE_DESC << " started successfully!" << endl;
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        EnsureServiceExists(scHandle, &srvHandle);
        if (!ControlService(srvHandle, 0, &srvStatus))
        {
            cout << "Failed to stop " << SERVICE_DESC << "." << endl;
            ExitProcess(ERROR_SERVICE_STOP_FAILED);
        }

        cout << SERVICE_DESC << " started successfully!" << endl;
    }
    else if (strcmp(argv[1], "remove") == 0)
    {
        EnsureServiceExists(scHandle, &srvHandle);
        if (!DeleteService(srvHandle))
        {
            cout << "Failed to remove " << SERVICE_DESC << "." << endl;
            ExitProcess(ERROR_SERVICE_REMOVE_FAILED);
        }

        cout << SERVICE_DESC << " removed successfully!" << endl;
    }
    else
    {
        ShowHelp();
    }

    ExitProcess(ERROR_SERVICE_OK);
}

SC_HANDLE GetOrCreateService(SC_HANDLE manager, BOOL create)
{
    if (!create)
        return OpenService(manager, SERVICE_NAME, GENERIC_ALL);

    return CreateService(manager, SERVICE_NAME, SERVICE_DESC,
                         SERVICE_ALL_ACCESS, SERVICE_FILE_SYSTEM_DRIVER,
                         SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                         DRIVER_PATH, NULL,
                         NULL, NULL,
                         NULL, NULL);
}

void ShowHelp(void)
{
    cout << "Usage: DriverLoader [install|start|stop|remove]" << endl;
    ExitProcess(ERROR_SERVICE_OK);
}

void EnsureServiceExists(SC_HANDLE manager, SC_HANDLE *srvHandle)
{
    *srvHandle = GetOrCreateService(manager);
    if (!*srvHandle)
    {
        cout << "The service does not exist!" << endl;
        ExitProcess(ERROR_SERVICE_UNAVAILABLE);
    }
}