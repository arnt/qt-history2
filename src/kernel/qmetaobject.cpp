/****************************************************************************
** $Id$
**
** Implementation of QMetaObject class
**
** Created : 930419
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmetaobject.h"
#include "qobject.h"
#include <qapplication.h>
#include <qvariant.h>
#include <ctype.h>

/*!
    \class QMetaObject qmetaobject.h

    \brief The QMetaObject class contains meta information about Qt
    objects.

    \ingroup objectmodel

    The QMeta Object System in Qt is responsible for the signals and
    slots inter-object communication mechanism, runtime type
    information and the property system. All meta information in Qt is
    kept in a single instance of QMetaObject per class.

    This class is not normally required for application programming.
    But if you write meta applications, such as scripting engines or
    GUI builders, you might find these functions useful:
    \list
    \i className() to get the name of a class.
    \i superClass() to access the superclass's meta object.
    \i numSlots(), slot(), numSignals() and signal() to get information
       about a class's signals and slots.
    \i numEnumerators() and enumerator() to obtain information about
       a class's enumerators.
    \i numProperties() and property() to obtain information about a
       class's properties.
    \endlist

    The search functions findSlot(), findSignal(), findEnumerator()
    and findProperty() map names of member functions, enumerators or
    properties to indices in the meta object. Qt uses
    e.g. findSignal() and findSlot() internally when you connect a
    signal to a slot.

    Classes may further have a list of name-value pairs of additonal
    class information. The number of pairs is returned by
    numClassInfo(), single pairs are returned by classInfo(), and you
    can search for pairs with findClassInfo().

    \sa \link moc.html moc (Meta QObject Compiler)\endlink

*/

enum ProperyFlags  {
    Invalid		= 0x00000000,
    Readable		= 0x00000001,
    Writable		= 0x00000002,
    Resetable		= 0x00000004,
    EnumOrSet		= 0x00000008,
    StdCppSet		= 0x00000100,
    Override		= 0x00000200,
    Designable		= 0x00001000,
    ResolveDesignable	= 0x00002000,
    Scriptable		= 0x00004000,
    ResolveScriptable	= 0x00008000,
    Stored		= 0x00010000,
    ResolveStored	= 0x00020000,
    Editable		= 0x00040000,
    ResolveEditable	= 0x00080000
};

struct QMetaObjectPrivate
{
    int revision;
    int className;
    int numClassInfo, classInfoData;
    int numSignals, signalData;
    int numSlots, slotData;
    int numProperties, propertyData;
    int numEnumerators, enumeratorData;
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
    \fn const char *QMetaObject::superClassName() const

    \obsolete
*/

/*!
    Returns true if this class inherits \a classname within the meta
    object inheritance chain; otherwise returns false.

    (A class is considered to inherit itself.)
*/
bool QMetaObject::inherits(const char* classname) const
{
    if (classname) {
	const QMetaObject *meta = this;
	do {
	    if ( strcmp(classname, meta->d.stringdata) == 0 )
		return true;
	} while ((meta = meta->d.superdata));
    }
    return false;
}

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
    if (qApp)
	return qApp->translate(d.stringdata, s, c, QApplication::DefaultCodec);
    else
	return QString::fromLatin1(s);
}
/*!
    \internal
    Forwards a trUtf8() call from the Q_OBJECT macro to QApplication
*/
QString QMetaObject::trUtf8(const char *s, const char *c) const
{
    if (qApp)
	return qApp->translate(d.stringdata, s, c, QApplication::UnicodeUTF8);
    else
	return QString::fromUtf8(s);
}
#endif // QT_NO_TRANSLATION


/*!
    Returns the slot offset, i.e. the summarized number of slots in
    all super classes.
 */
int QMetaObject::slotOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
	offset += priv(m->d.data)->numSlots;
	m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the signal offset, i.e. the summarized number of signals in
    all super classes.
 */
int QMetaObject::signalOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
	offset += priv(m->d.data)->numSignals;
	m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the enumerator offset, i.e. the summarized number of
    enumerators in all super classes.
 */
int QMetaObject::enumeratorOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
	offset += priv(m->d.data)->numEnumerators;
	m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the property offset, i.e. the summarized number of
    properties in all super classes.
 */
int QMetaObject::propertyOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
	offset += priv(m->d.data)->numProperties;
	m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the class information offset, i.e. the summarized number
    of items of class information in all super classes.
 */
int QMetaObject::classInfoOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
	offset += priv(m->d.data)->numClassInfo;
	m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the number of slots for this class.  If \a super is true,
    inherited slots are included.

    \sa slot()
*/
int QMetaObject::numSlots(bool super) const
{
    int n = priv(d.data)->numSlots;
    if (super) {
	const QMetaObject *m = d.superdata;
	while (m) {
	    n += priv(m->d.data)->numSlots;
	    m = m->d.superdata;
	}
    }
    return n;
}

/*!
    Returns the number of signals for this class.  If \a super is
    true, inherited signals are included.

    \sa signal()
*/
int QMetaObject::numSignals(bool super) const
{
    int n = priv(d.data)->numSignals;
    if (super) {
	const QMetaObject *m = d.superdata;
	while (m) {
	    n += priv(m->d.data)->numSignals;
	    m = m->d.superdata;
	}
    }
    return n;
}

/*!
    Returns the number of enumerators for this class.  If \a super is
    true, inherited enumerators are included.

    \sa enumerator()
*/
int QMetaObject::numEnumerators(bool super) const
{
    int n = priv(d.data)->numEnumerators;
    if (super) {
	const QMetaObject *m = d.superdata;
	while (m) {
	    n += priv(m->d.data)->numEnumerators;
	    m = m->d.superdata;
	}
    }
    return n;
}

/*!
    Returns the number of properties for this class.  If \a super is
    true, inherited properties are included.

    \sa property()
*/
int QMetaObject::numProperties(bool super) const
{
    int n = priv(d.data)->numProperties;
    if (super) {
	const QMetaObject *m = d.superdata;
	while (m) {
	    n += priv(m->d.data)->numProperties;
	    m = m->d.superdata;
	}
    }
    return n;
}

/*!
    Returns the number of items of class information for this class.
    If \a super is true, inherited classInfo are included.
*/
int QMetaObject::numClassInfo(bool super) const
{
    int n = priv(d.data)->numClassInfo;
    if (super) {
	const QMetaObject *m = d.superdata;
	while (m) {
	    n += priv(m->d.data)->numClassInfo;
	    m = m->d.superdata;
	}
    }
    return n;
}

/*!
    Finds \a slot and returns its index; otherwise returns -1.

    If \a super is true, inherited slots are included.

    \sa slot(), numSlots()
*/
int QMetaObject::findSlot(const char *slot, bool super) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
	for (i = priv(m->d.data)->numSlots-1; i >= 0; --i)
	    if (strcmp(slot, m->d.stringdata
		       + m->d.data[priv(m->d.data)->slotData + 4*i] ) == 0) {
		if (super)
		    i += m->slotOffset();
		break;
	    }
	m = m->d.superdata;
    }
    return i;
}

/*!

    Finds \a signal and returns its index; otherwise returns -1.

    If \a super is true, inherited slots are included.

    \sa signal(), numSignals()
*/
int QMetaObject::findSignal(const char *signal, bool super) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
	for (i = priv(m->d.data)->numSignals-1; i >= 0; --i)
	    if (strcmp(signal, m->d.stringdata
		       + m->d.data[priv(m->d.data)->signalData + 4*i] ) == 0) {
		if (super)
		    i += m->signalOffset();
		break;
	    }
	m = m->d.superdata;
    }
#if defined(QT_CHECK_RANGE)
    if (i >= 0 && super && m->d.superdata ) {
	int conflict = m->d.superdata->findSignal(signal, super);
	if ( conflict >= 0 )
	    qWarning( "QMetaObject::findSignal:%s: Conflict with %s::%s",
		      m->d.stringdata, m->d.superdata->d.stringdata, signal );
    }
#endif
    return i;
}


/*!
    Finds enumerator \a name and returns its index; otherwise returns
    -1.

    If \a super is true, inherited properties are included.

    \sa enumerator(), numEnumerators()
*/
int QMetaObject::findEnumerator(const char *name, bool super) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
	for (i = priv(m->d.data)->numEnumerators-1; i >= 0; --i) {
	    if (strcmp(name, m->d.stringdata
		       + m->d.data[priv(m->d.data)->enumeratorData + 4*i] ) == 0) {
		if (super)
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

    If \a super is true, inherited properties are included.

    \sa property(), numProperties()
*/
int QMetaObject::findProperty(const char *name, bool super) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
	for (i = priv(m->d.data)->numProperties-1; i >= 0; --i)
	    if (strcmp(name, m->d.stringdata
		       + m->d.data[priv(m->d.data)->propertyData + 3*i] ) == 0) {
		if (super)
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

    If \a super is true, inherited items of class information are
    included.

    \sa classInfo(), numClassInfo()
*/
int QMetaObject::findClassInfo(const char *name, bool super) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
	for (i = priv(m->d.data)->numClassInfo-1; i >= 0; --i)
	    if (strcmp(name, m->d.stringdata
		       + m->d.data[priv(d.data)->classInfoData + 2*i] ) == 0) {
		if (super)
		    i += m->classInfoOffset();
		break;
	    }
	m = m->d.superdata;
    }
    return i;
}

/*!
    Returns the meta data for the slot with index \a index.

    If \a super is true, inherited slots are included.
*/
QMetaMember QMetaObject::slot(int index, bool super) const
{
    int i = index;
    if (super) {
	i -= slotOffset();
	if (i < 0 && d.superdata)
	    return d.superdata->slot(index, super);
    }

    QMetaMember result;
    if (i >= 0 && i <= priv(d.data)->numSlots ) {
	result.mobj = this;
	result.handle = priv(d.data)->slotData + 4*i;
    }

    return result;
}

/*!
    Returns the meta data for the signal with index \a index.

    If \a super is true, inherited signal are included.
 */
QMetaMember QMetaObject::signal(int index, bool super) const
{
    int i = index;
    if (super) {
	i -= signalOffset();
	if (i < 0 && d.superdata)
	    return d.superdata->signal(index, super);
    }

    QMetaMember result;
    if (i >= 0 && i <= priv(d.data)->numSignals ) {
	result.mobj = this;
	result.handle = priv(d.data)->signalData + 4*i;
    }

    return result;

}


/*!
    Returns the meta data for the enumerator with index \a index.

    If \a super is true, inherited enumerator are included.
 */
QMetaEnum QMetaObject::enumerator(int index, bool super) const
{
    int i = index;
    if (super) {
	i -= enumeratorOffset();
	if (i < 0 && d.superdata)
	    return d.superdata->enumerator(index, super);
    }

    QMetaEnum result;
    if (i >= 0 && i <= priv(d.data)->numEnumerators ) {
	result.mobj = this;
	result.handle = priv(d.data)->enumeratorData + 4*i;
    }
    return result;
}

/*!
    Returns the meta data for the property with index \a index.

    If \a super is true, inherited properties are included.
 */
QMetaProperty QMetaObject::property(int index, bool super) const
{
    int i = index;
    if (super) {
	i -= propertyOffset();
	if (i < 0 && d.superdata)
	    return d.superdata->property(index, super);
    }
    QMetaProperty result;
    if (i >= 0 && i <= priv(d.data)->numProperties ) {
	int handle = priv(d.data)->propertyData + 3*i;
	int flags = d.data[handle + 2];
	const char *name = d.stringdata + d.data[handle];
	const char *type = d.stringdata + d.data[handle + 1];
	if ((flags & Override) && d.superdata){
	    result = property(d.superdata->findProperty(name, true), true);
 	    if (qstrcmp(result.type(), type)) // type missmatch, no override
 		::memset(&result, 0, sizeof(QMetaProperty));
	}
	if (flags & EnumOrSet) {
	    result.menum = enumerator(findEnumerator(type, true), true);
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

    If \a super is true, inherited items of class information are
    included.
 */
QMetaClassInfo QMetaObject::classInfo(int index, bool super) const
{
    int i = index;
    if (super) {
	i -= classInfoOffset();
	if (i < 0 && d.superdata)
	    return d.superdata->classInfo(index, super);
    }

    QMetaClassInfo result;
    if (i >= 0 && i <= priv(d.data)->numClassInfo ) {
	result.mobj = this;
	result.handle = priv(d.data)->classInfoData + 2*i;
    }
    return result;
}

/*!
    Returns true if the \a signal and the \a member arguments are
    compatible; otherwise returns false.

    Both \a signal and \a member are expected to be normalized.

    \sa normalizeSignature()
*/
bool QMetaObject::checkConnectArgs( const char *signal, const char *member )
{
    const char *s1 = signal;
    const char *s2 = member;
    while ( *s1++ != '(' ) { }			// scan to first '('
    while ( *s2++ != '(' ) { }
    if ( *s2 == ')' || qstrcmp(s1,s2) == 0 )	// member has no args or
	return true;				//   exact match
    int s1len = strlen(s1);
    int s2len = strlen(s2);
    if ( s2len < s1len && strncmp(s1,s2,s2len-1)==0 && s1[s2len-1]==',' )
	return true;				// member has less args
    return false;
}

static inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace( char x )
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.  isspace() usually
    works, but not here.  This implementation is sufficient for our
    internal use.
  */
    return (uchar) x <= 32;
#else
    return isspace( (uchar) x );
#endif
}

static QByteArray normalizeTypeInternal(const char *t, const char *e, bool adjustConst = true){
    int len = e - t;
    if (strncmp("void", t, len) == 0 )
	return "";
    /*
      Convert 'char const *' into 'const char *'. Start at index 1,
      not 0, because 'const char *' is already OK.
    */
    QByteArray constbuf;
    for ( int i = 1; i < len; i++ ) {
	if ( t[i] == 'c' &&
	     strncmp(t + i + 1, "onst", 4) == 0 ) {
	    constbuf = QByteArray(t, len);
	    if ( isSpace( t[i-1] ) )
		constbuf.remove( i-1, 6 );
	    else
		constbuf.remove( i, 5 );
	    constbuf.prepend( "const" );
	    t = constbuf.data();
	    e = constbuf.data() + constbuf.length();
	    break;
	}
	/*
	  We musn't convert 'char * const *' into 'const char **'
	  and we must beware of 'Bar<const Bla>'.
	*/
	if ( t[i] == '&' || t[i] == '*' ||t[i] == '<' )
	    break;
    }
    if (adjustConst && e > t + 6 && strncmp("const ", t, 6) == 0 ) {
	if ( *(e-1) == '&' ) { // treat const reference as value
	    t += 6;
	    --e;
	} else if ( isIdentChar(*(e-1)) ) { // treat const value as value
	    t += 6;
	}
    }
    QByteArray result;
    result.reserve(len);

    // some type substitutions for 'unsigned x'
    if (strncmp("unsigned ", t, 9) == 0) {
	if ( strncmp("int", t+9, 3) == 0) {
	    t += 9+3;
	    result += "uint";
	} else if ( strncmp("long", t+9, 4) == 0) {
	    t += 9+4;
	    result += "ulong";
	}
    }
    while (t != e) {
	result += *t;
	if ( *t == '<' ) {
	    //template recursion
	    const char* tt = ++t;
	    int templdepth = 1;
	    while (*++t) {
		if ( *t == '<' )
		    ++templdepth;
		if ( *t == '>' )
		    --templdepth;
		if (templdepth == 0) {
		    result += normalizeTypeInternal(tt, t, false);
		    result += *t;
		    if (t[1] == '>' )
			result += ' '; // avoid >>
		    break;
		}
	    }
	}
	++t;
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
QByteArray QMetaObject::normalizeSignature(const char *member)
{
    const char *s = member;
    if (!s)
	return "";
    int len = strlen(s);
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len] : stackbuf);
    char *d = buf;
    char last = 0;
    while( *s && isSpace(*s) )
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    *d = '\0';
    d = buf;

    QByteArray result;
    result.reserve(len);

    int argdepth = 0;
    int templdepth = 0;
    while ( *d ) {
	if ( argdepth == 1 ) {
	    const char *t = d;
	    while ( *d&& ( templdepth
			   || ( *d != ',' && *d != ')') ) ) {
		if ( *d == '<' )
		    ++templdepth;
		if ( *d == '>' )
		    --templdepth;
		d++;
	    }
	    result += normalizeTypeInternal(t, d);
	}
	if ( *d == '(' )
	    ++argdepth;
	if ( *d == ')' )
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
    Returns the access specification of this member: private,
    protected, or public. Signals are always protected.
*/

QMetaMember::Access QMetaMember::access() const
{
    if (!mobj)
	return Private;
    return (Access) mobj->d.data[handle + 3];
}


/*!
    \class QMetaEnum qmetaobject.h

    \brief The QMetaEnum class provides meta data about an enumerator.

    \ingroup objectmodel

    Use name() for the enumerator's name and keys() to get a list of
    all keys. The function isSet() returns whether the enumerator is
    meant to be a set, meaning its values can be OR-ed together.

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

    \sa numKeys()
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
    Returns true if this enumerator is is used as set, i.e. the
    enumeration values can be OR-ed together; otherwise returns false.

    \sa keysToValue(), valueToKeys()
*/
bool QMetaEnum::isSet() const
{
    return mobj && mobj->d.data[handle + 1];
}

/*!
    Converts the enumeration key \a key to its integer value.

    For set types, use keysToValue().

    \sa valueToKey(), isSet(), keysToValue()
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
    Converts the enumeration value \a value to its literal key.

    For set types, use valueToKeys().

    \sa valueToKey(), isSetType(), valueToKeys()
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
    Converts the list of '|'-separated keys \a keys to their combined
    (OR-ed) integer value.

    \sa isSet(), valueToKey(), keysToValue()
*/
int QMetaEnum::keysToValue(const char *keys) const
{
    if (!mobj)
	return -1;
    QStringList l = QStringList::split('|', QString::fromLatin1(keys));
    //#### TODO write proper code, do not use QStringList
    int value = 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int li = 0; li < (int)l.size(); ++li) {
	QString s = l[li].stripWhiteSpace();
	int i;
	for (i = count-1; i >= 0; --i)
	    if (!strcmp(s.latin1(),
			mobj->d.stringdata + mobj->d.data[data + 2*i])) {
		value |= mobj->d.data[data + 2*i + 1];
		break;
	    }
	if ( i < 0 )
	    value |= -1;
    }
    return value;
}

/*!
    Converts the set value \a value to a list of '|'-separated keys.

    \sa isSet(), valueToKey(), valueToKeys()
*/
QByteArray QMetaEnum::valueToKeys(int value) const
{
    QByteArray keys;
    if (!mobj)
	return keys;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    int v = value;
    for( int i = count - 1; i >= 0; --i ) {
	int k = mobj->d.data[data + 2*i + 1];
	if ( ( k != 0 && (v & k) == k  ) ||  ( k == value) )  {
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

    Property meta data includes type(), name(), and whether a property
    isReadable(), isWritable(), isDesignable(), isScriptable(),
    isStored() or isEditable() .

    The functions isSetType(), isEnumType() and enumerator() provide
    further information about a property's type.

    Actual property values are set and received with read(), write(),
    or reset(); or through QObject's set and get functions. See
    QObject::setProperty() and QObject::property() for details.

    You receive meta property data through an object's meta object.
    See QMetaObject::property() and QMetaObject::numProperties() for
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
    Returns the name of this property.
 */
const char *QMetaProperty::name() const
{
    if (!mobj[QMetaObject::ReadProperty])
	return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    return mobj[QMetaObject::ReadProperty]->d.stringdata + mobj[QMetaObject::ReadProperty]->d.data[handle];
}

/*!
    Returns the type of this property.
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
    is used as set, i.e. if the enumeration values can be OR-ed
    together; otherwise returns false. A set type is implicitly also
    an enum type.

    \sa isEnumType(), enumerator()
*/

bool QMetaProperty::isSetType() const
{
    return isEnumType() && menum.isSet();
}

/*!
    Returns true if the property's type is an enumeration value;
    otherwise returns false.

    \sa enumerator(), isSetType()
*/
bool QMetaProperty::isEnumType() const
{
    if (!mobj[QMetaObject::ReadProperty])
	return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
    return (flags & EnumOrSet) && menum.name();
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
    Tries to read the property's value from object \a obj.  On
    success, returns the value; otherwise returns an invalid variant.
*/
QVariant QMetaProperty::read(const QObject *obj) const
{
    if (!obj || !mobj[QMetaObject::ReadProperty])
	return QVariant();

    QVariant::Type t = QVariant::Int;
    if (!isEnumType()) {
	int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
	int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
	t = (QVariant::Type)(flags >> 24);
	if ( t == QVariant::Invalid )
	    t = QVariant::nameToType( mobj[QMetaObject::ReadProperty]->d.stringdata
				      + mobj[QMetaObject::ReadProperty]->d.data[handle + 1] );
	if (t == QVariant::Invalid)
	    return QVariant();
    }
    QVariant value(t);
    void *argv[] = { value.data() };
    const_cast<QObject*>(obj)->qt_metacall(QMetaObject::ReadProperty,
		     idx[QMetaObject::ReadProperty] + mobj[QMetaObject::ReadProperty]->propertyOffset(),
		     argv);
    if (argv[0] != value.data())
	return QVariant(t, argv[0]);
    return value;
}

/*!
    Tries to write \a value as the property's value on object \a
    obj. On success, returns true; otherwise returns false.
*/
bool QMetaProperty::write(QObject *obj, const QVariant &value) const
{
    if (!obj || !isWritable())
	return false;

    QVariant v = value;
    if (isEnumType()) {
	if (v.type() == QVariant::String || v.type() == QVariant::CString) {
	    if ( isSetType() )
		v = QVariant(menum.keysToValue(value.toCString()));
	    else
		v = QVariant(menum.keyToValue(value.toCString()));
	} else if (v.type() != QVariant::Int && v.type() != QVariant::UInt) {
	    return false;
	}
	v.cast(QVariant::Int);
    } else {
	int handle = priv(mobj[QMetaObject::WriteProperty]->d.data)->propertyData + 3*idx[QMetaObject::WriteProperty];
	int flags = mobj[QMetaObject::WriteProperty]->d.data[handle + 2];
	QVariant::Type t = (QVariant::Type)(flags >> 24);
	if ( t == QVariant::Invalid )
	    t = QVariant::nameToType( mobj[QMetaObject::WriteProperty]->d.stringdata
				      + mobj[QMetaObject::WriteProperty]->d.data[handle + 1] );
	if (t != QVariant::Invalid && !v.cast(t))
	    return false;
    }

    void *argv[] = { v.data() };
    obj->qt_metacall(QMetaObject::WriteProperty,
		     idx[QMetaObject::WriteProperty] + mobj[QMetaObject::WriteProperty]->propertyOffset(),
		     argv);
    return true;
}

/*!
    Tries to reset the property for object \a obj with a reset method.
    On success, returns true; otherwise returns false.

    Reset methods are optional, usually only few properties support
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
    return !(flags & EnumOrSet) || menum.name();
}


static bool qt_query_property(const QMetaObject*const*mobj,const int *idx, uint flag,
			      QMetaObject::Call call, const QObject* obj)
{
    if (!mobj[call] )
	return false;
    int handle = priv(mobj[call]->d.data)->propertyData + 3*idx[call];
    int flags = mobj[call]->d.data[handle + 2];
    bool b = (flags & flag);
    if (obj) {
	void *argv[] = { 0, &b };
	const_cast<QObject*>(obj)->qt_metacall(call,
					       idx[call]
					       + mobj[call]->propertyOffset(),
					       argv);
    }
    return b;
}

/*!
    Returns true if the property is designable for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns a static
    approximation.
 */
bool QMetaProperty::isDesignable(const QObject *obj) const
{
    if (!mobj[QMetaObject::WriteProperty])
	return false;
    return qt_query_property( mobj, idx, Designable,
			      QMetaObject::QueryPropertyDesignable,
			      obj);
}

/*!
    Returns true if the property is scriptable for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns a static
    approximation.
 */
bool QMetaProperty::isScriptable(const QObject *obj) const
{
    return qt_query_property( mobj, idx, Scriptable,
			      QMetaObject::QueryPropertyScriptable,
			      obj);
}

/*!
    Returns true if the property is stored for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns a static
    approximation.
 */
bool QMetaProperty::isStored(const QObject *obj) const
{
    return qt_query_property( mobj, idx, Stored,
			      QMetaObject::QueryPropertyStored,
			      obj);
}

/*!
    Returns true if the property is editable for object \a obj;
    otherwise returns false.

    If no object \a obj is given, the function returns a static
    approximation.
 */
bool QMetaProperty::isEditable(const QObject *obj) const
{
    return qt_query_property( mobj, idx, Editable,
			      QMetaObject::QueryPropertyEditable,
			      obj);
}


/*!
    \class QMetaClassInfo qmetaobject.h

    \brief The QMetaClassInfo class provides additional information on a
    class

    \ingroup objectmodel


    A class information item is a simple \e name - \e value pair that
    was declared with Q_CLASSINFO in the code. Access its name with
    name() and its value with value().
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
