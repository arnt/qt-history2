#ifndef __QKERNEL_MAC_H__
#define __QKERNEL_MAC_H__
/****************************************************************************
**
** Definition of various Qt/Mac specifics.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#undef OLD_DEBUG
#ifdef DEBUG
#define OLD_DEBUG DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#ifndef __IMAGECAPTURE__
#define __IMAGECAPTURE__
#endif
#ifdef qDebug
#    undef qDebug
#    include <Carbon/Carbon.h>
#    ifdef QT_NO_DEBUG
#        define qDebug qt_noop(),1?(void)0:qDebug
#    endif
#else
#    include <Carbon/Carbon.h>
#endif

#include <QuickTime/Movies.h>
#undef QT_BUILD_KEY

#undef DEBUG
#ifdef OLD_DEBUG
#define DEBUG OLD_DEBUG
#endif
#undef OLD_DEBUG

#ifndef QT_H
#include "qstring.h"
#endif

template <typename T>
class QCFType
{
public:
    inline QCFType(const T &t = 0) : type(t) {}
    inline QCFType(const QCFType &helper) : type(helper.type) { if (type) CFRetain(type); }
    inline ~QCFType() { if (type) CFRelease(type); }
    inline operator T() { return type; }
    inline QCFType operator =(const QCFType &helper)
    {
	if (helper.type)
	    CFRetain(helper.type);
	CFTypeRef type2 = type;
	type = helper.type;
	if (type2)
	    CFRelease(type2);
	return *this;
    }
    inline T *operator&() { return &type; }
protected:
    T type;
};

class Q_CORE_EXPORT QCFString : public QCFType<CFStringRef>
{
public:
    inline QCFString(const QString &str) : QCFType<CFStringRef>(0), string(str) {}
    inline QCFString(const CFStringRef cfstr = 0) : QCFType<CFStringRef>(cfstr) {}
    operator QString() const;
    operator CFStringRef() const;
    static QString cfstring2qstring(CFStringRef cfstr);
    static CFStringRef qstring2cfstring(const QString &str);
private:
    QString string;
};

#endif /* __QKERNEL_MAC_H__ */
