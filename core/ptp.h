#ifndef __PTP_H
#define __PTP_H

#define PTP_OC_CHDK 0x9999

#define PTP_RC_OK 0x2001
#define PTP_RC_GeneralError 0x2002
#define PTP_RC_ParameterNotSupported 0x2006

enum {
  PTP_CHDK_Shutdown = 0,    // param2 is 0 (hard), 1 (soft), 2 (reboot) or 3 (reboot fw update)
                            // if param2 == 3, then filename of fw update is send as data (empty for default)
  PTP_CHDK_GetMemory,       // param2 is base address (not NULL; circumvent by taking 0xFFFFFFFF and size+1)
                            // param3 is size (in bytes)
                            // return data is memory block
  PTP_CHDK_SetMemoryLong,   // param2 is address
                            // param3 is value
  PTP_CHDK_CallFunction,    // data is array of function pointer and (long) arguments  (max: 10 args)
                            // return param1 is return value
  PTP_CHDK_GetPropCase,     // param2 is base id
                            // param3 is number of properties
                            // return data is array of longs
  PTP_CHDK_GetParamData,    // param2 is base id
                            // param3 is number of parameters
                            // return data is sequence of strings prefixed by their length (as long)
  PTP_CHDK_TempData,        // data is data to be stored for later
  PTP_CHDK_UploadFile,      // data is 4-byte length of filename, followed by filename and contents
  PTP_CHDK_DownloadFile,    // preceded by PTP_CHDK_TempData with filename
                            // return data are file contents
  PTP_CHDK_SwitchMode,      // param2 is 0 (playback) or 1 (record)
  PTP_CHDK_ExecuteLUA,      // data is script to be executed
} ptp_chdk_command;

#endif // __PTP_H
