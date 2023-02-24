#include <iostream>
#include <windows.h>
#include <wdmguid.h>

using namespace std;

#define SERVICE_NAME "ScaphandreDrv"
#define SERVICE_DESC "Scaphandre Driver Service"
#define DRIVER_EXE "ScaphandreDrv.sys"

typedef enum {
    ERROR_SERVICE_OK,
    ERROR_SERVICE_FAILED_TO_OPEN_MANAGER,
    ERROR_SERVICE_CREATE_FAILED,
    ERROR_SERVICE_START_FAILED,
    ERROR_SERVICE_STOP_FAILED,
    ERROR_SERVICE_REMOVE_FAILED,
    ERROR_SERVICE_UNAVAILABLE,
    ERROR_SERVICE_ALREADY_EXISTS,
    ERROR_SERVICE_RESTRICTED_PRIVILEGES,
    ERROR_SERVICE_FILE_NOT_FOUND
}e_service_error;

static SC_HANDLE scHandle, srvHandle;
SERVICE_STATUS srvStatus;
static char message[255];

void ShowHelp(void);
BOOL IsProcessElevated(void);
void EnsureServiceExists(SC_HANDLE manager, SC_HANDLE* srvHandle);
SC_HANDLE GetOrCreateService(SC_HANDLE manager, BOOL create = FALSE);
void _doInstall(void);

int main(int argc, char **argv)
{
    if (!IsProcessElevated())
    {
        MessageBox(NULL, "The application is not running with administrator rights. Please restart the application with administrator rights.", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(ERROR_SERVICE_RESTRICTED_PRIVILEGES);
    }

    /* Open Service Manager */
	scHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scHandle == NULL)
    {
        cout << "Failed to open Service Manager. Run as admin?" << endl;
        ExitProcess(ERROR_SERVICE_FAILED_TO_OPEN_MANAGER);
    }

    if (argc < 2)
    {
        // Execute install function by default
        _doInstall();
    }

    if (strcmp(argv[1], "install") == 0)
    {
        _doInstall();
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
        if (!ControlService(srvHandle, SERVICE_CONTROL_STOP, &srvStatus))
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
    HANDLE hFile;
    LPSTR currentPath;
    CHAR pathToExe[MAX_PATH];

    if (!create)
        return OpenService(manager, SERVICE_NAME, SERVICE_ALL_ACCESS);

    currentPath = new CHAR[MAX_PATH];
    GetCurrentDirectory(MAX_PATH - 1, currentPath);
    sprintf_s(pathToExe, MAX_PATH - 1, "%s\\%s", currentPath, DRIVER_EXE);
    delete[] currentPath;

    hFile = CreateFile(pathToExe, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        snprintf(message, sizeof(message) - 1, "Could not find driver file %s. The driver file must be placed in the same directory of the installer.", DRIVER_EXE);
        MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
        ExitProcess(ERROR_SERVICE_FILE_NOT_FOUND);
    }
    CloseHandle(hFile);
    
    return CreateService(manager, SERVICE_NAME, SERVICE_DESC,
                         SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                         SERVICE_SYSTEM_START, SERVICE_ERROR_NORMAL,
                         pathToExe, NULL,
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
        MessageBox(NULL, "The service does not exist!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(ERROR_SERVICE_UNAVAILABLE);
    }
}

BOOL IsProcessElevated(void)
{
    DWORD dwSize;
    HANDLE hToken = NULL;
    TOKEN_ELEVATION elevation;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        snprintf(message, sizeof(message) - 1, "OpenProcessToken failed: %i.", GetLastError());
        MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
        ExitProcess(ERROR_SERVICE_RESTRICTED_PRIVILEGES);
    }

    if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
    {
        snprintf(message, sizeof(message) - 1, "GetTokenInformation failed: %i.", GetLastError());
        MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hToken);
        ExitProcess(ERROR_SERVICE_RESTRICTED_PRIVILEGES);
    }

    return elevation.TokenIsElevated;
}

void _doInstall(void)
{
    CHAR pathToExe[MAX_PATH];
    CHAR pathToSys[MAX_PATH];

    GetSystemDirectory(pathToSys, MAX_PATH - 1);
    snprintf(pathToExe, MAX_PATH - 1, "%s\\%s", pathToSys, "bcdedit.exe");

    /* Launch commands to disable Windows signature enforcement */
    ShellExecute(NULL, "open", pathToExe, "/set testsigning on", NULL, SW_SHOWMINNOACTIVE);
    ShellExecute(NULL, "open", pathToExe, "/set nointegritychecks on", NULL, SW_SHOWMINNOACTIVE);

    srvHandle = GetOrCreateService(scHandle);
    if (srvHandle)
    {
        MessageBox(NULL, "Scaphandre driver is already installed!", "Info", MB_OK | MB_ICONINFORMATION);
        ExitProcess(ERROR_SERVICE_ALREADY_EXISTS);
    }

    srvHandle = GetOrCreateService(scHandle, TRUE);
    if (!srvHandle)
    {
        snprintf(message, sizeof(message) - 1, "Failed to create %s. Error code: %i", SERVICE_DESC, GetLastError());
        MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
        ExitProcess(ERROR_SERVICE_CREATE_FAILED);
    }

    snprintf(message, sizeof(message) - 1, "%s successfully created and installed!. The computer must be restarted in order to activate the service.", SERVICE_DESC);
    MessageBox(NULL, message, "Success", MB_OK | MB_ICONINFORMATION);
    ExitProcess(ERROR_SERVICE_OK);
}