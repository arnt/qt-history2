/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ActiveQtEXE_h__
#define __ActiveQtEXE_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IQActiveX_FWD_DEFINED__
#define __IQActiveX_FWD_DEFINED__
typedef interface IQActiveX IQActiveX;
#endif 	/* __IQActiveX_FWD_DEFINED__ */


#ifndef ___IQActiveXEvents_FWD_DEFINED__
#define ___IQActiveXEvents_FWD_DEFINED__
typedef interface _IQActiveXEvents _IQActiveXEvents;
#endif 	/* ___IQActiveXEvents_FWD_DEFINED__ */


#ifndef __QActiveX_FWD_DEFINED__
#define __QActiveX_FWD_DEFINED__

#ifdef __cplusplus
typedef class QActiveX QActiveX;
#else
typedef struct QActiveX QActiveX;
#endif /* __cplusplus */

#endif 	/* __QActiveX_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IQActiveX_INTERFACE_DEFINED__
#define __IQActiveX_INTERFACE_DEFINED__

/* interface IQActiveX */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IQActiveX;

// IID_IQActiveX
// MIDL_INTERFACE("616F620B-91C5-4410-A74E-6B81C76FFFE0")
struct __declspec(novtable)
IQActiveX : public IDispatch
{
public:
};

#endif 	/* __IQActiveX_INTERFACE_DEFINED__ */



#ifndef __ACTIVEQTEXELib_LIBRARY_DEFINED__
#define __ACTIVEQTEXELib_LIBRARY_DEFINED__

/* library ACTIVEQTEXELib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ACTIVEQTEXELib;

#ifndef ___IQActiveXEvents_DISPINTERFACE_DEFINED__
#define ___IQActiveXEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IQActiveXEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IQActiveXEvents;

// DIID__IQActiveXEvents
// MIDL_INTERFACE("E1816BBA-BF5D-4A31-9855-D6BA432055FF")
struct __declspec(novtable)
_IQActiveXEvents : public IDispatch
{
};

#endif 	/* ___IQActiveXEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_QActiveX;
/*
#ifdef __cplusplus

// CLSID_QActiveX
class __declspec(uuid("DF16845C-92CD-4AAB-A982-EB9840E74669"))
QActiveX;
#endif
*/
EXTERN_C const IID IID_ActiveQtApp;

#endif /* __ACTIVEQTEXELib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
