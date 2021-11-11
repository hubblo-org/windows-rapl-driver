#include <iostream>
#include <windows.h>

using namespace std;

#define SERVICE_NAME "RAPLDrvService"
#define SERVICE_DESC "RAPL Driver Service"
#define DRIVER_PATH "C:\\Users\\Vico\\source\\repos\\RAPLDrv\\x64\\Debug\\RAPLDrv.sys"

int main()
{
    char c;
	SC_HANDLE scHandle, srvHandle;

    /* Open Service Manager */
	scHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (scHandle == NULL)
    {
        cout << "Failed to open Service Manager." << endl;
        cin >> c;
        ExitProcess(-1);
    }

    /* Check if th service already exists... */
	srvHandle = OpenService(scHandle, SERVICE_NAME, GENERIC_ALL);
	if (srvHandle == NULL)
	{
        /* ... otherwise try to create it */
		srvHandle = CreateService(scHandle, SERVICE_NAME, SERVICE_DESC,
			GENERIC_ALL, SERVICE_FILE_SYSTEM_DRIVER,
			SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
			DRIVER_PATH, NULL,
			NULL, NULL,
			NULL, NULL);
        if (srvHandle == NULL)
        {
    		cout << "Failed to create " << SERVICE_DESC << ". Error code: " << GetLastError() << endl;
            cin >> c;
            ExitProcess(-1);
        }
	}

    /* Start the service */
  	if (!StartService(srvHandle, 0, NULL))
    {
        cout << "Failed to start " << SERVICE_DESC << ". As the driver is unsigned, you might need to disable Windows signature enforcement and restart your computer." << endl;
        cin >> c;
        ExitProcess(-1);
    }

    cout << SERVICE_DESC << " started successfully!" << endl;
    cin >> c;

    ExitProcess(0);
}
