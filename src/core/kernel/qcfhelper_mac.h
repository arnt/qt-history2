/****************************************************************************
**
** Definition of QCFHelper class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef CFHELPER_H
#define CFHELPER_H

#include <CoreFoundation/CoreFoundation.h>
#ifndef QT_H
#include "qstring.h"
#include "qvarlengtharray.h"
#endif

template <typename T>
class Q_CORE_EXPORT QCFHelper
{
public:
    QCFHelper(const T &t = 0) : type(t) {}
    QCFHelper(const QCFHelper &helper) : type(helper.type) { if (type) CFRetain(type); }
    virtual ~QCFHelper() { if (type) CFRelease(type); }
    inline operator T() { return type; }
    inline QCFHelper operator =(const QCFHelper &helper)
    {
	CFTypeRef type2;
	if (helper.type)
	    CFRetain(helper.type);
	type2 = type;
	type = helper.type;
	if (type2)
	    CFRelease(type2);
	return *this;
    }
    inline T &dataRef() { return type; }
protected:
    T type;
};

class Q_CORE_EXPORT QCFStringHelper : public QCFHelper<CFStringRef>
{
public:
    QCFStringHelper(const QString &str) : QCFHelper<CFStringRef>(0), string(str) {}
    QCFStringHelper(const CFStringRef cfstr = 0) : QCFHelper<CFStringRef>(cfstr) {}
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
#endif
