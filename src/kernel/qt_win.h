/*
 * This is a precompiled header file for use on Windows to greatly
 * speed the building of Qt. It may also be of use to people
 * developing their own project, but it is probably better to define
 * your own header.  Use of this header is currently UNSUPPORTED.
 */


#define _POSIX_ 	/* Make sure PATH_MAX et al. are defined    */
#include <limits.h>
#undef _POSIX_  	/* Don't polute                             */

/* Make sure IP v6 is defined first of all, before windows.h        */
#ifndef QT_NO_IPV6
# include <winsock2.h>
#endif
#include <stdlib.h>

#if defined __cplusplus

# include <private/qucomextra_p.h>  	/* For moc generated code   */
# include <qapplication.h>
# include <qbitmap.h>
# include <qcursor.h>
# include <qdatetime.h>
# include <qglobal.h>
# include <qimage.h>
# include <qmetaobject.h>  		/* For moc generated code   */
# include <qobject.h>
# include <qpainter.h>
# include <qpixmap.h>
# include <qplatformdefs.h>
# include <qptrlist.h>
# include <qstring.h>
# include <qstringlist.h>
# include <qtimer.h>
# include <qwidget.h>

#endif /* __cplusplus */

#undef max /*  These are defined in windef.h, but                   */
#undef min /*  we don't want them when building Qt                  */
