/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Oct 23 11:08:49 2000
 */
/* Compiler settings for C:\Documents and Settings\ebakke\My Documents\MyProjects\ActiveQtEXE\ActiveQtEXE.idl:
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

const IID IID_IQActiveX = {0x616F620B,0x91C5,0x4410,{0xA7,0x4E,0x6B,0x81,0xC7,0x6F,0xFF,0xE0}};


const IID LIBID_ACTIVEQTEXELib = {0xEC08F8FC,0x2754,0x47AB,{0x8E,0xFE,0x56,0xA5,0x40,0x57,0xF3,0x4E}};


const IID DIID__IQActiveXEvents = {0xE1816BBA,0xBF5D,0x4A31,{0x98,0x55,0xD6,0xBA,0x43,0x20,0x55,0xFF}};


const CLSID CLSID_QActiveX = {0xDF16845C,0x92CD,0x4AAB,{0xA9,0x82,0xEB,0x98,0x40,0xE7,0x46,0x69}};


#ifdef __cplusplus
}
#endif

