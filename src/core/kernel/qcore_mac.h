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

/* We don't use the ApplicationEventLoop because it can causes bad behaviour in
   multithreaded applications. I've left the code in however because using the
   ApplicationEventLoop solved other problems (ages ago) - for example the gumdrop
   "hover" effects. */
//#define QMAC_USE_APPLICATION_EVENT_LOOP

#undef DEBUG
#ifdef OLD_DEBUG
#define DEBUG OLD_DEBUG
#endif
#undef OLD_DEBUG

#ifndef QT_H
#include "qstring.h"
#include "qvarlengtharray.h"
#endif

template <typename T>
class QCFHelper
{
public:
    inline QCFHelper(const T &t = 0) : type(t) {}
    inline QCFHelper(const QCFHelper &helper) : type(helper.type) { if (type) CFRetain(type); }
    inline ~QCFHelper() { if (type) CFRelease(type); }
    inline operator T() { return type; }
    inline QCFHelper operator =(const QCFHelper &helper)
    {
	if (helper.type)
	    CFRetain(helper.type);
	CFTypeRef type2 = type;
	type = helper.type;
	if (type2)
	    CFRelease(type2);
	return *this;
    }
    inline T &dataRef() { return type; }
    inline bool isNull() const { return type == 0; }
protected:
    T type;
};

class Q_CORE_EXPORT QCFStringHelper : public QCFHelper<CFStringRef>
{
public:
    inline QCFStringHelper(const QString &str) : QCFHelper<CFStringRef>(0), string(str) {}
    inline QCFStringHelper(const CFStringRef cfstr = 0) : QCFHelper<CFStringRef>(cfstr) {}
    inline operator QString()
    {
	if (string.isEmpty() && type) {
	    CFIndex length = CFStringGetLength(type);
	    const UniChar *chars = CFStringGetCharactersPtr(type);
	    if (chars) {
		string = QString(reinterpret_cast<const QChar *>(chars), length);
	    } else {
		QVarLengthArray<UniChar> buffer(length);
		CFStringGetCharacters(type, CFRangeMake(0, length), buffer);
		string = QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
	    }
	}
	return string;
    }
    inline operator CFStringRef() {
	if (!type)
	    type = CFStringCreateWithCharacters(0,
		    reinterpret_cast<const UniChar *>(string.unicode()), string.length());
	return static_cast<CFStringRef>(type);
    }
    inline void setString(const QString &str) {
        Q_ASSERT_X(!type, "QCFStringHelper", "QCFStringHelper already had a value and can't"
                   " be stomped on");
        string = str;
    }
private:
    QString string;
};

#endif /* __QKERNEL_MAC_H__ */
