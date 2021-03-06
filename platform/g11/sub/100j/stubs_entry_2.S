#include "stubs_asm.h"
NHSTUB(TurnOffBackLight, 0xFF8A348C)
NHSTUB(TurnOnBackLight, 0xFF8A3474  )
NHSTUB(GiveSemaphore, 0xFF838E38)
NHSTUB(GetKbdState, 0xFF860424)
NHSTUB(kbd_read_keys_r2, 0xff85fb94)
NHSTUB(SetScriptMode, 0xFF810F4C)

NHSTUB(EnterToCompensationEVF, 0xFF8373D8)
NHSTUB(ExitFromCompensationEVF, 0xFF83741C)
NHSTUB(PT_PlaySound, 0xFF882414)
NHSTUB(UIFS_WriteFirmInfoToFile, 0xFF980560)
NHSTUB(MakeAFScan, 0xFF966130)
NHSTUB(ExpCtrlTool_StartContiAE, 0xFF844CE0)
NHSTUB(ExpCtrlTool_StopContiAE, 0xFF844DB8)
NHSTUB(Remove, 0xFF8359E4)
NHSTUB(readfastdir, 0xFFA84270)

NHSTUB(WriteSDCard, 0xFF94B750)    //                                   v.c"
NHSTUB(UnlockAF, 0xFF837190)
NHSTUB(DoAFLock, 0xFF837158)
NHSTUB(UnsetZoomForMovie,0xFF99CE28)
NHSTUB(PutInNdFilter, 0xFFB00F1C)
NHSTUB(PutOutNdFilter, 0xFFB00F54)
NHSTUB(closefastdir, 0xFFA840F8)
NHSTUB(openfastdir, 0xFFA8413C)
NHSTUB(kbd_p1_sub, 0xFF833584)
NHSTUB(ExportToEventProcedure,0xFF88A33C)
NHSTUB(UnRegisterEvntProc,0xFF889870)
NHSTUB(SetFileTimeStamp, 0xFF835C3C) // filename instead of filehandle
NHSTUB(GetLogToFile,0xFF96D104) // OK
NHSTUB(apex2us, 0xFFA148C0)
NHSTUB(strrchr, 0xFF813FE4) // correction for sig.finder addr.
NHSTUB(strncpy, 0xff813f38) // sigfinder founds same addr. for strcat
NHSTUB(strcat, 0xFF8AA80C)  // sigfinder founds same addr. for strncpy
NHSTUB(LEDDrive,0xFF85F674)
NHSTUB(RefreshPhysicalScreen, 0xFFA0FED8)
NHSTUB(ScreenLock, 0xFFA11108)
NHSTUB(ScreenUnlock, 0xFFA11130)
NHSTUB(Lseek, 0xff835acc) // same address as lseek
NHSTUB(rename, 0xFF835A8C) 
NHSTUB(RenameFile_Fut, 0xFF8352A8)

// nullsubs
NHSTUB(kbd_pwr_on, 0xFF810B04) // nullsub
NHSTUB(rewinddir, 0xFF810B04)  // nullsub
NHSTUB(Unmount_FileSystem, 0xFF810B04)  // nullsub
NHSTUB(Mount_FileSystem, 0xFF810B04)  // nullsub
NHSTUB(SetZoomActuatorSpeedPercent, 0xFF810B04)  // nullsub
