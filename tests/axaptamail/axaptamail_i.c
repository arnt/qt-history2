/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Oct 24 19:58:52 2001
 */
/* Compiler settings for C:\depot\qt\main\tests\axaptamail\axaptamail.idl:
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

const IID IID_ITemplateParser = {0xE514925C,0x6823,0x419D,{0xAF,0x2B,0x9B,0xA1,0xB7,0x46,0xE8,0xB5}};


const IID LIBID_AXAPTAMAILLib = {0xA4CC5C89,0x1ECA,0x45D7,{0x95,0x7F,0x23,0xC8,0x19,0x89,0x76,0x7A}};


const CLSID CLSID_TemplateParser = {0x74013298,0x9F5A,0x41F5,{0xBC,0x49,0x63,0x04,0xB0,0x59,0x21,0xDC}};


#ifdef __cplusplus
}
#endif

