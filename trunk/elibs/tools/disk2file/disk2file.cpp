#include <windows.h>
#include <winioctl.h>  // From the Win32 SDK \Mstools\Include
#include "ntddcdrm.h"  // From the Windows NT DDK \Ddk\Src\Storage\Inc

/*
   This code reads sectors 16 and 17 from a compact disc and writes
   the contents to a disk file named Sector.dat
*/


void main (void)
{ 
   HANDLE  hCD, hFile;
   DWORD   dwNotUsed;

   //  Disk file that will hold the CD-ROM sector data.
   hFile = CreateFile ("sector.dat",
                       GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);

   // For the purposes of this sample, drive F: is the CD-ROM
   // drive.
   hCD = CreateFile ("\\\\.\\A:", GENERIC_READ,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                     NULL);

   // If the CD-ROM drive was successfully opened, read sectors 16
   // and 17 from it and write their contents out to a disk file.
   if (hCD != INVALID_HANDLE_VALUE)
   {
      DISK_GEOMETRY         dgCDROM;
      PREVENT_MEDIA_REMOVAL pmrLockCDROM;

      // Lock the compact disc in the CD-ROM drive to prevent accidental
      // removal while reading from it.
      pmrLockCDROM.PreventMediaRemoval = TRUE;
      DeviceIoControl (hCD, IOCTL_CDROM_MEDIA_REMOVAL,
                       &pmrLockCDROM, sizeof(pmrLockCDROM), NULL,
                       0, &dwNotUsed, NULL);

      // Get sector size of compact disc
      if (DeviceIoControl (hCD, IOCTL_CDROM_GET_DRIVE_GEOMETRY,
                           NULL, 0, &dgCDROM, sizeof(dgCDROM),
                           &dwNotUsed, NULL))
      {
         LPBYTE lpSector;
         DWORD  dwSize = 2 * dgCDROM.BytesPerSector;  // 2 sectors

         // Allocate buffer to hold sectors from compact disc. Note that
         // the buffer will be allocated on a sector boundary because the
         // allocation granularity is larger than the size of a sector on a
         // compact disk.
         lpSector = VirtualAlloc (NULL, dwSize,
                                  MEM_COMMIT|MEM_RESERVE,
                                  PAGE_READWRITE);

         // Move to 16th sector for something interesting to read.
         SetFilePointer (hCD, dgCDROM.BytesPerSector * 16,
                         NULL, FILE_BEGIN);

         // Read sectors from the compact disc and write them to a file.
         if (ReadFile (hCD, lpSector, dwSize, &dwNotUsed, NULL))
            WriteFile (hFile, lpSector, dwSize, &dwNotUsed, NULL);

         VirtualFree (lpSector, 0, MEM_RELEASE);
      }

      // Unlock the disc in the CD-ROM drive.
      pmrLockCDROM.PreventMediaRemoval = FALSE;
      DeviceIoControl (hCD, IOCTL_CDROM_MEDIA_REMOVAL,
                       &pmrLockCDROM, sizeof(pmrLockCDROM), NULL,
                       0, &dwNotUsed, NULL);

      CloseHandle (hCD);
      CloseHandle (hFile);
   }

} 