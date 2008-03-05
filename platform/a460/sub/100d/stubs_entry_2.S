#include "stubs_asm.h"

// maybe 0xFFDBA0A8 but it is not used in CHDK at all
NHSTUB(DisplayImagePhysicalScreen, 0xFFDBA0A8)

NHSTUB(Mount_FileSystem, 0xFFE1CAF4)
NHSTUB(free, 0xFFCCC760)
NHSTUB(kbd_read_keys_r2, 0xFFDC4AC8) 

NHSTUB(SetZoomActuatorSpeedPercent, 0xFFC017E8)  //null stub
       
NHSTUB(kbd_pwr_off, 0xFFC017E8)   //null stub
NHSTUB(kbd_pwr_on, 0xFFC017E8)    //null stub                                                      

NHSTUB(SetPropertyCase,0xFFC0B664) 

NHSTUB(GetDrive_ClusterSize, 0xFFE1CFBC)
NHSTUB(GetDrive_TotalClusters, 0xFFE1CFF8)
NHSTUB(GetDrive_FreeClusters, 0xFFE1D034)