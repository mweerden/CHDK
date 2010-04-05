#ifndef __PTP_H
#define __PTP_H

// N.B.: not checking to see if CAM_CHDK_PTP is set as ptp.h is currently
// only included by ptp.c (which already checks this before including ptp.h)

#define PTP_CHDK_VERSION_MAJOR 0  // increase only with backwards incompatible changes (and reset minor)
#define PTP_CHDK_VERSION_MINOR 0  // increase with extensions of functionality

#define PTP_OC_CHDK 0x9999

#define PTP_RC_OK 0x2001
#define PTP_RC_GeneralError 0x2002
#define PTP_RC_ParameterNotSupported 0x2006

enum {
  PTP_CHDK_Version = 0,     // return param1 is major version number
                            // return param2 is minor version number
  PTP_CHDK_GetMemory,       // param2 is base address (not NULL; circumvent by taking 0xFFFFFFFF and size+1)
                            // param3 is size (in bytes)
                            // return data is memory block
  PTP_CHDK_SetMemory,       // param2 is address
                            // param3 is size (in bytes)
                            // data is new memory block
  PTP_CHDK_CallFunction,    // data is array of function pointer and (long) arguments  (max: 10 args)
                            // return param1 is return value
  PTP_CHDK_TempData,        // data is data to be stored for later
  PTP_CHDK_UploadFile,      // data is 4-byte length of filename, followed by filename and contents
  PTP_CHDK_DownloadFile,    // preceded by PTP_CHDK_TempData with filename
                            // return data are file contents
  PTP_CHDK_ExecuteScript,   // data is script to be executed
                            // param2 is language of script
} ptp_chdk_command;

// Script Languages
#define PTP_CHDK_SL_LUA    0
#define PTP_CHDK_SL_UBASIC 1

#endif // __PTP_H
