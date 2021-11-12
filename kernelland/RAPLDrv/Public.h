/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_RAPLDrv,
    0x03a47afd,0xcb09,0x47e7,0xa2,0x72,0xe0,0x9a,0x9a,0x00,0xbd,0xc7);
// {03a47afd-cb09-47e7-a272-e09a9a00bdc7}
