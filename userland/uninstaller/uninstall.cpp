#include <windows.h>
#include <setupapi.h>
#include <cstdio>

#pragma comment(lib, "setupapi")

#define INF_FILE "ScaphandreDrv.inf"

int main(void)
{
    HANDLE hFile;
    CHAR message[256];
    CHAR cmdLine[512];
    CHAR currentPath[MAX_PATH];
    CHAR pathToFile[MAX_PATH];

    GetCurrentDirectory(MAX_PATH - 1, currentPath);
    snprintf(pathToFile, MAX_PATH - 1, "%s\\%s", currentPath, INF_FILE);

    hFile = CreateFile(pathToFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        snprintf(message, sizeof(message) - 1, "Could not find INF file %s. The driver file must be placed in the same directory of the INF file.", INF_FILE);
        MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    CloseHandle(hFile);

    snprintf(cmdLine, sizeof(cmdLine) - 1, "%s %s", "DefaultUninstall.Services 132", pathToFile);
	InstallHinfSection(NULL, NULL, cmdLine, 0);

    snprintf(cmdLine, sizeof(cmdLine) - 1, "%s %s", "DefaultUninstall 132", pathToFile);
	InstallHinfSection(NULL, NULL, cmdLine, 0);

	return 0;
}
