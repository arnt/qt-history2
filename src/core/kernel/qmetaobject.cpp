/****************************************************************************
**
** Implementation of QMetaObject class.
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

#include "qmetaobject.h"
#include "qobject.h"
#include <qcoreapplication.h>
#include <qstringlist.h>
#include <qcorevariant.h>
#include <qhash.h>
#include <ctype.h>

/*!
    \class QMetaObject qmetaobject.h

    \brief The QMetaObject class contains meta information about Qt
    objects.

    \ingroup objectmodel

    The Qt Meta Object System in Qt is responsible for the signals and
    slots inter-object communication mechanism, runtime type
    information, and the Qt property system. A single QMetaObject
    instance is created for each QObject subclass that is used in an
    application, and this instance stores all the meta information for
    the QObject subclass.

    This class is not normally required for application programming,
    but it is useful if you write meta applications, such as scripting
    engines or GUI builders.

    The functions you are most likely to find useful are these:
    \list
    \i className() returns the name of a class.
    \i superClass() returns the superclass's meta object.
    \i slotCount(), slot(), signalCount(), and signal() provide information
       about a class's signals and slots.
    \i enumeratorCount() and enumerator() provide information about
       a class's enumerators.
    \i propertyCount() and property() provide information about a
       class's properties.
    \endlist

    The index functions indexOfSlot(), indexOfSignal(),
    indexOfEnumerator(), and indexOfProperty(), map names of member
    functions, enumerators or properties to indices in the meta
    object. Qt uses e.g. indexOfSignal() and indexOfSlot() internally
    when you connect a signal to a slot.

    Classes can also have a list of name--value pairs of additonal
    \link QMetaClassInfo class information\endlink. The number of
    pairs is returned by classInfoCount(), single pairs are returned
    by classInfo(), and you can search for pairs with
    indexOfClassInfo().

    \sa \link moc.html moc (Meta QObject Compiler)\endlink

*/

/*!
    \enum QMetaMember::Access

    \internal
*/

/*!
    \fn QMetaEnum::operator bool() const

    Returns true if this member has a name; otherwise returns false.
*/

/*!
    \fn QMetaProperty::operator bool() const

    Returns true if this property is readable; otherwise returns false.
*/

enum ProperyFlags  {
    Invalid              = 0x00000000,
    Readable             = 0x00000001,
    Writable             = 0x00000002,
    Resetable            = 0x00000004,
    EnumOrFlag           = 0x00000008,
    StdCppSet            = 0x00000100,
    Override             = 0x00000200,
    Designable           = 0x00001000,
    ResolveDesignable    = 0x00002000,
    Scriptable           = 0x00004000,
    ResolveScriptable    = 0x00008000,
    Stored               = 0x00010000,
    ResolveStored        = 0x00020000,
    Editable             = 0x00040000,
    ResolveEditable      = 0x00080000
};

enum FunctionFlags  {
    AccessPrivate         = 0x01,
    AccessPublic          = 0x02,
    AccessProtected       = 0x04,
    AccessMask            = 0x07, //mask
    Compatability         = 0x08,
    Cloned                = 0x10
};

struct QMetaObjectPrivate
{
    int revision;
    int className;
    int classInfoCount, classInfoData;
    int signalCount, signalData;
    int slotCount, slotData;
    int propertyCount, propertyData;
    int enumeratorCount, enumeratorData;
};

static inline const QMetaObjectPrivate *priv(const uint* data)
{ return reinterpret_cast<const QMetaObjectPrivate*>(data); }


/*! \fn const char *QMetaObject::className() const

    Returns the class name.

    \sa QObject::className(), superClass()
*/

/*!
    \fn QMetaObject *QMetaObject::superClass() const

    Returns the meta object of the super class or 0 if there is no
    such object.
*/

/*!
    \internal
    Returns \a obj if object \a obj inherits from this meta
    object; otherwise returns 0.
*/
QObject *QMetaObject::cast(const QObject *obj) const
{
    if (obj) {
        const QMetaObject *m = obj->metaObject();
        do {
            if (m == this)
                return const_cast<QObject*>(obj);
        } while ((m = m->d.superdata));
    }
    return 0;
}


#ifndef QT_NO_TRANSLATION
/*!
    \internal
    Forwards a tr() call from the Q_OBJECT macro to QApplication
*/
QString QMetaObject::tr(const char *s, const char *c) const
{
    if (QCoreApplication::instance())
        return QCoreApplication::instance()->translate(d.stringdata, s, c, QCoreApplication::DefaultCodec);
    else
        return QString::fromLatin1(s);
}
/*!
    \internal
    Forwards a trUtf8() call from the Q_OBJECT macro to QApplication
*/
QString QMetaObject::trUtf8(const char *s, const char *c) const
{
    if (QCoreApplication::instance())
        return QCoreApplication::instance()->translate(d.stringdata, s, c, QCoreApplication::UnicodeUTF8);
    else
        return QString::fromUtf8(s);
}
#endif // QT_NO_TRANSLATION


/*!
    Returns the slot offset for this class, i.e. the index position of
    this class's first slot. The offset is the sum of all the slots in
    the class's super classes (which is always positive since QObject
    has the deleteLater() slot).
 */
int QMetaObject::slotOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->slotCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the signal offset for this class, i.e. the index position
    of this class's first signal. The offset is the sum of all the
    signals in the class's super classes (which is always positive
    since QObject has slots destroyed() and destroyed(QObject*)).
 */
int QMetaObject::signalOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->signalCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the enumerator offset for this class, i.e. the index
    position of this class's first enumerator. If the class has no
    super classes with enumerators, the offset is 0, otherwise the
    offset is the sum of all the enumerators in the class's super
    classes.
 */
int QMetaObject::enumeratorOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->enumeratorCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the property offset for this class, i.e. the index
    position of this class's first property. The offset is the sum of
    all the properties in the class's super classes (which is always
    positive since QObject has the name() property).
 */
int QMetaObject::propertyOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->propertyCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the class information offset for this class, i.e. the
    index position of this class's first class information item. If
    the class has no super classes with class information, the offset
    is 0, otherwise the offset is the sum of all the class information
    items in the class's super classes.
 */
int QMetaObject::classInfoOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->classInfoCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the number of slots in this class.

    \sa slot()
*/
int QMetaObject::slotCount() const
{
    int n = priv(d.data)->slotCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->slotCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of signals in this class.

    \sa signal()
*/
int QMetaObject::signalCount() const
{
    int n = priv(d.data)->signalCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->signalCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of enumerators in this class.

    \sa enumerator()
*/
int QMetaObject::enumeratorCount() const
{
    int n = priv(d.data)->enumeratorCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->enumeratorCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of properties in this class.

    \sa property()
*/
int QMetaObject::propertyCount() const
{
    int n = priv(d.data)->propertyCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->propertyCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of items of class information in this class.
*/
int QMetaObject::classInfoCount() const
{
    int n = priv(d.data)->classInfoCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->classInfoCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Finds \a slot and returns its index; otherwise returns -1.

    \sa slot(), slotCount()
*/
int QMetaObject::indexOfSlot(const char *slot) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->slotCount-1; i >= 0; --i)
            if (strcmp(slot, m->d.stringdata
                       + m->d.data[priv(m->d.data)->slotData + 5*i]) == 0) {
                i += m->slotOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}

/*!

    Finds \a signal and returns its index; otherwise returns -1.

    \sa signal(), signalCount()
*/
int QMetaObject::indexOfSignal(const char *signal) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->signalCount-1; i >= 0; --i)
            if (strcmp(signal, m->d.stringdata
                       + m->d.data[priv(m->d.data)->signalData + 5*i]) == 0) {
                i += m->signalOffset();
                break;
            }
        m = m->d.superdata;
    }
#ifndef QT_NO_DEBUG
    if (i >= 0 && m->d.superdata) {
        int conflict = m->d.superdata->indexOfSignal(signal);
        if (conflict >= 0)
            qWarning("QMetaObject::indexOfSignal:%s: Conflict with %s::%s",
                      m->d.stringdata, m->d.superdata->d.stringdata, signal);
    }
#endif
    return i;
}


/*!
    Finds enumerator \a name and returns its index; otherwise returns
    -1.

    \sa enumerator(), enumeratorCount()
*/
int QMetaObject::indexOfEnumerator(const char *name) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->enumeratorCount-1; i >= 0; --i) {
            if (strcmp(name, m->d.stringdata
                       + m->d.data[priv(m->d.data)->enumeratorData + 4*i]) == 0) {
                i += m->enumeratorOffset();
                break;
            }
        }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Finds property \a name and returns its index; otherwise returns
    -1.

    \sa property(), propertyCount()
*/
int QMetaObject::indexOfProperty(const char *name) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->propertyCount-1; i >= 0; --i)
            if (strcmp(name, m->d.stringdata
                       + m->d.data[priv(m->d.data)->propertyData + 3*i]) == 0) {
                i += m->propertyOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Finds class information item \a name and returns its index;
    otherwise returns -1.

    \sa classInfo(), classInfoCount()
*/
int QMetaObject::indexOfClassInfo(const char *name) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->classInfoCount-1; i >= 0; --i)
            if (strcmp(name, m->d.stringdata
                       + m->d.data[priv(d.data)->classInfoData + 2*i]) == 0) {
                i += m->classInfoOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Returns the meta data for the slot with index \a index.
*/
QMetaMember QMetaObject::slot(int index) const
{
    int i = index;
    i -= slotOffset();
    if (i < 0 && d.superdata)
        return d.superdata->slot(index);

    QMetaMember result;
    if (i >= 0 && i <= priv(d.data)->slotCount) {
        result.mobj = this;
        result.handle = priv(d.data)->slotData + 5*i;
    }
    return result;
}

/*!
    Returns the meta data for the signal with index \a index.
*/
QMetaMember QMetaObject::signal(int index) const
{
    int i = index;
    i -= signalOffset();
    if (i < 0 && d.superdata)
        return d.superdata->signal(index);

    QMetaMember result;
    if (i >= 0 && i <= priv(d.data)->signalCount) {
        result.mobj = this;
        result.handle = priv(d.data)->signalData + 5*i;
    }
    return result;
}


/*!
    Returns the meta data for the enumerator with index \a index.
*/
QMetaEnum QMetaObject::enumerator(int index) const
{
    int i = index;
    i -= enumeratorOffset();
    if (i < 0 && d.superdata)
        return d.superdata->enumerator(index);

    QMetaEnum result;
    if (i >= 0 && i <= priv(d.data)->enumeratorCount) {
        result.mobj = this;
        result.handle = priv(d.data)->enumeratorData + 4*i;
    }
    return result;
}

/*!
    Returns the meta data for the property with index \a index.
*/
QMetaProperty QMetaObject::property(int index) const
{
    int i = index;
    i -= propertyOffset();
    if (i < 0 && d.superdata)
        return d.superdata->property(index);

    QMetaProperty result;
    if (i >= 0 && i <= priv(d.data)->propertyCount) {
        int handle = priv(d.data)->propertyData + 3*i;
        int flags = d.data[handle + 2];
        const char *name = d.stringdata + d.data[handle];
        const char *type = d.stringdata + d.data[handle + 1];
        if ((flags & Override) && d.superdata){
            result = property(d.superdata->indexOfProperty(name));
            if (qstrcmp(result.type(), type)) // type missmatch, no override
                ::memset(&result, 0, sizeof(QMetaProperty));
        }
        if (flags & EnumOrFlag) {
            result.menum = enumerator(indexOfEnumerator(type));
        }
        if (flags & Readable) {
            result.mobj[ReadProperty] = this;
            result.idx[ReadProperty] = i;
        }
        if (flags & Writable) {
            result.mobj[WriteProperty] = this;
            result.idx[WriteProperty] = i;
        }
        if (flags & Resetable) {
            result.mobj[ResetProperty] = this;
            result.idx[ResetProperty] = i;
        }
        if ((flags & ResolveDesignable) == 0) {
            result.mobj[QueryPropertyDesignable] = this;
            result.idx[QueryPropertyDesignable] = i;
        }
        if ((flags & ResolveScriptable) == 0) {
            result.mobj[QueryPropertyScriptable] = this;
            result.idx[QueryPropertyScriptable] = i;
        }
        if ((flags & ResolveStored) == 0) {
            result.mobj[QueryPropertyStored] = this;
            result.idx[QueryPropertyStored] = i;
        }
        if ((flags & ResolveEditable) == 0) {
            result.mobj[QueryPropertyEditable] = this;
            result.idx[QueryPropertyEditable] = i;
        }
    }
    return result;
}

/*!
    Returns the meta data for the item of class information with index
    \a index.
 */
QMetaClassInfo QMetaObject::classInfo(int index) const
{
    int i = index;
    i -= classInfoOffset();
    if (i < 0 && d.superdata)
        return d.superdata->classInfo(index);

    QMetaClassInfo result;
    if (i >= 0 && i <= priv(d.data)->classInfoCount) {
        result.mobj = this;
        result.handle = priv(d.data)->classInfoData + 2*i;
    }
    return result;
}

/*!
    Returns true if the \a signal and the \a member arguments are
    compatible; otherwise returns false.

    Both \a signal and \a member are expected to be normalized.

    \sa normalizedSignature()
*/
bool QMetaObject::checkConnectArgs(const char *signal, const char *member)
{
    const char *s1 = signal;
    const char *s2 = member;
    while (*s1++ != '(') { }                        // scan to first '('
    while (*s2++ != '(') { }
    if (*s2 == ')' || qstrcmp(s1,s2) == 0)        // member has no args or
        return true;                                //   exact match
    int s1len = strlen(s1);
    int s2len = strlen(s2);
    if (s2len < s1len && strncmp(s1,s2,s2len-1)==0 && s1[s2len-1]==',')
        return true;                                // member has less args
    return false;
}

static inline bool isIdentChar(char x)
{                                                // Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
         (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace(char x)
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.  isspace() usually
    works, but not here.  This implementation is sufficient for our
    internal use.
  */
    return (uchar) x <= 32;
#else
    return isspace((uchar) x);
#endif
}

static QByteArray normalizeTypeInternal(const char *t, const char *e, bool adjustConst = true){
    int len = e - t;
    if (strncmp("void", t, len) == 0)
        return "";
    /*
      Convert 'char const *' into 'const char *'. Start at index 1,
      not 0, because 'const char *' is already OK.
    */
    QByteArray constbuf;
    for (int i = 1; i < len; i++) {
        if (t[i] == 'c' &&
             strncmp(t + i + 1, "onst", 4) == 0) {
            constbuf = QByteArray(t, len);
            if (isSpace(t[i-1]))
                constbuf.remove(i-1, 6);
            else
                constbuf.remove(i, 5);
            constbuf.prepend("const ");
            t = constbuf.data();
            e = constbuf.data() + constbuf.length();
            break;
        }
        /*
          We musn't convert 'char * const *' into 'const char **'
          and we must beware of 'Bar<const Bla>'.
        */
        if (t[i] == '&' || t[i] == '*' ||t[i] == '<')
            break;
    }
    if (adjustConst && e > t + 6 && strncmp("const ", t, 6) == 0) {
        if (*(e-1) == '&') { // treat const reference as value
            t += 6;
            --e;
        } else if (isIdentChar(*(e-1))) { // treat const value as value
            t += 6;
        }
    }
    QByteArray result;
    result.reserve(len);

    // some type substitutions for 'unsigned x'
    if (strncmp("unsigned ", t, 9) == 0) {
        if (strncmp("int", t+9, 3) == 0) {
            t += 9+3;
            result += "uint";
        } else if (strncmp("long", t+9, 4) == 0) {
            t += 9+4;
            result += "ulong";
        }
    }
    while (t != e) {
        char c = *t++;
        result += c;
        if (c == '<') {
            //template recursion
            const char* tt = t;
            int templdepth = 1;
            while (t != e) {
                c = *t++;
                if (c == '<')
                    ++templdepth;
                if (c == '>')
                    --templdepth;
                if (templdepth == 0) {
                    result += normalizeTypeInternal(tt, t-1, false);
                    result += c;
                    if (*t == '>')
                        result += ' '; // avoid >>
                    break;
                }
            }
        }
    }
    return result;
}


/*!
    Normalizes the signature of \a member.

    Qt uses normalized signatures to decide whether two given signals
    and slots are compatible. Normalization reduces whitespace to a
    minimum, moves 'const' to the front where appropriate, removes
    'const' from value types and replaces const references with
    values.

    \sa checkConnectArgs()
 */
QByteArray QMetaObject::normalizedSignature(const char *member)
{
    const char *s = member;
    if (!s || !*s)
        return "";
    int len = strlen(s);
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len+1] : stackbuf);
    char *d = buf;
    char last = 0;
    while(*s && isSpace(*s))
        s++;
    while (*s) {
        while (*s && !isSpace(*s))
            last = *d++ = *s++;
        while (*s && isSpace(*s))
            s++;
        if (*s && isIdentChar(*s) && isIdentChar(last))
            last = *d++ = ' ';
    }
    *d = '\0';
    d = buf;

    QByteArray result;
    result.reserve(len);

    int argdepth = 0;
    int templdepth = 0;
    while (*d) {
        if (argdepth == 1) {
            const char *t = d;
            while (*d&& (templdepth
                           || (*d != ',' && *d != ')'))) {
                if (*d == '<')
                    ++templdepth;
                if (*d == '>')
                    --templdepth;
                d++;
            }
            result += normalizeTypeInternal(t, d);
        }
        if (*d == '(')
            ++argdepth;
        if (*d == ')')
            --argdepth;
        result += *d++;
    }

    if (buf != stackbuf)
        delete [] buf;
    return result;
}


/*!
    \class QMetaMember qmetaobject.h

    \brief The QMetaMember class provides meta data about a signal or
    slot member function.

    \ingroup objectmodel

    A QMetaMember has a signature(), a list of parameters(), a
    return type(), a tag(), and an access() specifier.
*/


/*!
    \fn QMetaMember::QMetaMember()
    \internal
*/

/*!
    Returns the signature of this member.
*/
const char *QMetaMember::signature() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle];
}

/*!
    Returns a comma separated list of parameter names.
*/
const char *QMetaMember::parameters() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 1];
}

/*!
    Returns the return type of this member, or an empty string if the
    return type is \e void.
*/
const char *QMetaMember::type() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 2];
}

/*!
    Returns the tag associated with this member.
*/

const char *QMetaMember::tag() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 3];
}


/*! \internal */

bool QMetaMember::isCompat() const
{
    if (!mobj)
        return false;
    return mobj->d.data[handle + 4] & Compatability;
}

/*! \internal */

bool QMetaMember::isCloned() const
{
    if (!mobj)
        return false;
    return mobj->d.data[handle + 4] & Cloned;
}

/*!
    Returns the access specification of this member: private,
    protected, or public. Signals are always protected.
*/

QMetaMember::Access QMetaMember::access() const
{
    if (!mobj)
        return Private;
    switch(mobj->d.data[handle + 4] & AccessMask) {
    case AccessPublic:
        return Public;
    case AccessPrivate:
        return Private;
    case AccessProtected:
        return Protected;
    default:
        break;
    }
    return Private;
}

/*!
    \class QMetaEnum qmetaobject.h

    \brief The QMetaEnum class provides meta data about an enumerator.

    \ingroup objectmodel

    Use name() for the enumerator's name. The enumerator's keys (names
    of each enumerated item) are returned by key(); the number of keys
    is given by numKeys(). isFlag() returns whether the enumerator is
    meant to be used as a flag, meaning that its values can be OR'ed
    together.

    The conversion functions keyToValue(), valueToKey(), keysToValue()
    and valueToKeys() allow conversion between the integer
    representation of an enumeration or set value and its literal
    representation.
*/

/*!
    \fn QMetaEnum::QMetaEnum()
    \internal
*/

/*!
    Returns the name of the enumerator.
*/
const char* QMetaEnum::name() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle];
}

/*!
    Returns the number of keys.

    \sa key()
*/
int QMetaEnum::numKeys() const
{
    if (!mobj)
        return 0;
    return mobj->d.data[handle + 2];
}


/*!
    Returns the key with index \a index; or returns 0 if there is no
    such key.

    \sa numKeys() value()
*/
const char *QMetaEnum::key(int index) const
{
    if (!mobj)
        return 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    if (index >= 0  && index < count)
        return mobj->d.stringdata + mobj->d.data[data + 2*index];
    return 0;
}

/*!
    Returns the value with index \a index; or returns -1 if there is no
    such value.

    \sa numKeys() key()
*/
int QMetaEnum::value(int index) const
{
    if (!mobj)
        return 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    if (index >= 0  && index < count)
        return mobj->d.data[data + 2*index + 1];
    return -1;
}


/*!
    Returns true if this enumerator is is used as a flag, i.e. the
    enumeration values can be OR'ed together; otherwise returns false.

    \sa keysToValue(), valueToKeys()
*/
bool QMetaEnum::isFlag() const
{
    return mobj && mobj->d.data[handle + 1];
}

/*!
    Returns the integer value of the enumeration key \a key, or -1 if
    \a key isn't found.

    For set types, use keysToValue().

    \sa valueToKey(), isFlag(), keysToValue()
*/
int QMetaEnum::keyToValue(const char *key) const
{
    if (!mobj || !key)
        return -1;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int i = 0; i < count; ++i)
        if (!strcmp(key, mobj->d.stringdata + mobj->d.data[data + 2*i]))
            return mobj->d.data[data + 2*i + 1];
    return -1;
}

/*!
    Returns the key string for the enumeration value \a value, or 0 if
    \a value isn't found.

    For set types, use valueToKeys().

    \sa valueToKey() isFlag() valueToKeys()
*/
const char* QMetaEnum::valueToKey(int value) const
{
    if (!mobj)
        return 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int i = 0; i < count; ++i)
        if (value == (int)mobj->d.data[data + 2*i + 1])
            return mobj->d.stringdata + mobj->d.data[data + 2*i];
    return 0;
}

/*!
    Returns the value derived from OR'ing together the values of the
    keys given in \a keys. Note that the key strings in \a keys must
    be '|'-separated.

    \sa isFlag(), valueToKey(), keysToValue()
*/
int QMetaEnum::keysToValue(const char *keys) const
{
    if (!mobj)
        return -1;
    QStringList l = QString::fromLatin1(keys).split('|');
    //#### TODO write proper code, do not use QStringList
    int value = 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int li = 0; li < (int)l.size(); ++li) {
        QString s = l[li].trimmed();
        int i;
        for (i = count-1; i >= 0; --i)
            if (!strcmp(s.latin1(),
                        mobj->d.stringdata + mobj->d.data[data + 2*i])) {
                value |= mobj->d.data[data + 2*i + 1];
                break;
            }
        if (i < 0)
            value |= -1;
    }
    return value;
}

/*!
    Returns a byte array of '|'-separated keys that represents the
    given \a value.

    \sa isFlag(), valueToKey(), valueToKeys()
*/
QByteArray QMetaEnum::valueToKeys(int value) const
{
    QByteArray keys;
    if (!mobj)
        return keys;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    int v = value;
    for(int i = count - 1; i >= 0; --i) {
        int k = mobj->d.data[data + 2*i + 1];
        if ((k != 0 && (v & k) == k ) ||  (k == value))  {
            v = v & ~k;
            if (!keys.isEmpty())
                keys += '|';
            keys += mobj->d.stringdata + mobj->d.data[data + 2*i];
        }
    }
    return keys;
}


/*!
    \class QMetaProperty qmetaobject.h

    \brief The QMetaProperty class provides meta data about a property.

    \ingroup objectmodel

    A property has a name() and a type(), as well as various
    attributes that specify whether it isReadable(), isWritable(),
    isDesignable(), isScriptable(), isStored(), or isEditable().

    If the property is an enumeration isEnumType() returns true; and
    if the property is an enumeration that is a flag (i.e. its values
    can be OR'ed together), isEnumType() and isFlagType() both return
    true. The enumerator for these types is available from
    enumerator().

    The property's values are set and retrieved with read(), write(),
    and reset(); or through QObject's set and get functions. See
    QObject::setProperty() and QObject::property() for details.

    You get meta property data through an object's meta object. See
    QMetaObject::property() and QMetaObject::propertyCount() for
    details.
*/

/*!
    Constructs an invalid property
    \internal
*/
QMetaProperty::QMetaProperty()
{
    ::memset(this, 0, sizeof(QMetaProperty));
}


/*!
    Returns this property's name.
 */
const char *QMetaProperty::name() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    return mobj[QMetaObject::ReadProperty]->d.stringdata + mobj[QMetaObject::ReadProperty]->d.data[handle];
}

/*!
    Returns this property's type.
 */
const char *QMetaProperty::type() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    return mobj[QMetaObject::ReadProperty]->d.stringdata + mobj[QMetaObject::ReadProperty]->d.data[handle + 1];
}


/*!
    Returns true if the property's type is an enumeration value that
    is used as a flag, i.e. if the enumeration values can be OR'ed
    together; otherwise returns false. A set type is implicitly also
    an enum type.

    \sa isEnumType(), enumerator()
*/

bool QMetaProperty::isFlagType() const
{
    return isEnumType() && menum.isFlag();
}

/*!
    Returns true if the property's type is an enumeration value;
    otherwise returns false.

    \sa enumerator(), isFlagType()
*/
bool QMetaProperty::isEnumType() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
    return (flags & EnumOrFlag) && menum.name();
}

/*!
    \internal

    Returns true if the property has a C++ setter function that
    follows Qt's standard "name" / "setName" pattern. Designer and uic
    query hasStdCppSet() in order to avoid expensive
    QObject::setProperty() calls. All properties in Qt [should] follow
    this pattern.
*/
bool QMetaProperty::hasStdCppSet() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
    return (flags & StdCppSet);
}

/*!
    Returns the enumerator if this property's type is an enumerator
    type; otherwise returns something undefined.

    \sa isEnumType()
 */
QMetaEnum QMetaProperty::enumerator() const
{
    return menum;
}


/*!
    Reads the property's value from object \a obj. Returns the value
    if it was able to read it; otherwise returns an invalid variant.
*/
QCoreVariant QMetaProperty::read(const QObject *obj) const
{
    if (!obj || !mobj[QMetaObject::ReadProperty])
        return QCoreVariant();

    QCoreVariant::Type t = QCoreVariant::Int;
    if (!isEnumType()) {
        int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
        int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
        t = (QCoreVariant::Type)(flags >> 24);
        if (t == QCoreVariant::Invalid)
            t = QCoreVariant::nameToType(mobj[QMetaObject::ReadProperty]->d.stringdata
                                      + mobj[QMetaObject::ReadProperty]->d.data[handle + 1]);
        if (t == QCoreVariant::Invalid)
            return QCoreVariant();
    }
    QCoreVariant value(t, (void*)0);
    void *argv[] = { value.data() };
    const_cast<QObject*>(obj)->qt_metacall(QMetaObject::ReadProperty,
                     idx[QMetaObject::ReadProperty] + mobj[QMetaObject::ReadProperty]->propertyOffset(),
                     argv);
    if (argv[0] != value.data())
        return QCoreVariant(t, argv[0]);
    return value;
}

/*!
    Writes \a value as the property's value to object \a obj. Returns
    true if the write succeeded; otherwise returns false.
*/
bool QMetaProperty::write(QObject *obj, const QCoreVariant &value) const
{
    if (!obj || !isWritable())
        return false;

    QCoreVariant v = value;
    if (isEnumType()) {
        if (v.type() == QCoreVariant::String || v.type() == QCoreVariant::CString) {
            if (isFlagType())
                v = QCoreVariant(menum.keysToValue(value.toByteArray()));
            else
                v = QCoreVariant(menum.keyToValue(value.toByteArray()));
        } else if (v.type() != QCoreVariant::Int && v.type() != QCoreVariant::UInt) {
            return false;
        }
        v.cast(QCoreVariant::Int);
    } else {
        int handle = priv(mobj[QMetaObject::WriteProperty]->d.data)->propertyData + 3*idx[QMetaObject::WriteProperty];
        int flags = mobj[QMetaObject::WriteProperty]->d.data[handle + 2];
        QCoreVariant::Type t = (QCoreVariant::Type)(flags >> 24);
        if (t == QCoreVariant::Invalid)
            t = QCoreVariant::nameToType(mobj[QMetaObject::WriteProperty]->d.stringdata
                                      + mobj[QMetaObject::WriteProperty]->d.data[handle + 1]);
        if (t != QCoreVariant::Invalid && !v.cast(t))
            return false;
    }

    void *argv[] = { v.data() };
    obj->qt_metacall(QMetaObject::WriteProperty,
                     idx[QMetaObject::WriteProperty] + mobj[QMetaObject::WriteProperty]->propertyOffset(),
                     argv);
    return true;
}

/*!
    Resets the property for object \a obj with a reset method.
    Returns true if the reset worked; otherwise returns false.

    Reset methods are optional, with only a few properties supporting
    them.
*/
bool QMetaProperty::reset(QObject *obj) const
{
    if (!obj || !mobj[QMetaObject::ResetProperty])
        return false;
    void *argv[] = { 0 };
    obj->qt_metacall(QMetaObject::ResetProperty,
                     idx[QMetaObject::ResetProperty] + mobj[QMetaObject::ResetProperty]->propertyOffset(),
                     argv);
    return true;
}


/*!
    Returns true if this property is readable; otherwise returns false.
 */
bool QMetaProperty::isReadable() const
{
    return mobj[QMetaObject::ReadProperty] != 0;
}

/*!
    Returns true if this property is writable; otherwise returns
    false.
 */
bool QMetaProperty::isWritable() const
{
    if (!mobj[QMetaObject::WriteProperty])
        return false;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
    return !(flags & EnumOrFlag) || menum.name();
}


static bool qt_query_property(const QMetaObject*const*mobj,const int *idx, uint flag,
                              QMetaObject::Call call, const QObject* obj)
{
    if (!mobj[call])
        return false;
    int handle = priv(mobj[call]->d.data)->propertyData + 3*idx[call];
    int flags = mobj[call]->d.data[handle + 2];
    bool b = (flags & flag);
    if (obj) {
        void *argv[] = { &b };
        const_cast<QObject*>(obj)->qt_metacall(call,
                                               idx[call]
                                               + mobj[call]->propertyOffset(),
                                               argv);
    }
    return b;
}

/*!
    Returns true if this property is designable for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns false if the
    Q_PROPERTY's DESIGNABLE attribute is false; otherwise, (i.e. if
    the attribute is true or is a function or expression), returns
    true.
 */
bool QMetaProperty::isDesignable(const QObject *obj) const
{
    if (!mobj[QMetaObject::WriteProperty])
        return false;
    return qt_query_property(mobj, idx, Designable,
                              QMetaObject::QueryPropertyDesignable,
                              obj);
}

/*!
    Returns true if the property is scriptable for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns false if the
    Q_PROPERTY's DESIGNABLE attribute is false; otherwise, (i.e. if
    the attribute is true or is a function or expression), returns
    true.
 */
bool QMetaProperty::isScriptable(const QObject *obj) const
{
    return qt_query_property(mobj, idx, Scriptable,
                              QMetaObject::QueryPropertyScriptable,
                              obj);
}

/*!
    Returns true if the property is stored for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns false if the
    Q_PROPERTY's DESIGNABLE attribute is false; otherwise, (i.e. if
    the attribute is true or is a function or expression), returns
    true.
 */
bool QMetaProperty::isStored(const QObject *obj) const
{
    return qt_query_property(mobj, idx, Stored,
                              QMetaObject::QueryPropertyStored,
                              obj);
}

/*!
    Returns true if the property is editable for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns false if the
    Q_PROPERTY's DESIGNABLE attribute is false; otherwise, (i.e. if
    the attribute is true or is a function or expression), returns
    true.
 */
bool QMetaProperty::isEditable(const QObject *obj) const
{
    return qt_query_property(mobj, idx, Editable,
                              QMetaObject::QueryPropertyEditable,
                              obj);
}


/*!
    \class QMetaClassInfo qmetaobject.h

    \brief The QMetaClassInfo class provides additional information
    about a class.

    \ingroup objectmodel

    Class information items are simple \e{name}--\e{value} pairs that
    are specified using \c Q_CLASSINFO in the source code. The
    information can be retrieved using name() and value().
*/


/*!
    \fn QMetaClassInfo::QMetaClassInfo()
    \internal
*/

/*!
    Returns the name of this item.

    \sa value()
*/
const char* QMetaClassInfo::name() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle];
}

/*!
    Returns the value of this item.

    \sa name()
*/
const char* QMetaClassInfo::value() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 1];
}


/*!
    \class QMetaType qmetaobject.h
    \brief The QMetaType class manages named types in the meta object system.

    \internal


    The class is used to queue signals and slots connections.
*/


static const struct { const char * typeName; int type; }types[]  = {
    {"void*", QMetaType::VoidStar},
    {"long", QMetaType::Long},
    {"int", QMetaType::Int},
    {"short", QMetaType::Short},
    {"char", QMetaType::Char},
    {"ulong", QMetaType::ULong},
    {"unsigned long", QMetaType::ULong},
    {"uint", QMetaType::UInt},
    {"unsigned int", QMetaType::UInt},
    {"ushort", QMetaType::UShort},
    {"unsigned short", QMetaType::UShort},
    {"uchar", QMetaType::UChar},
    {"unsigned char", QMetaType::UChar},
    {"bool", QMetaType::Bool},
    {"float", QMetaType::Float},
    {"double", QMetaType::Double},
    {"QChar", QMetaType::QChar},
    {"QByteArray", QMetaType::QByteArray},
    {"QString", QMetaType::QString},
    {"void", QMetaType::Void},
    {"", QMetaType::Void},
    {0, QMetaType::Void}
};

class QCustomTypeInfo
{
public:
    QCustomTypeInfo() : typeName(0), copy(0), destr(0) {}
    ~QCustomTypeInfo() { delete typeName; }
    void setData(const char *tname, QMetaType::CopyConstructor cp, QMetaType::Destructor de)
    { delete typeName; typeName = qstrdup(tname); copy = cp; destr = de; }
    void setData(QMetaType::CopyConstructor cp, QMetaType::Destructor de)
    { copy = cp; destr = de; }

    const char *typeName;
    QMetaType::CopyConstructor copy;
    QMetaType::Destructor destr;
};

static QVector<QCustomTypeInfo> customTypes;

/*
   returns the type name associated for \a type or 0 if no type was
   found. The returned pointer must not be deleted.
 */
const char *QMetaType::typeName(int type)
{
    if (type >= User)
        return isRegistered(type) ? customTypes.at(type - User).typeName : 0;

    int i = 0;
    while (types[i].typeName) {
        if (types[i].type == type)
            return types[i].typeName;
        ++i;
    }
    return 0;
}

/*!
  Registers a user type for marshalling, with \a typeName, a \a
  destructor, and a \a copyConstructor. Returns the type's handle, or
  -1 if the type could not be registered.
 */
int QMetaType::registerType(const char *typeName, Destructor destructor,
                            CopyConstructor copyConstructor)
{
    static int currentIdx = User;
    if (!typeName || !destructor || !copyConstructor)
        return -1;

    customTypes.ensure_constructed();
    int idx = type(typeName);
    if (idx) {
        if (idx < User) {
            qWarning("cannot re-register basic type '%s'", typeName);
            return -1;
        }
        customTypes[idx - User].setData(copyConstructor, destructor);
    } else {
        idx = currentIdx++;
        customTypes.resize(customTypes.count() + 1);
        customTypes[idx - User].setData(typeName, copyConstructor, destructor);
    }
    return idx;
}

/*!
  Returns whether a custom datatype with id \a type is registred or not.
 */
bool QMetaType::isRegistered(int type)
{
    return (type >= User) && (customTypes.count() > type - User);
}

/*!
  Returns a handle to the type with name \a typeName, or 0 if there is
  no such type.
 */
int QMetaType::type(const char *typeName)
{
    if (!typeName)
        return 0;
    int i = 0;
    while (types[i].typeName && strcmp(typeName, types[i].typeName))
        ++i;
    if (!types[i].type) {
        customTypes.ensure_constructed();
        for (int v = 0; v < customTypes.count(); ++v) {
            if (strcmp(customTypes.at(v).typeName, typeName) == 0)
                return v + User;
        }
    }
    return types[i].type;
}

/*
  Returns a copy of data, assuming it is of type \a type.
 */
void *QMetaType::copy(int type, const void *data)
{
    if (!data)
        return 0;
    switch(type) {
    case QMetaType::VoidStar:
        return new void *(*static_cast<void* const *>(data));
    case QMetaType::Long:
        return new long(*static_cast<const long*>(data));
    case QMetaType::Int:
        return new int(*static_cast<const int*>(data));
    case QMetaType::Short:
        return new short(*static_cast<const short*>(data));
    case QMetaType::Char:
        return new char(*static_cast<const char*>(data));
    case QMetaType::ULong:
        return new ulong(*static_cast<const ulong*>(data));
    case QMetaType::UInt:
        return new uint(*static_cast<const uint*>(data));
    case QMetaType::UShort:
        return new ushort(*static_cast<const ushort*>(data));
    case QMetaType::UChar:
        return new uchar(*static_cast<const uchar*>(data));
    case QMetaType::Bool:
        return new bool(*static_cast<const bool*>(data));
    case QMetaType::Float:
        return new float(*static_cast<const float*>(data));
    case QMetaType::Double:
        return new double(*static_cast<const double*>(data));
    case QMetaType::QChar:
        return new ::QChar(*static_cast<const ::QChar*>(data));
    case QMetaType::QByteArray:
        return new ::QByteArray(*static_cast<const ::QByteArray*>(data));
    case QMetaType::QString:
        return new ::QString(*static_cast<const ::QString*>(data));
    case QMetaType::Void:
        return 0;
    default:
        customTypes.ensure_constructed();
        if (type >= User && customTypes.count() > type - User)
            return customTypes.at(type - User).copy(data);
        return 0;
    }
}

/*!
  Destroys the \a data, assuming it is of type \a type.
 */
void QMetaType::destroy(int type, void *data)
{
    if (!data)
        return;
    switch(type) {
    case QMetaType::VoidStar:
        delete static_cast<void**>(data);
        break;
    case QMetaType::Long:
        delete static_cast<long*>(data);
        break;
    case QMetaType::Int:
        delete static_cast<int*>(data);
        break;
    case QMetaType::Short:
        delete static_cast<short*>(data);
        break;
    case QMetaType::Char:
        delete static_cast<char*>(data);
        break;
    case QMetaType::ULong:
        delete static_cast<ulong*>(data);
        break;
    case QMetaType::UInt:
        delete static_cast<uint*>(data);
        break;
    case QMetaType::UShort:
        delete static_cast<ushort*>(data);
        break;
    case QMetaType::UChar:
        delete static_cast<uchar*>(data);
        break;
    case QMetaType::Bool:
        delete static_cast<bool*>(data);
        break;
    case QMetaType::Float:
        delete static_cast<float*>(data);
        break;
    case QMetaType::Double:
        delete static_cast<double*>(data);
        break;
    case QMetaType::QChar:
        delete static_cast< ::QChar*>(data);
        break;
    case QMetaType::QByteArray:
        delete static_cast< ::QByteArray*>(data);
        break;
    case QMetaType::QString:
        delete static_cast< ::QString*>(data);
        break;
    case QMetaType::Void:
        break;
    default:
        customTypes.ensure_constructed();
        if (type >= User && customTypes.count() > type - User)
            customTypes.at(type - User).destr(data);
        break;
    }
}
