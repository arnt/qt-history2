/****************************************************************************
**
** Implementation of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsettings.h"

#include <qfile.h>
#include <qvariant.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qimage.h>
#include <qpointarray.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qfont.h>
#include <qsizepolicy.h>
#include <qxml.h>
#include <qregion.h>

// #define QSETTINGS_NODE_DEBUG
// #define QSETTINGS_ENTRY_DEBUG




/*!
  \class QSettings qsettings.h
  \brief The QSettings class provides persistent application settings.

  \ingroup misc

  QSettings provides a platform independent way to load and save settings
  for applications.

  Settings are organized in a tree. Each node is a QVariant, and all
  nodes are indexed by strings.  readEntry(), writeEntry() and
  removeEntry() all work on QString keys and QVariant data.

  The key is a Unicode string, similar to UNIX file paths.  The key
  must begin with a slash, must not end with a slash and must not
  contain "//".

  The object can be writable or read-only. Read-only is the default,
  and is useful e.g. for system-wide and for an application's default
  settings.  Writable settings are, of course, well-suited for user
  settings.

  QSettings can chain together multiple settings objects to allow searching
  in multiple objects by specifying a \e fallback.  For example, if you
  need to search system wide-defaults, per-application defaults and
  user-specific settings, something like this could be done:

  \code
  // we cannot change these (may not have write access to the files)
  QSettings sApp(FALSE);
  QSettings sSys(FALSE);

   // we can change the user settings
  QSettings sUser(TRUE);

  sUser.setFallback ( &sSys );
  sSys.setFallback ( &sApp );

  // set the file paths with QSettings::setPath()
  ...

  QVariant v = sUser.readEntry ( "/mysoft/myapp/showSplash" );

  // check the validity and type of the QVariant
  ...

  \endcode

  The call to sUser.readEntry() above would first look in sUser,
  then sSys, and finally in sApp.
*/




// **********************************************************************
// QSettingsNode internal data structure for QSettings
// **********************************************************************

class QSettingsNode
{
public:
    QSettingsNode();
    ~QSettingsNode();

    void addChild(QSettingsNode *child);
    void removeChild(QSettingsNode *child);

    QString attrString() const;
    void setAttributes(const QXmlAttributes &a);

    QMap<QString,QString> attributes;
    QString data, tagName;

    QSettingsNode *parent, *prev, *next, *first, *last;
};


inline QSettingsNode::QSettingsNode()
    : parent(0), prev(0), next(0), first(0), last(0)
{
}


inline QSettingsNode::~QSettingsNode()
{
    QSettingsNode *n = first, *m;

    while (n) {
	m = n->next;
	delete n;
	n = m;
    }

    parent = prev = next = first = last = 0;
}


inline void QSettingsNode::addChild(QSettingsNode *child)
{
    child->parent = this;

    if (last) {
	last->next = child;
    }
    child->prev = last;

    if (! first) {
	first = child;
    }
    last = child;

#ifdef QSETTINGS_NODE_DEBUG
    qDebug("QSettingsNode::addChild: parent %p child %p first %p last %p",
	   this, child, first, last);
#endif // QSETTINGS_NODE_DEBUG

}


inline void QSettingsNode::removeChild(QSettingsNode *child)
{
    if (child->parent != this ||
	(! child->next &&
	 ! child->prev &&
	 first != child)) {

#ifdef QSETTINGS_NODE_DEBUG
	qDebug("QSettingsNode::removeChild: this %p is not parent of  %p",
	       this, child);
#endif // QSETTINGS_NODE_DEBUG

	return;
    }

    if (child->next)
	child->next->prev = child->prev;
    if (child->prev)
	child->prev->next = child->next;

    if (last == child)
	last = child->prev;
    if (first == child)
	first = child->next;

    child->parent = child->prev = child->next = 0;

#ifdef QSETTINGS_NODE_DEBUG
    qDebug("QSettingsNode::removeChild: parent %p child %p first %p last %p",
	   this, child, first, last);
#endif // QSETTINGS_NODE_DEBUG

}


inline void QSettingsNode::setAttributes(const QXmlAttributes &a)
{
    for (int i = 0; i < a.length(); i++) {
	attributes[a.qName(i)] = a.value(i);

#ifdef QSETTINGS_NODE_DEBUG
	qDebug("QSettingsNode::setAttributes: %s = %s",
	       a.qName(i).latin1(), a.value(i).latin1());
#endif // QSETTINGS_NODE_DEBUG

    }
}


inline QString QSettingsNode::attrString() const
{
    QString as;

    QMapConstIterator<QString,QString> it;
    it = attributes.begin();
    while (it != attributes.end()) {
	as += " ";
	as += it.key();
	as += "=\"";
	as += it.data();
	as += "\"";

	++it;
    }

#ifdef QSETTINGS_NODE_DEBUG
    qDebug("QSettingsNode::attrString: %s", as.latin1());
#endif // QSETTINGS_NODE_DEBUG

    return as;
}




// **********************************************************************
// QSettingsXmlHandler - XML parser interface
// **********************************************************************

class QSettingsXmlHandler : public QXmlDefaultHandler
{
public:
    QSettingsXmlHandler();

    bool startDocument();
    bool endDocument();
    bool startElement(const QString &ns, const QString &ln, const QString &qName,
		      const QXmlAttributes &attr);
    bool endElement(const QString &ns, const QString &ln, const QString &qName);
    bool characters(const QString &ch);

    QSettingsNode *node, *tree;
};


inline QSettingsXmlHandler::QSettingsXmlHandler()
    : node(0)
{
}


inline bool QSettingsXmlHandler::startDocument()
{
    tree = node = new QSettingsNode;
    node->tagName = "QSettings";

    return TRUE;
}


inline bool QSettingsXmlHandler::endDocument()
{
    if (node != tree)
	return FALSE;

    return TRUE;
}

inline bool QSettingsXmlHandler::startElement(const QString &,
					      const QString &,
					      const QString &qName,
					      const QXmlAttributes &attr)
{
    QSettingsNode *nnode = new QSettingsNode;
    nnode->setAttributes(attr);
    nnode->tagName = qName;

    node->addChild(nnode);
    node = nnode;

    return TRUE;
}


inline bool QSettingsXmlHandler::endElement(const QString &,
					    const QString &,
					    const QString &)
{
    if (node == tree)
	return FALSE;

    node = node->parent;
    return TRUE;
}


inline bool QSettingsXmlHandler::characters(const QString &ch)
{
    node->data += ch;

    return TRUE;
}




// **********************************************************************
// Base64 encoder/decoder
// **********************************************************************

static const char *Base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789+/";

static char Base64Index[128] = {
    '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
    '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
    '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
    '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
    '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
    '\377', '\377', '\377', 62    , '\377', '\377', '\377', 63    ,
    52    , 53    , 54    , 55    , 56    , 57    , 58    , 59    ,
    60    , 61    , '\377', '\377', '\377', '\377', '\377', '\377',
    '\377', 0     , 1     , 2     , 3     , 4     , 5     , 6     ,
    7     , 8     , 9     , 10    , 11    , 12    , 13    , 14    ,
    15    , 16    , 17    , 18    , 19    , 20    , 21    , 22    ,
    23    , 24    , 25    , '\377', '\377', '\377', '\377', '\377',
    '\377', 26    , 27    , 28    , 29    , 30    , 31    , 32    ,
    33    , 34    , 35    , 36    , 37    , 38    , 39    , 40    ,
    41    , 42    , 43    , 44    , 45    , 46    , 47    , 48    ,
    49    , 50    , 51    , '\377', '\377', '\377', '\377', '\377'
};


static inline int isbase64(int a)
{
    return (('A' <= a && a <= 'Z') ||
	    ('a' <= a && a <= 'z') ||
	    ('0' <= a && a <= '9') ||
	    (a == '+') || (a == '/'));
}


static inline QByteArray encodeBase64 (const QByteArray &ina)
{
    if (ina.isNull() || ina.isEmpty()) {
	return QByteArray(0);
    }

    uint osize = (ina.count() + 2) * 4 / 3, inpos = 0, outpos = 0,
       linelen = 0, stop = ina.count() - 3;

    osize += ((strlen("\n") * osize) / 76) + 2;

    char *in = ina.data();
    char *out = new char[osize];

    int c[3];

    while (inpos < stop) {
	c[0] = in[inpos++] & 0xff;
	c[1] = in[inpos++] & 0xff;
	c[2] = in[inpos++] & 0xff;

	out[outpos++] = Base64Table[(c[0] & 0xfc) >> 2];
	out[outpos++] = Base64Table[((c[0] & 0x03) << 4) | ((c[1] & 0xf0) >> 4)];
	out[outpos++] = Base64Table[((c[1] & 0x0f) << 2) | ((c[2] & 0xc0) >> 6)];
	out[outpos++] = Base64Table[c[2] & 0x3f];

	linelen += 4;

	if (linelen >= 73) {
	    out[outpos++] = '\n';
	    linelen = 0;
	}
    }

    switch (ina.count() % 3) {
    case 1:
	{
	    c[0] = in[inpos] & 0xff;

	    out[outpos++] = Base64Table[(c[0] & 0xfc) >> 2];
	    out[outpos++] = Base64Table[((c[0] & 0x03) << 4)];
	    out[outpos++] = '=';
	    out[outpos++] = '=';

	    break;
	}

    case 2:
	{
	    c[0] = in[inpos++] & 0xff;
	    c[1] = in[inpos] & 0xff;

	    out[outpos++] = Base64Table[(c[0] & 0xfc) >> 2];
	    out[outpos++] = Base64Table[((c[0] & 0x03) << 4) | ((c[1] & 0xf0) >> 4)];
	    out[outpos++] = Base64Table[((c[1] & 0x0f) << 2)];
	    out[outpos++] = '=';

	    break;
	}
    }

    out[outpos++] = '\n';
    out[outpos++] = '\0';

    QByteArray ret;
    ret.assign(out, outpos);

    return ret;
}


static inline QByteArray decodeBase64(const QByteArray &ina)
{
    if (ina.isNull() || ina.isEmpty()) {
	return QByteArray(0);
    }

    uint osize = (((ina.count() / 4) + 1) * 3) + 1, inpos = 0, outpos = 0;

    char *in = ina.data();
    char *out = new char[osize];

    bool atEnd = FALSE;
    int c[4], d[3];

    while (inpos < ina.count()) {
	c[0] = c[1] = c[2] = c[3] = 0;

	while (inpos < ina.count()) {
	    c[0] = in[inpos++] & 0xff;

	    if (isbase64(c[0])) {
		break;
	    } else if (c[0] == '=') {
		atEnd = true;
		break;
	    }
	}

	while (inpos < ina.count()) {
	    c[1] = in[inpos++] & 0xff;

	    if (isbase64(c[1])) {
		break;
	    } else if (c[1] == '=') {
		atEnd = true;
		break;
	    }
	}

	while (inpos < ina.count()) {
	    c[2] = in[inpos++] & 0xff;

	    if (isbase64(c[2])) {
		break;
	    } else if (c[2] == '=') {
		atEnd = true;
		break;
	    }
	}

	while (inpos < ina.count()) {
	    c[3] = in[inpos++] & 0xff;

	    if (isbase64(c[3])) {
		break;
	    } else if (c[3] == '=') {
		atEnd = true;
		break;
	    }
	}

	if (isbase64(c[0]) &&
	    isbase64(c[1]) &&
	    isbase64(c[2]) &&
	    isbase64(c[3])) {

            c[0] = Base64Index[c[0]] & 0xFF;
            c[1] = Base64Index[c[1]] & 0xFF;
            c[2] = Base64Index[c[2]] & 0xFF;
            c[3] = Base64Index[c[3]] & 0xFF;

            d[0] = ((c[0] << 2) & 0xFC) | ((c[1] >> 4) & 0x03);
            d[1] = ((c[1] << 4) & 0xF0) | ((c[2] >> 2) & 0x0F);
            d[2] = ((c[2] << 6) & 0xC0) | ( c[3]       & 0x3F);

            out[outpos++] = char(d[0]);
            out[outpos++] = char(d[1]);
            out[outpos++] = char(d[2]);
        } else if (isbase64(c[0]) &&
		   isbase64(c[1]) &&
		   isbase64(c[2]) &&
		   c[3] == '=') {

            c[0] = Base64Index[c[0]] & 0xFF;
            c[1] = Base64Index[c[1]] & 0xFF;
            c[2] = Base64Index[c[2]] & 0xFF;

            d[0] = ((c[0] << 2) & 0xFC) | ((c[1] >> 4) & 0x03);
            d[1] = ((c[1] << 4) & 0xF0) | ((c[2] >> 2) & 0x0F);

            out[outpos++] = char(d[0]);
            out[outpos++] = char(d[1]);

            break;
        } else if (isbase64(c[0]) &&
		   isbase64(c[1]) &&
		   c[2] == '=' &&
		   c[3] == '=') {

            c[0] = Base64Index[c[0]] & 0xFF;
            c[1] = Base64Index[c[1]] & 0xFF;

            d[0] = ((c[0] << 2) & 0xFC) | ((c[1] >> 4) & 0x03);

            out[outpos++] = char(d[0]);

            break;
        } else {
            break;
        }

        if (atEnd) {
            break;
        }
    }

    QByteArray ret;
    ret.assign(out ,outpos);

    return ret;
}




// **********************************************************************
// static helper functions
// **********************************************************************

// QPoint
static inline QPoint readPoint(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    int x = 0, y = 0;

    while (n) {
	if (n->tagName == "x")
	    x = n->first->data.toInt();
	else if (n->tagName == "y")
	    y = n->first->data.toInt();

	n = n->next;
    }

    return QPoint(x, y);
}

static inline void writePoint(QSettingsNode *e, const QPoint &p)
{
    e->tagName = "point";

    if (! e->first)
	e->addChild(new QSettingsNode);
    if (! e->first->next)
	e->addChild(new QSettingsNode);

    e->first->tagName = "x";
    e->first->data = QString::number(p.x());
    e->first->next->tagName = "y";
    e->first->next->data = QString::number(p.y());
}


// QFont
static inline QFont readFont(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QFont font;

    while (n) {
	if (n->tagName == "family")
	    font.setFamily(n->data);
	else if (n->tagName == "pointsize")
	    font.setPointSize(n->data.toInt());
	else if (n->tagName == "bold")
	    font.setBold(n->data.toInt());
	else if (n->tagName == "italic")
	    font.setItalic(n->data.toInt());
	else if (n->tagName == "underline")
	    font.setUnderline(n->data.toInt());
	else if (n->tagName == "strikeout")
	    font.setStrikeOut(n->data.toInt());

	n = n->next;
    }

    return font;
}

static inline void writeFont(QSettingsNode *e, const QFont &font)
{
    e->tagName = "font";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    n->tagName = "family";
    n->data = font.family();

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "pointsize";
    n->data = QString::number(font.pointSize());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "bold";
    n->data = QString::number(font.bold());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "italic";
    n->data = QString::number(font.italic());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "underline";
    n->data = QString::number(font.underline());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "strikeout";
    n->data = QString::number(font.strikeOut());
}


// QRect
static inline QRect readRect(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    int x = 0, y = 0, w = 0, h = 0;

    while (n) {
	if (n->tagName == "x")
	    x = n->data.toInt();
	else if (n->tagName == "y")
	    y = n->data.toInt();
	else if (n->tagName == "width")
	    w = n->data.toInt();
	else if (n->tagName == "height")
	    h = n->data.toInt();

	n = n->next;
    }

    return QRect(x, y, w, h);
}

static inline void writeRect(QSettingsNode *e, const QRect &r)
{
    e->tagName = "rect";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    n->tagName = "x";
    n->data = QString::number(r.x());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "y";
    n->data = QString::number(r.y());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "width";
    n->data = QString::number(r.width());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "height";
    n->data = QString::number(r.height());
}


// QCString
static inline QCString readCString(QSettingsNode *e)
{
    return e->data.utf8();
}

static inline void writeCString(QSettingsNode *e, const QCString &s)
{
    e->tagName = "cstring";
    e->data = QString::fromUtf8(s);
}


// QString
static inline QString readString(QSettingsNode *e)
{
    return e->data;
}

static inline void writeString(QSettingsNode *e, const QString &s)
{
    e->tagName = "string";
    e->data = s;
}


// double
static inline double readDouble(QSettingsNode *e)
{
    return e->data.toDouble();
}

static inline void writeDouble(QSettingsNode *e, double d)
{
    e->tagName = "double";
    e->data = QString::number(d, 'g', 10);
}


// int
static inline int readInt(QSettingsNode *e)
{
    return e->data.toInt();
}

static inline void writeInt(QSettingsNode *e, int i)
{
    e->tagName = "int";
    e->data = QString::number(i);
}


// uint
static inline uint readUInt(QSettingsNode *e)
{
    return e->data.toUInt();
}

static inline void writeUInt(QSettingsNode *e, uint u)
{
    e->tagName = "uint";
    e->data = QString::number(u);
}


// bool
static inline bool readBool(QSettingsNode *e)
{
    bool value = false;

    if (e->data == "true") {
	value = TRUE;
    } else if (e->data == "false") {
	value = FALSE;
    } else {
	uint n = e->data.toUInt();

	if (n > 0) value = TRUE; else value = FALSE;
    }

    return value;
}

static inline void writeBool(QSettingsNode *e, bool b)
{
    e->tagName = "bool";
    e->data = QString(b ? "true" : "false");
}


// QSize
static inline QSize readSize(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    int w = -1, h = -1;

    while (n) {
	if (n->tagName == "width") {
	    w = n->data.toInt();
	} else if (n->tagName == "height") {
	    h = n->data.toInt();
	}

	n = n->next;
    }

    return QSize(w, h);
}

static inline void writeSize(QSettingsNode *e, const QSize &s)
{
    e->tagName = "size";

    if (! e->first)
	e->addChild(new QSettingsNode);
    if (! e->first->next)
	e->addChild(new QSettingsNode);

    e->first->tagName = "width";
    e->first->data = QString::number(s.width());
    e->first->next->tagName = "height";
    e->first->next->data = QString::number(s.height());
}


// QColor
static inline QColor readColor(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    int r = 0, g = 0, b = 0;

    while (n) {
	if (n->tagName == "red") {
	    r = n->data.toInt();
	} else if (n->tagName == "green") {
	    g = n->data.toInt();
	} else if (n->tagName == "blue") {
	    b = n->data.toInt();
	}

	n = n->next;
    }

    return QColor(r, g, b);
}

static inline void writeColor(QSettingsNode *e, const QColor &c)
{
    e->tagName = "color";

    if (! e->first)
	e->addChild(new QSettingsNode);
    if (! e->first->next)
	e->addChild(new QSettingsNode);
    if (! e->first->next->next)
	e->addChild(new QSettingsNode);

    e->first->tagName = "red";
    e->first->data = QString::number(c.red());
    e->first->next->tagName = "green";
    e->first->next->data = QString::number(c.green());
    e->first->next->next->tagName = "blue";
    e->first->next->next->data = QString::number(c.blue());
}


// QColorGroup
static inline QColorGroup readColorGroup(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QColorGroup cg;

    for (int i = 0; i < QColorGroup::NColorRoles && n; i++) {
	if (n->tagName == "color") {
	    cg.setColor((QColorGroup::ColorRole) i, readColor(n));
	}

	n = n->next;
    }

    return cg;
}

static inline void writeColorGroup(QSettingsNode *e, const QColorGroup &cg)
{
    e->tagName = "colorgroup";

    if (! e->first)
	e->addChild(new QSettingsNode);

    QSettingsNode *n = e->first;

    for (int i = 0; i < QColorGroup::NColorRoles; i++) {
	writeColor(n, cg.color((QColorGroup::ColorRole) i));

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// QPalette
static inline QPalette readPalette(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QColorGroup a, i, d;

    while (n) {
	if (n->tagName == "active") {
	    a = readColorGroup(n);
	} else if (n->tagName == "inactive") {
	    i = readColorGroup(n);
	} else if (n->tagName == "disabled") {
	    d = readColorGroup(n);
	}

 	n = n->next;
    }

    return QPalette(a, d, i);
}

static inline void writePalette(QSettingsNode *e, const QPalette &p)
{
    e->tagName = "palette";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    writeColorGroup(n, p.active());
    n->tagName = "active";

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    writeColorGroup(n, p.inactive());
    n->tagName = "inactive";

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    writeColorGroup(n, p.disabled());
    n->tagName = "disabled";
}


// QStringList
static inline QStringList readStringList(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QStringList list;

    while (n) {
	if (n->tagName == "string") {
	    list.append(readString(n));
	}

	n = n->next;
    }

    return list;
}

static inline void writeStringList(QSettingsNode *e, const QStringList &strlist)
{
    e->tagName = "stringlist";

    if (! e->first)
	e->addChild(new QSettingsNode);

    QSettingsNode *n = e->first;
    QStringList::ConstIterator it = strlist.begin();

    while (it != strlist.end()) {
	n->tagName = "string";
	n->data = (*it);

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
	++it;
    }

    e->removeChild(n);
    delete n;
}


// QPixmap
static inline QPixmap readPixmap(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QSize sz;
    int depth = 0;
    QByteArray data;

    while (n) {
	if (n->tagName == "size") {
	    sz = readSize(n);
	} else if (n->tagName == "depth") {
	    depth = n->first->data.toInt();
	} else if (n->tagName == "data") {
	    data.duplicate(n->first->data.utf8(),
			   n->first->data.length());
	}

	n = e->first;
    }

    QPixmap pixmap;

    if (! data.isNull() &&
	depth > 0 &&
	sz.isValid() &&
	! sz.isNull()) {
	QByteArray imgdata = decodeBase64(data);

	if (! imgdata.isNull()) {
	    QImage img((uchar *) data.data(), sz.width(), sz.height(), depth,
		       0, 0, QImage::systemByteOrder());
	    pixmap = img;
	}
    }

    return pixmap;
}

static inline void writePixmap(QSettingsNode *e, const QPixmap &pixmap)
{
    e->tagName = "pixmap";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    writeSize(n, pixmap.size());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "data";

    QImage img;
    img = pixmap;
    QByteArray ba;
    ba.duplicate((const char *) img.bits(),
		 img.width() * img.height() * (img.depth() / 8));
    n->data = "\n" + QString::fromLocal8Bit(encodeBase64(ba).data());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "depth";
    n->data = QString::number(img.depth());
}


// QBrush
static inline QBrush readBrush(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QBrush brush;

    while (n) {
	if (n->tagName == "color") {
	    brush.setColor(readColor(n));
	} else if (n->tagName == "pixmap") {
	    brush.setPixmap(readPixmap(n));
	} else if (n->tagName == "brushstyle") {
	    if (n->first->data == "NoBrush") {
		brush.setStyle(Qt::NoBrush);
	    } else if (n->first->data == "SolidPattern") {
		brush.setStyle(Qt::SolidPattern);
	    } else if (n->first->data == "Dense1Pattern") {
		brush.setStyle(Qt::Dense1Pattern);
	    } else if (n->first->data == "Dense2Pattern") {
		brush.setStyle(Qt::Dense2Pattern);
	    } else if (n->first->data == "Dense3Pattern") {
		brush.setStyle(Qt::Dense3Pattern);
	    } else if (n->first->data == "Dense4Pattern") {
		brush.setStyle(Qt::Dense4Pattern);
	    } else if (n->first->data == "Dense5Pattern") {
		brush.setStyle(Qt::Dense5Pattern);
	    } else if (n->first->data == "Dense6Pattern") {
		brush.setStyle(Qt::Dense6Pattern);
	    } else if (n->first->data == "Dense7Pattern") {
		brush.setStyle(Qt::Dense7Pattern);
	    } else if (n->first->data == "HorPattern") {
		brush.setStyle(Qt::HorPattern);
	    } else if (n->first->data == "VerPattern") {
		brush.setStyle(Qt::VerPattern);
	    } else if (n->first->data == "CrossPattern") {
		brush.setStyle(Qt::CrossPattern);
	    } else if (n->first->data == "BDiagPattern") {
		brush.setStyle(Qt::BDiagPattern);
	    } else if (n->first->data == "FDiagPattern") {
		brush.setStyle(Qt::FDiagPattern);
	    } else if (n->first->data == "DiagCrossPattern") {
		brush.setStyle(Qt::DiagCrossPattern);
	    }
	}

	n = n->next;
    }

    return brush;
}

static inline void writeBrush(QSettingsNode *e, const QBrush &brush)
{
    e->tagName = "brush";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    writeColor(n, brush.color());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "brushstyle";

    switch (brush.style()) {
    case Qt::NoBrush:
	n->data = "NoBrush";
	break;

    case Qt::SolidPattern:
	n->data = "SolidPattern";
	break;

    case Qt::Dense1Pattern:
	n->data = "Dense1Pattern";
	break;

    case Qt::Dense2Pattern:
	n->data = "Dense2Pattern";
	break;

    case Qt::Dense3Pattern:
	n->data = "Dense3Pattern";
	break;

    case Qt::Dense4Pattern:
	n->data = "Dense4Pattern";
    	break;

    case Qt::Dense5Pattern:
	n->data = "Dense5Pattern";
	break;

    case Qt::Dense6Pattern:
	n->data = "Dense6Pattern";
	break;

    case Qt::Dense7Pattern:
	n->data = "Dense7Pattern";
	break;

    case Qt::HorPattern:
	n->data = "HorPattern";
	break;

    case Qt::VerPattern:
	n->data = "VerPattern";
	break;

    case Qt::CrossPattern:
	n->data = "CrossPattern";
	break;

    case Qt::BDiagPattern:
	n->data = "BDiagPattern";
	break;

    case Qt::FDiagPattern:
	n->data = "FDiagPattern";
	break;

    case Qt::DiagCrossPattern:
	n->data = "DiagCrossPattern";
	break;

    case Qt::CustomPattern:
	if (brush.pixmap() && ! brush.pixmap()->isNull())
	    writePixmap(n, *brush.pixmap());
	else
	    n->data = "SolidPattern";

	break;
    }
}


// QIconSet
static inline QIconSet readIconSet(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QIconSet iconset;
    int mode = 0, size = 0;

    while (n) {
	if (n->tagName == "pixmap") {
	    QString sizeAttrib = n->first->attributes["size"];
	    QString modeAttrib = n->first->attributes["mode"];

	    if (sizeAttrib == "Large") {
		size = QIconSet::Large;
	    } else if (sizeAttrib == "Small") {
		size = QIconSet::Small;
	    }

	    if (modeAttrib == "Active") {
		mode = QIconSet::Active;
	    } else if (modeAttrib == "Disabled") {
		mode = QIconSet::Disabled;
	    } else if (modeAttrib == "Normal") {
		mode = QIconSet::Normal;
	    }

	    iconset.setPixmap(readPixmap(n), (QIconSet::Size) size,
			      (QIconSet::Mode) mode);
	}

	n = n->next;
    }

    return iconset;
}

static inline void writeIconSet(QSettingsNode *e, const QIconSet &iconset)
{
    e->tagName = "iconset";

    // Large
    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    n->attributes["size"] = "Large";
    n->attributes["mode"] = "Active";
    writePixmap(n, iconset.pixmap(QIconSet::Large, QIconSet::Active));

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;

    n->attributes["size"] = "Large";
    n->attributes["mode"] = "Normal";
    writePixmap(n, iconset.pixmap(QIconSet::Large, QIconSet::Normal));

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;

    n->attributes["size"] = "Large";
    n->attributes["mode"] = "Disabled";
    writePixmap(n, iconset.pixmap(QIconSet::Large, QIconSet::Disabled));

    // Small
    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;

    n->attributes["size"] = "Small";
    n->attributes["mode"] = "Active";
    writePixmap(n, iconset.pixmap(QIconSet::Small, QIconSet::Active));

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;

    n->attributes["size"] = "Small";
    n->attributes["mode"] = "Normal";
    writePixmap(n, iconset.pixmap(QIconSet::Small, QIconSet::Normal));

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;

    n->attributes["size"] = "Small";
    n->attributes["mode"] = "Disabled";
    writePixmap(n, iconset.pixmap(QIconSet::Small, QIconSet::Disabled));
}


// QImage
static inline QImage readImage(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QSize sz;
    int depth = 0;
    QByteArray data;

    while (n) {
	if (n->tagName == "size") {
	    sz = readSize(n);
	} else if (n->tagName == "depth") {
	    depth = n->first->data.toInt();
	} else if (n->tagName == "data") {
	    data.duplicate(n->first->data.utf8(),
			   n->first->data.length());
	}

	n = n->next;
    }

    QImage image;
    if (! data.isNull() &&
	depth > 0 &&
	sz.isValid() && ! sz.isNull()) {
	QByteArray imgdata = decodeBase64(data);


	if (! imgdata.isNull()) {
	    QImage img((uchar *) data.data(), sz.width(), sz.height(), depth,
		       0, 0, QImage::systemByteOrder());
	    image = img;
	    image.detach();
	}
    }

    return image;
}

static inline void writeImage(QSettingsNode *e, const QImage &img)
{
    e->tagName = "image";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    writeSize(n, img.size());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "depth";
    n->data = QString::number(img.depth());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "data";

    QByteArray ba;
    ba.duplicate((const char *) img.bits(),
		 img.width() * img.height() * (img.depth() / 8));
    n->data = "\n" + QString::fromLocal8Bit(encodeBase64(ba).data());
}


// QPointArray
static inline QPointArray readPointArray(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QPointArray array;

    unsigned int i = 0;
    while (n) {
	if (n->tagName == "count")
	    array.resize(n->first->data.toUInt());
	else if (n->tagName == "point")
	    array[(int)i++] = readPoint(n);

	n = n->next;
    }

    return array;
}

static inline void writePointArray(QSettingsNode *e, const QPointArray &array)
{
    e->tagName = "pointarray";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    for (unsigned int i = 0; i < array.count(); i++) {
	writePoint(n, array[(int)i]);

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// QRegion
static inline QRegion readRegion(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QRegion region;

    while (n) {
	if (n->tagName == "rect") {
	    if (region.isNull())
		region = QRegion(readRect(n));
	    else
		region.unite(QRegion(readRect(n)));
	}

	n = n->next;
    }

    return region;
}

static inline void writeRegion(QSettingsNode *e, const QRegion &region)
{
    e->tagName = "region";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    for (uint i = 0; i < region.rects().count(); i++) {
	writeRect(n, region.rects()[(int)i]);

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// QBitmap
static inline QBitmap readBitmap(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QSize sz;
    QByteArray data;

    while (n) {
	if (n->tagName == "size") {
	    sz = readSize(n);
	} else if (n->tagName == "data") {
	    data.duplicate(n->first->data.utf8(),
			   n->first->data.length());
	}

	n = n->next;
    }

    QBitmap bitmap;
    if (! data.isNull()) {
	QByteArray imgdata = decodeBase64(data);

	if (! imgdata.isNull()) {
	    QImage img((uchar *) data.data(), sz.width(), sz.height(), 1,
		       0, 0, QImage::systemByteOrder());
	    bitmap = img;
	}
    }

    return bitmap;
}

static inline void writeBitmap(QSettingsNode *e, const QBitmap &bitmap)
{
    e->tagName = "bitmap";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    writeSize(n, bitmap.size());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "data";

    QImage img;
    img = bitmap;
    QByteArray ba;
    ba.duplicate((const char *) img.bits(), img.width() * img.height());
    n->data = "\n" + QString::fromLocal8Bit(encodeBase64(ba).data());
}


// QCursor
static inline QCursor readCursor(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QBitmap bitmap, mask;
    QPoint hotSpot(-1, -1);
    int shape = -1;

    while (n) {
	if (n->tagName == "bitmap") {
	    QString maskAttrib = n->first->attributes["mask"];

	    if (maskAttrib == "true") {
		mask = readBitmap(n);
	    } else {
		bitmap = readBitmap(n);
	    }
	} else if (n->tagName == "shape") {
	    if (n->data == "ArrowCursor") {
		shape = Qt::ArrowCursor;
	    } else if (n->data == "UpArrowCursor") {
		shape = Qt::UpArrowCursor;
	    } else if (n->data == "CrossCursor") {
		shape = Qt::CrossCursor;
	    } else if (n->data == "WaitCursor") {
		shape = Qt::WaitCursor;
	    } else if (n->data == "IbeamCursor") {
		shape = Qt::IbeamCursor;
	    } else if (n->data == "SizeVerCursor") {
		shape = Qt::SizeVerCursor;
	    } else if (n->data == "SizeHorCursor") {
		shape = Qt::SizeHorCursor;
	    } else if (n->data == "SizeBDiagCursor") {
		shape = Qt::SizeBDiagCursor;
	    } else if (n->data == "SizeFDiagCursor") {
		shape = Qt::SizeFDiagCursor;
	    } else if (n->data == "SizeAllCursor") {
		shape = Qt::SizeAllCursor;
	    } else if (n->data == "BlankCursor") {
		shape = Qt::BlankCursor;
	    } else if (n->data == "SplitVCursor") {
		shape = Qt::SplitVCursor;
	    } else if (n->data == "SplitHCursor") {
		shape = Qt::SplitHCursor;
	    } else if (n->data == "PointingHandCursor") {
		shape = Qt::PointingHandCursor;
	    } else if (n->data == "ForbiddenCursor") {
		shape = Qt::ForbiddenCursor;
	    }
	} else if (n->tagName == "hotspot") {
	    hotSpot = readPoint(n);
	}

	n = n->next;
    }

    if (shape == -1) {
	return QCursor(bitmap, mask, hotSpot.x(), hotSpot.y());
    }

    return QCursor(shape);
}

static inline void writeCursor(QSettingsNode *e, const QCursor &cursor)
{
    e->tagName = "cursor";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    n->tagName = "shape";

    switch (cursor.shape()) {
    case Qt::ArrowCursor:
	n->data = "ArrowCursor";
	break;

    case Qt::UpArrowCursor:
	n->data = "UpArrowCursor";
	break;

    case Qt::CrossCursor:
	n->data = "CrossCursor";
	break;

    case Qt::WaitCursor:
	n->data = "WaitCursor";
	break;

    case Qt:: IbeamCursor:
	n->data = "IbeamCursor";
	break;

    case Qt::SizeVerCursor:
	n->data = "SizeVerCursor";
	break;

    case Qt::SizeHorCursor:
	n->data = "SizeHorCursor";
	break;

    case Qt::SizeBDiagCursor:
	n->data = "SizeBDiagCursor";
	break;

    case Qt::SizeFDiagCursor:
	n->data = "SizeFDiagCursor";
	break;

    case Qt::SizeAllCursor:
	n->data = "SizeAllCursor";
	break;

    case Qt::BlankCursor:
	n->data = "BlankCursor";
	break;

    case Qt::SplitVCursor:
	n->data = "SplitVCursor";
	break;

    case Qt::SplitHCursor:
	n->data = "SplitHCursor";
	break;

    case Qt::PointingHandCursor:
	n->data = "PointingHandCursor";
	break;

    case Qt::ForbiddenCursor:
	n->data = "ForbiddenCursor";
	break;

    case Qt::BitmapCursor:
	if (cursor.bitmap() &&
	    ! cursor.bitmap()->isNull()) {
	    writeBitmap(n, *cursor.bitmap());

	    if (cursor.mask() &&
		! cursor.mask()->isNull()) {
		if (! n->next)
		    e->addChild(new QSettingsNode);
		n = n->next;

		writeBitmap(n, *cursor.mask());
		n->attributes["mask"] = "true";
	    }

	    if (! n->next)
		e->addChild(new QSettingsNode);
	    n = n->next;
	    writePoint(n, cursor.hotSpot());
	} else
	    n->data = "ArrowCursor";

	break;
    }
}


// QSizePolicy
static inline QSizePolicy readSizePolicy(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QSizePolicy sizepolicy;

    while (n) {
	if (n->tagName == "hordata") {
	    if (n->data == "Fixed")
		sizepolicy.setHorData(QSizePolicy::Fixed);
	    else if (n->data == "Minimum")
		sizepolicy.setHorData(QSizePolicy::Minimum);
	    else if (n->data == "Maximum")
		sizepolicy.setHorData(QSizePolicy::Maximum);
	    else if (n->data == "Preferred")
		sizepolicy.setHorData(QSizePolicy::Preferred);
	    else if (n->data == "Expanding")
		sizepolicy.setHorData(QSizePolicy::Expanding);
	    else if (n->data == "MinimumExpanding")
		sizepolicy.setHorData(QSizePolicy::MinimumExpanding);
	} else if (n->tagName == "verdata") {
	    if (n->data == "Fixed")
		sizepolicy.setVerData(QSizePolicy::Fixed);
	    else if (n->data == "Minimum")
		sizepolicy.setVerData(QSizePolicy::Minimum);
	    else if (n->data == "Maximum")
		sizepolicy.setVerData(QSizePolicy::Maximum);
	    else if (n->data == "Preferred")
		sizepolicy.setVerData(QSizePolicy::Preferred);
	    else if (n->data == "Expanding")
		sizepolicy.setVerData(QSizePolicy::Expanding);
	    else if (n->data == "MinimumExpanding")
		sizepolicy.setVerData(QSizePolicy::MinimumExpanding);
	}

	n = n->next;
    }

    return sizepolicy;
}

static inline void writeSizePolicy(QSettingsNode *e, const QSizePolicy &sizepolicy)
{
    e->tagName = "sizepolicy";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    n->tagName = "hordata";

    switch (sizepolicy.horData()) {
    case QSizePolicy::Fixed:
	n->data = "Fixed";
	break;

    case QSizePolicy::Minimum:
	n->data = "Minimum";
	break;

    case QSizePolicy::Maximum:
	n->data = "Maximum";
	break;

    case QSizePolicy::Preferred:
	n->data = "Preferred";
	break;

    default:
    case QSizePolicy::MinimumExpanding:
	n->data = "MinimumExpanding";
	break;

    case QSizePolicy::Expanding:
	n->data = "Expanding";
	break;
    }

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "verdata";

    switch (sizepolicy.verData()) {
    case QSizePolicy::Fixed:
	n->data = "Fixed";
	break;

    case QSizePolicy::Minimum:
	n->data = "Minimum";
	break;

    case QSizePolicy::Maximum:
	n->data = "Maximum";
	break;

    case QSizePolicy::Preferred:
	n->data = "Preferred";
	break;

    default:
    case QSizePolicy::MinimumExpanding:
	n->data = "MinimumExpanding";
	break;

    case QSizePolicy::Expanding:
	n->data = "Expanding";
	break;
    }
}


// QValueList<QString,QVariant>
static QMap<QString,QVariant> readMap(QSettingsNode *e);
static void writeMap(QSettingsNode *e, const QMap<QString,QVariant> &map);

static QValueList<QVariant> readList(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QValueList<QVariant> list;

    while (n) {
	if (n->tagName == "map") {
	    list.append((readMap(n)));
	} else if (n->tagName == "list") {
	    list.append((readList(n)));
	} else if (n->tagName == "string") {
	    list.append((readString(n)));
	} else if (n->tagName == "stringlist") {
	    list.append((readStringList(n)));
	} else if (n->tagName == "font") {
	    list.append((readFont(n)));
	} else if (n->tagName == "brush") {
	    list.append((readBrush(n)));
	} else if (n->tagName == "pixmap") {
	    list.append((readPixmap(n)));
	} else if (n->tagName == "rect") {
	    list.append((readRect(n)));
	} else if (n->tagName == "size") {
	    list.append((readSize(n)));
	} else if (n->tagName == "color") {
	    list.append((readColor(n)));
	} else if (n->tagName == "palette") {
	    list.append((readPalette(n)));
	} else if (n->tagName == "colorgroup") {
	    list.append((readColorGroup(n)));
	} else if (n->tagName == "iconset") {
	    list.append((readIconSet(n)));
	} else if (n->tagName == "point") {
	    list.append((readPoint(n)));
	} else if (n->tagName == "image") {
	    list.append((readImage(n)));
	} else if (n->tagName == "int") {
	    list.append((readInt(n)));
	} else if (n->tagName == "uint") {
	    list.append((readUInt(n)));
	} else if (n->tagName == "bool") {
	    list.append((readBool(n)));
	} else if (n->tagName == "double") {
	    list.append((readDouble(n)));
	} else if (n->tagName == "cstring") {
	    list.append((readCString(n)));
	} else if (n->tagName == "pointarray") {
	    list.append((readPointArray(n)));
	} else if (n->tagName == "region") {
	    list.append((readRegion(n)));
	} else if (n->tagName == "bitmap") {
	    list.append((readBitmap(n)));
	} else if (n->tagName == "cursor") {
	    list.append((readCursor(n)));
	} else if (n->tagName == "sizepolicy") {
	    list.append((readSizePolicy(n)));
	}

	n = n->next;
    }

    return list;
}

static void writeList(QSettingsNode *e, const QValueList<QVariant> &list)
{
    e->tagName = "list";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    QVariant value;
    QValueList<QVariant>::ConstIterator it = list.begin();

    while (it != list.end()) {
	value = (*it);
	++it;

	switch (value.type()) {
	case QVariant::Color:
	    writeColor(n, value.toColor());
	    break;

	case QVariant::CString:
	    writeCString(n, value.toCString());
	    break;

	case QVariant::Point:
	    writePoint(n, value.toPoint());
	    break;

	case QVariant::Size:
	    writeSize(n, value.toSize());
	    break;

	case QVariant::Font:
	    writeFont(n, value.toFont());
	    break;

	case QVariant::Pixmap:
	    writePixmap(n, value.toPixmap());
	    break;

	case QVariant::Brush:
	    writeBrush(n, value.toBrush());
	    break;

	case QVariant::Rect:
	    writeRect(n, value.toRect());
	    break;

	case QVariant::Palette:
	    writePalette(n, value.toPalette());
	    break;

	case QVariant::ColorGroup:
	    writeColorGroup(n, value.toColorGroup());
	    break;

	case QVariant::IconSet:
	    writeIconSet(n, value.toIconSet());
	    break;

	case QVariant::Image:
	    writeImage(n, value.toImage());
	    break;

	case QVariant::Int:
	    writeInt(n, value.toInt());
	    break;

	case QVariant::UInt:
	    writeUInt(n, value.toUInt());
	    break;

	case QVariant::Bool:
	    writeBool(n, value.toBool());
	    break;

	case QVariant::Double:
	    writeDouble(n, value.toDouble());
	    break;

	case QVariant::PointArray:
	    writePointArray(n, value.toPointArray());
	    break;

	case QVariant::Region:
	    writeRegion(n, value.toRegion());
	    break;

	case QVariant::Bitmap:
	    writeBitmap(n, value.toBitmap());
	    break;

	case QVariant::Cursor:
	    writeCursor(n, value.toCursor());
	    break;

	case QVariant::List:
	    writeList(n, value.toList());
	    break;

	case QVariant::StringList:
	    writeStringList(n, value.toStringList());
	    break;

	case QVariant::Map:
	    writeMap(n, value.toMap());
	    break;

	case QVariant::SizePolicy:
	    writeSizePolicy(n, value.toSizePolicy());
	    break;

	case QVariant::String:
	default:
	    writeString(n, value.toString());
	}

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// QMap<QString,QVariant>
static QMap<QString,QVariant> readMap(QSettingsNode *e)
{
    QSettingsNode *n = e->first;
    QMap<QString,QVariant> map;

    while (n) {
	QString key = n->first->attributes["key"];

	if (key.isNull()) continue;

	if (n->tagName == "map") {
	    map[key] = ((readMap(n)));
	} else if (n->tagName == "list") {
	    map[key] = ((readList(n)));
	} else if (n->tagName == "string") {
	    map[key] = ((readString(n)));
	} else if (n->tagName == "stringlist") {
	    map[key] = ((readStringList(n)));
	} else if (n->tagName == "font") {
	    map[key] = ((readFont(n)));
	} else if (n->tagName == "brush") {
	    map[key] = ((readBrush(n)));
	} else if (n->tagName == "pixmap") {
	    map[key] = ((readPixmap(n)));
	} else if (n->tagName == "rect") {
	    map[key] = ((readRect(n)));
	} else if (n->tagName == "size") {
	    map[key] = ((readSize(n)));
	} else if (n->tagName == "color") {
	    map[key] = ((readColor(n)));
	} else if (n->tagName == "palette") {
	    map[key] = ((readPalette(n)));
	} else if (n->tagName == "colorgroup") {
	    map[key] = ((readColorGroup(n)));
	} else if (n->tagName == "iconset") {
	    map[key] = ((readIconSet(n)));
	} else if (n->tagName == "point") {
	    map[key] = ((readPoint(n)));
	} else if (n->tagName == "image") {
	    map[key] = ((readImage(n)));
	} else if (n->tagName == "int") {
	    map[key] = ((readInt(n)));
	} else if (n->tagName == "uint") {
	    map[key] = ((readUInt(n)));
	} else if (n->tagName == "bool") {
	    map[key] = ((readBool(n)));
	} else if (n->tagName == "double") {
	    map[key] = ((readDouble(n)));
	} else if (n->tagName == "cstring") {
	    map[key] = ((readCString(n)));
	} else if (n->tagName == "pointarray") {
	    map[key] = ((readPointArray(n)));
	} else if (n->tagName == "region") {
	    map[key] = ((readRegion(n)));
	} else if (n->tagName == "bitmap") {
	    map[key] = ((readBitmap(n)));
	} else if (n->tagName == "cursor") {
	    map[key] = ((readCursor(n)));
	} else if (n->tagName == "sizepolicy") {
	    map[key] = ((readSizePolicy(n)));
	}

	n = n->next;
    }

    return map;
}

static void writeMap(QSettingsNode *e, const QMap<QString,QVariant> &map)
{
    e->tagName = "map";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    QVariant value;
    QMap<QString,QVariant>::ConstIterator it = map.begin();

    while (it != map.end()) {
	n->attributes["key"] = it.key();
	value = (*it);
	++it;

	switch (value.type()) {
	case QVariant::Color:
	    writeColor(n, value.toColor());
	    break;

	case QVariant::CString:
	    writeCString(n, value.toCString());
	    break;

	case QVariant::Point:
	    writePoint(n, value.toPoint());
	    break;

	case QVariant::Size:
	    writeSize(n, value.toSize());
	    break;

	case QVariant::Font:
	    writeFont(n, value.toFont());
	    break;

	case QVariant::Pixmap:
	    writePixmap(n, value.toPixmap());
	    break;

	case QVariant::Brush:
	    writeBrush(n, value.toBrush());
	    break;

	case QVariant::Rect:
	    writeRect(n, value.toRect());
	    break;

	case QVariant::Palette:
	    writePalette(n, value.toPalette());
	    break;

	case QVariant::ColorGroup:
	    writeColorGroup(n, value.toColorGroup());
	    break;

	case QVariant::IconSet:
	    writeIconSet(n, value.toIconSet());
	    break;

	case QVariant::Image:
	    writeImage(n, value.toImage());
	    break;

	case QVariant::Int:
	    writeInt(n, value.toInt());
	    break;

	case QVariant::UInt:
	    writeUInt(n, value.toUInt());
	    break;

	case QVariant::Bool:
	    writeBool(n, value.toBool());
	    break;

	case QVariant::Double:
	    writeDouble(n, value.toDouble());
	    break;

	case QVariant::PointArray:
	    writePointArray(n, value.toPointArray());
	    break;

	case QVariant::Region:
	    writeRegion(n, value.toRegion());
	    break;

	case QVariant::Bitmap:
	    writeBitmap(n, value.toBitmap());
	    break;

	case QVariant::Cursor:
	    writeCursor(n, value.toCursor());
	    break;

	case QVariant::List:
	    writeList(n, value.toList());
	    break;

	case QVariant::StringList:
	    writeStringList(n, value.toStringList());
	    break;

	case QVariant::Map:
	    writeMap(n, value.toMap());
	    break;

	case QVariant::SizePolicy:
	    writeSizePolicy(n, value.toSizePolicy());
	    break;

	case QVariant::String:
	default:
	    writeString(n, value.toString());
	}

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// file loading
static inline QSettingsNode *load(QIODevice *device)
{
    QTextStream ts(device);
    QXmlInputSource inputsource(ts);
    QXmlSimpleReader reader;

    QSettingsXmlHandler handler;

    reader.
	setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData",
		   FALSE);
    reader.setContentHandler(&handler);

    reader.parse(inputsource);

    return handler.tree;
}

static inline QSettingsNode *load(const QString &filename)
{
    if (filename.isEmpty()) {
	return 0;
    }

    QSettingsNode *n = 0;
    QFile f(filename);
    if (f.open(IO_ReadOnly)) {
	n = load(&f);
	f.close();
    }

    return n;
}

// key checking
static inline bool validKey(const QString &key) {
    return (key[0] == '/' &&
	    key[(int)key.length() - 1] != '/' &&
	    ! key.contains("//"));
}




// **********************************************************************
// QSettingsPrivate
// **********************************************************************

class QSettingsPrivate
{
public:
    QSettingsPrivate(bool w, QSettings *f):
	writable(w), modified(FALSE), fallback(f), node(0), tree(0)
    {
    }

    ~QSettingsPrivate()
    {
	if (tree) {
	    delete tree;
	}
    }

    bool writable, modified;
    QSettings *fallback;
    QSettingsNode *node, *tree;
    QMap<int, QString> pathMap;
};




// **********************************************************************
// QSettings member functions
// **********************************************************************

/*!
  Construct a QSettings object.

  The object is modifiable if \a writable is specified and TRUE (the
  default is FALSE), and uses \a fallback as read fallback if \a
  fallback is specified and non-null (the default is null).

  \sa setWritable(), setFallback()
 */
QSettings::QSettings( bool writable, QSettings *fallback )
{
    d = new QSettingsPrivate(writable, fallback);
    Q_CHECK_PTR(d);
}


/*!  Destroys the QSettings object. Note that this does not write any
  changes to path().
*/
QSettings::~QSettings() {
    delete d;
}


/*!  Returns TRUE if modifications to the settings are allowed (with
  removeEntry() and writeEntry()), and FALSE otherwise.

  \sa setWritable()
*/
bool QSettings::writable() const {
    return d->writable;
}


/*! Makes the object's settings writable if \a writable is TRUE, or
  read-only if \a writable is FALSE. Initially the object is
  read-only.

  \sa writable()
*/
void QSettings::setWritable( bool writable ) {
    d->writable = writable;
}


/*!  Returns the fallback settings for the object, or a null pointer
  if there is no fallback set.

  \sa setFallback(), readEntry()
*/
const QSettings *QSettings::fallback() const {
    return (const QSettings *) d->fallback;
}


/*!  Sets the fallback settings object to \a fallback, or removes any
  fallback if \a fallback is null.

  \sa fallback(), readEntry()
*/
void QSettings::setFallback( QSettings *fallback ) {
    d->fallback = fallback;
}


/*!  Returns the file name where settings are stored when the
  application runs on \a system.

  \sa setPath()
*/
const QString &QSettings::path( System system ) const {
    return d->pathMap[system];
}


/*! Reads settings from \a dev, replacing all previous data.  This function
  is useful for reading data other than from the disk.  This function will replace
  all previous data in this object.

  If \a dev is null, this function does nothing, and leaves previous data
  unmodified.
*/
/*
  void QSettings::setRawContent(QIODevice *dev)
  {
  if (! dev) {
  return;
  }

  if (d->tree) {
  delete d->tree;
  }

  d->tree = load(dev);
  }
*/


/*!
  Set the file name where the settings are stored for the specified
  \a system to \a path.  Since Qt supports multiple operating systems,
  this function must be called for each operating system you intend to
  support.

  For example:

  \code
    QSettings s;
    // ...
    s.setPath( QSettings::Unix, "/opt/mysoft/etc/myapp.rc" );
    s.setPath( QSettings::Windows,"C:\\Program Files\\MySoft\\MyApp.rc" );
  \endcode

  As Qt is ported to more operating systems, the System enum will be
  extended to cover these systems.  Including support in your
  application will only require the addition of another setPath()
  call.

  There is no default path; if your application has not set a path for
  a given system, settings will never be written to disk on that system.

  \sa path() System
 */
void QSettings::setPath( System system, const QString &path )
{
    d->pathMap[system] = path;
}



// ### document error cases. may also need an error enum or
// similar. document lack of transaction semantics.

/*!
  Writes the settings to the location returned by QSettings::path().
  This function returns TRUE if the settings were successfully written
  to disk, and FALSE if an error occurred.

  If the disk is full when writing the settings, a partial file will
  be output.  QSettings will load as much of the partial file as the
  QXml parser will allow.

  Qt never calls this function. We recommend that you call it before
  your application exits, deletes the QSettings object or similar.

  \sa path(), setPath()
*/
bool QSettings::write()
{
    if (! d->modified) {
	return TRUE;
    }

    QString filename;

#ifdef Q_OS_UNIX
    filename = d->pathMap[Unix];
#endif // Q_OS_UNIX

    if (filename.isNull()) {

#  ifdef Q_CHECK_STATE
	qWarning("QSettings::write: path not set");
#  endif // Q_CHECK_STATE

	return FALSE;
    }

    if (! d->tree) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::write: settings never loaded! assuming no "
		 "change (success)");
#endif // Q_CHECK_STATE

	return TRUE;
    }

    QFile file(filename);
    if (! file.open(IO_WriteOnly)) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::write: failed to open file for writing");
#endif // Q_CHECK_STATE

	return FALSE;
    }

    QTextStream ts(&file);
    int level = 1, indent;
    ts << "<!DOCTYPE RC>" << endl;

    QSettingsNode *n = d->tree->first;
    while (level > 0 && n) {
	indent = level;
	while (--indent) ts << " ";

	ts << "<" << n->tagName << n->attrString() << ">";

	if (! n->data.isNull()) ts << n->data << "</" << n->tagName << ">";

	ts << endl;

	if (n->first) {
	    n = n->first;
	    level++;
	} else {
	    while (n && ! n->next) {
		n = n->parent;
		--level;

		if (level > 1) {
		    indent = level;
		    while (--indent) ts << " ";
		    ts << "</" << n->tagName << ">" << endl;
		}
	    }

	    if (n) n = n->next;
	}
    }
    ts << "</RC>" << endl;

    bool success = file.status() == IO_Ok;

    file.close();

    return success;
}


/*!  Writes the entry specified by \a key with \a value, replacing any
  previous setting.

  If the object is not writable, \a value is invalid or there is a
  different error, this function returns FALSE and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QVariant &value)
{
    if (! writable()) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::writeEntry: object is not writable");
#endif // Q_CHECK_STATE

	return FALSE;
    }

    if (! value.isValid()) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::writeEntry: value is not valid");
#endif // Q_CHECK_STATE

	return FALSE;
    }

    if (! validKey(key)) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::writeEntry: malformed key '%s'", key.latin1());
#endif // Q_CHECK_STATE

	return FALSE;
    }

    if (! d->tree) {

#ifdef QSETTINGS_ENTRY_DEBUG
	qDebug("QSettings::writeEntry: doing initial read...");
#endif // QSETTINGS_ENTRY_DEBUG

	QString filename;

#ifdef Q_OS_UNIX
	filename = d->pathMap[Unix];
#endif // Q_OS_UNIX

	if (! filename.isNull()) {
	    d->tree = load(filename);

	    if (d->tree && (d->tree->tagName == "QSettings") &&
		d->tree->first && (d->tree->first->tagName == "RC"))
		d->node = d->tree->first->first;

#ifdef QSETTINGS_ENTRY_DEBUG
	    if (! d->node) {
		qDebug("QSettings::writeEntry: failed to load settings");
	    }
#endif // QSETTINGS_ENTRY_DEBUG
	}

	if (! d->tree) {

#ifdef QSETTINGS_ENTRY_DEBUG
	    qDebug("QSettings::writeEntry: generating empty tree");
#endif // QSETTINGS_ENTRY_DEBUG

	    d->tree = new QSettingsNode;
	    d->tree->tagName = "QSettings";

	    QSettingsNode *n = new QSettingsNode;
	    n->tagName = "RC";
	    d->tree->addChild(n);
	}
    }

    QStringList strlist;
    QString entry;
    {
	int s, p = 0;

	while (p >= 0) {
	    s = p + 1;
	    p = key.find('/', s);

	    if (p > 0) {
		strlist.append(key.mid(s, p - s));
	    }
	}

	entry = key.right(key.length() - s);
    }

    if (! d->node && d->tree->first) {
	d->node = d->tree->first->first;
    }

    QSettingsNode *n = d->node, *p = d->node ? d->node->parent : d->tree->first;
    QStringList::Iterator it = strlist.begin();

    while (it != strlist.end()) {
	while (n) {
	    if (n->tagName == "node" && n->first && n->first->tagName == "key" &&
		n->first->data == (*it)) {
		++it;
		p = n;
		n = n->first;
	    }

	    n = n->next;
	}

	if (it != strlist.end()) {
	    QSettingsNode *c = new QSettingsNode;
	    c->tagName = "node";
	    p->addChild(c);

	    QSettingsNode *k = new QSettingsNode;
	    k->tagName = "key";
	    k->data = (*it);
	    c->addChild(k);

	    n = p->first;
	}
    }

    n = p->first;

    while (n) {
	if (n && n->tagName == "entry") {
	    if (n->first &&
		n->first->tagName == "key" &&
		n->first->data == entry) {
		break;
	    }
	}

	n = n->next;
    }

    if (n) {

#ifdef QSETTINGS_ENTRY_DEBUG
	qDebug("QSettings::writeEntry: replacing entry p %p n %p", p, n);
#endif // QSETTINGS_ENTRY_DEBUG

	p->removeChild(n);
	delete n;
    }

    QSettingsNode *e = new QSettingsNode;
    e->tagName = "entry";
    p->addChild(e);

    QSettingsNode *k = new QSettingsNode;
    k->tagName = "key";
    k->data = entry;
    e->addChild(k);

    QSettingsNode *t = new QSettingsNode;
    e->addChild(t);

    switch (value.type()) {
    case QVariant::Color:
	writeColor(t, value.toColor());
	break;

    case QVariant::CString:
	writeCString(t, value.toCString());
	break;

    case QVariant::Point:
	writePoint(t, value.toPoint());
	break;

    case QVariant::Size:
	writeSize(t, value.toSize());
	break;

    case QVariant::Font:
	writeFont(t, value.toFont());
	break;

    case QVariant::Pixmap:
	writePixmap(t, value.toPixmap());
	break;

    case QVariant::Brush:
	writeBrush(t, value.toBrush());
	break;

    case QVariant::Rect:
	writeRect(t, value.toRect());
	break;

    case QVariant::Palette:
	writePalette(t, value.toPalette());
	break;

    case QVariant::ColorGroup:
	writeColorGroup(t, value.toColorGroup());
	break;

    case QVariant::IconSet:
	writeIconSet(t, value.toIconSet());
	break;

    case QVariant::Image:
	writeImage(t, value.toImage());
	break;

    case QVariant::Int:
	writeInt(t, value.toInt());
	break;

    case QVariant::UInt:
	writeUInt(t, value.toUInt());
	break;

    case QVariant::Bool:
	writeBool(t, value.toBool());
	break;

    case QVariant::Double:
	writeDouble(t, value.toDouble());
	break;

    case QVariant::PointArray:
	writePointArray(t, value.toPointArray());
	break;

    case QVariant::Region:
	writeRegion(t, value.toRegion());
	break;

    case QVariant::Bitmap:
	writeBitmap(t, value.toBitmap());
	break;

    case QVariant::Cursor:
	writeCursor(t, value.toCursor());
	break;

    case QVariant::List:
	writeList(t, value.toList());
	break;

    case QVariant::StringList:
	writeStringList(t, value.toStringList());
	break;

    case QVariant::Map:
	writeMap(t, value.toMap());
	break;

    case QVariant::SizePolicy:
	writeSizePolicy(t, value.toSizePolicy());
	break;

    case QVariant::String:
    default:
	writeString(t, value.toString());
    }

    d->modified = TRUE;

    return TRUE;
}


/*!  Reads the entry specified by \a key, and returns a QVariant
  holding the data.  If \a key does not exist or an error occurs
  readEntry() returns an invalid QVariant.

  \sa writeEntry(), removeEntry(), QVariant::isValid()
*/
QVariant QSettings::readEntry(const QString &key)
{
    if (! validKey(key)) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::readEntry: malformed key '%s'", key.latin1());
#endif // Q_CHECK_STATE

	return QVariant();
    }

    if (! d->node) {
	QString filename;

#ifdef Q_OS_UNIX
	filename = d->pathMap[Unix];
#endif // Q_OS_UNIX

	if (! filename.isNull()) {

#ifdef QSETTINGS_ENTRY_DEBUG
	    qDebug("QSettings::readEntry: doing initial read...");
#endif // QSETTINGS_ENTRY_DEBUG

	    d->tree = load(filename);

	    if (d->tree && (d->tree->tagName == "QSettings") &&
		d->tree->first && (d->tree->first->tagName == "RC"))
		d->node = d->tree->first->first;

#ifdef QSETTINGS_ENTRY_DEBUG
	    if (! d->node) {
		qDebug("QSettings::readEntry: failed to load settings");
	    }
#endif // QSETTINGS_ENTRY_DEBUG

	}
    }

    QStringList strlist;
    QString entry;
    {
	int s, p = 0;

	while (p >= 0) {
	    s = p + 1;
	    p = key.find('/', s);

	    if (p > 0)
		strlist.append(key.mid(s, p - s));
	}

	entry = key.right(key.length() - s);
    }

    QSettingsNode *n = d->node;
    QStringList::Iterator it = strlist.begin();

    while (n && it != strlist.end()) {
	if (n->tagName == "node" &&
	    n->first && n->first->tagName == "key" &&
	    n->first->data == (*it)) {
	    ++it;
	    n = n->first;
	}

	n = n->next;

	if (it == strlist.end() && n) {
	    break;
	}
    }

    if (n && n->tagName == "entry") {
	while (n) {
	    QSettingsNode *c = n->first;

	    if (c->tagName == "key" && c->data == entry) {
		QSettingsNode *d = c->next;

		if (d->tagName == "map") {
		    return ((readMap(d)));
		} else if (d->tagName == "list") {
		    return ((readList(d)));
		} else if (d->tagName == "string") {
		    return ((readString(d)));
		} else if (d->tagName == "stringlist") {
		    return ((readStringList(d)));
		} else if (d->tagName == "font") {
		    return ((readFont(d)));
		} else if (d->tagName == "brush") {
		    return ((readBrush(d)));
		} else if (d->tagName == "pixmap") {
		    return ((readPixmap(d)));
		} else if (d->tagName == "rect") {
		    return ((readRect(d)));
		} else if (d->tagName == "size") {
		    return ((readSize(d)));
		} else if (d->tagName == "color") {
		    return ((readColor(d)));
		} else if (d->tagName == "palette") {
		    return ((readPalette(d)));
		} else if (d->tagName == "colorgroup") {
		    return ((readColorGroup(d)));
		} else if (d->tagName == "iconset") {
		    return ((readIconSet(d)));
		} else if (d->tagName == "point") {
		    return ((readPoint(d)));
		} else if (d->tagName == "image") {
		    return ((readImage(d)));
		} else if (d->tagName == "int") {
		    return ((readInt(d)));
		} else if (d->tagName == "uint") {
		    return ((readUInt(d)));
		} else if (d->tagName == "bool") {
		    return ((readBool(d)));
		} else if (d->tagName == "double") {
		    return ((readDouble(d)));
		} else if (d->tagName == "cstring") {
		    return ((readCString(d)));
		} else if (d->tagName == "pointarray") {
		    return ((readPointArray(d)));
		} else if (d->tagName == "region") {
		    return ((readRegion(d)));
		} else if (d->tagName == "bitmap") {
		    return ((readBitmap(d)));
		} else if (d->tagName == "cursor") {
		    return ((readCursor(d)));
		} else if (d->tagName == "sizepolicy") {
		    return ((readSizePolicy(d)));
		} else {
		    return readString(d);
		}
	    }

	    n = n->next;
	}
    }

    QVariant v;

    if (d->fallback) {
	v = d->fallback->readEntry(key);
    }

    return v;
}


/*!
  Removes the entry specified by \a key.  This function returns TRUE
  if the entry was found and removed and returns FALSE otherwise.

  \sa readEntry(), writeEntry()
*/
bool QSettings::removeEntry(const QString &key)
{
    if (! writable()) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::removeEntry: object not writable");
#endif // Q_CHECK_STATE

	return FALSE;
    }

    if (! validKey(key)) {

#ifdef Q_CHECK_STATE
	qWarning("QSettings::removeEntry: malformed key '%s'", key.latin1());
#endif // Q_CHECK_STATE

	return FALSE;
    }

    if (! d->node) {
	QString filename;

#ifdef Q_OS_UNIX
	filename = d->pathMap[Unix];
#endif // Q_OS_UNIX

	if (! filename.isNull()) {
	    qDebug("QSettings::removeEntry: doing initial read...");

	    d->tree = load(filename);

	    if (d->tree && (d->tree->tagName == "QSettings") &&
		d->tree->first && (d->tree->first->tagName == "RC"))
		d->node = d->tree->first->first;

#ifdef QSETTINGS_ENTRY_DEBUG
	    if (! d->node) {
		qDebug("QSettings::removeEntry: failed to load settings");
	    }
#endif // QSETTINGS_ENTRY_DEBUG

	}
    }

    QStringList strlist;
    QString entry;
    {
	int s, p = 0;

	while (p >= 0) {
	    s = p + 1;
	    p = key.find('/', s);

	    if (p > 0)
		strlist.append(key.mid(s, p - s));
	}

	entry = key.right(key.length() - s);
    }

    QSettingsNode *n = d->node;
    QStringList::Iterator it = strlist.begin();

    while (n && it != strlist.end()) {
	if (n->tagName == "node" &&
	    n->first && n->first->tagName == "key" &&
	    n->first->data == (*it)) {
	    ++it;
	    n = n->first;
	}

	n = n->next;

	if (it == strlist.end() && n) {
	    break;
	}
    }

    while (n) {
	if ((n->tagName == "entry" ||
	     n->tagName == "node") &&
	    (n->first &&
	     n->first->tagName == "key" &&
	     n->first->data == entry)) {

#ifdef QSETTINGS_ENTRY_DEBUG
	    qDebug("QSettings::removeEntry: removed '%s' '%s' ('%s')",
		   (n->tagName + "/" + n->data).latin1(),
		   (n->first->tagName + "/" + n->first->data).latin1(),
		   key.latin1());
#endif // QSETTINGS_ENTRY_DEBUG

	    n->parent->removeChild(n);
	    delete n;
	    d->modified = TRUE;
	    return TRUE;
	}

	n = n->next;
    }

    return FALSE;
}
