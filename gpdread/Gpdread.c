#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>
#include "gpioctl.h"        // This defines the IOCTL constants.
#include <dontuse.h>

#define READ_TIMEOUT 500

VOID CompletedReadRoutine(DWORD,DWORD,LPOVERLAPPED);
// VOID HandleASuccessfulRead(UCHAR lpBuf,ULONG dwRead);

VOID __cdecl
main(
    __in ULONG argc,
    __in_ecount(argc) PCHAR argv[]
    )
{
  HANDLE hndFile;
  UCHAR readSymbol = 'p';
  ULONG readSymbolDataLength = sizeof(readSymbol);
  BOOL readResult;
  UCHAR c;
  // ULONG numOfBytesLength = 0;
  // DWORD dwRes;
  // int i = 0;
  // BOOL fWaitingOnRead = FALSE;
  OVERLAPPED ov = {0,0,0,0,NULL};

  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
/*----------------------------------------------------------------------------*/

  hndFile = CreateFile(
                "\\\\.\\GpdDev",    // Open the Device "file"
                GENERIC_READ,       // access rights
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED, //установка OVERLAPPED флага
                NULL
                );

  if (hndFile == INVALID_HANDLE_VALUE){        // Was the device opened?
    printf("Unable to open the device.\n");
    exit(1);
  }
/*----------------------------------------------------------------------------*/
  ov.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
/*
  if (ov.hEvent == NULL){
    printf("CreateEvent failed with %d. \n", GetLastError());
    exit(1);
  }
  do{
    if (!fWaitingOnRead){
      if (!ReadFileEx(
                      hndFile, 
                      &readSymbol, 
                      readSymbolDataLength, 
                      &ov, 
                      (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine))
      {
        if (GetLastError() != ERROR_IO_PENDING){
          printf("problems with compiling reqest\n");
          exit(1);
        }
        else
          fWaitingOnRead = TRUE;
      }
      else{
        HandleASuccessfulRead(readSymbol, numOfBytesLength);
      }
    }
    else{
      if (fWaitingOnRead){
        dwRes = WaitForSingleObject(ov.hEvent, READ_TIMEOUT);
        switch(dwRes){
          case WAIT_OBJECT_0:
            if (!GetOverlappedResult(hndFile, &ov, &ov.InternalHigh, FALSE))
              printf("Error in communications; report it.\n");
            else{
              printf("Read completed successfully.\n");
              HandleASuccessfulRead(readSymbol, numOfBytesLength);
            }
            //  Reset flag so that another opertion can be issued.
            fWaitingOnRead = FALSE;
            break;
          case WAIT_TIMEOUT:
            // Operation isn't complete yet. fWaitingOnRead flag isn't
            // changed since I'll loop back around, and I don't want
            // to issue another read until the first one finishes.
            //
            // This is a good time to do some background work.
            printf("%s\n", "WAIT_TIMEOUT");
            break; 
          default:
            // Error in the WaitForSingleObject; abort.
            // This indicates a problem with the OVERLAPPED structure's
            // event handle.
            printf("%s\n", "default");
            break;
        }
      }
    }
    i = i + 1;
  } while (i != 10);              */
  /*------------------------------------------------------------------------*/
  do {
    // WaitForSingleObjectEx(hndFile, 3000, TRUE);
    readResult = 1;
    if (ReadFileEx(
                 hndFile,
                 &readSymbol,
                 readSymbolDataLength,
                 &ov,
                 NULL))
      printf("test to immediately return from ReadFileEx and continue processing this thread\n");
    if (readResult == 0){
      printf("Read failed, code %i \n", GetLastError());
      exit(1);
    }
    
    if (ov.InternalHigh == 1)
      printf("Read symbol from COM1: %c\n",readSymbol );
    scanf_s("%c",&c,1);
  } while (c != 'e');

/*----------------------------------------------------------------------------*/

  if (!CloseHandle(hndFile)){                  // Close the Device "file".
    printf("Failed to close device.\n");
  }
  exit(0);
}

// VOID  HandleASuccessfulRead(UCHAR lpBuf,ULONG dwRead){
//   UNREFERENCED_PARAMETER(dwRead);
//   // if (dwRead != 0)
//   printf("Symbol from COM1 port is %c: \n", lpBuf);
//   // else
//   // printf("%s\n", "nothing to print");
// }

VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{
  UNREFERENCED_PARAMETER(lpOverLap);
  UNREFERENCED_PARAMETER(dwErr);
  UNREFERENCED_PARAMETER(cbBytesRead);
  // if ((dwErr == 0) && (cbBytesRead != 0))
  printf("%s\n", "ReadFileEx completed successfully");
}
