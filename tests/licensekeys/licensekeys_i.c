/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Oct 24 10:36:04 2001
 */
/* Compiler settings for C:\depot\qt\main\tests\licensekeys\licensekeys.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_ILicenseKeyGenerator = {0xE19839D3,0xA96B,0x4732,{0x95,0x7A,0xD8,0x57,0xF1,0xF9,0x0E,0xBB}};


const IID LIBID_LICENSEKEYSLib = {0xEB42E3F8,0x17A9,0x488D,{0x85,0xA4,0xE3,0x20,0x00,0x46,0x10,0x7B}};


const CLSID CLSID_LicenseKeyGenerator = {0x3F868798,0x3D8A,0x4186,{0x8B,0x6C,0x4A,0xF8,0xE2,0x38,0x35,0x7B}};


#ifdef __cplusplus
}
#endif

