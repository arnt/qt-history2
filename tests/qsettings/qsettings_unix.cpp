/****************************************************************************
**
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
#include "qxml.h"

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


QSettingsNode::QSettingsNode()
    : parent(0), prev(0), next(0), first(0), last(0)
{
}


QSettingsNode::~QSettingsNode()
{
    QSettingsNode *n = first, *m;

    while (n) {
	m = n->next;
	delete n;
	n = m;
    }
}


void QSettingsNode::addChild(QSettingsNode *child)
{
    child->parent = this;

    if (last) last->next = child;
    child->prev = last;

    if (! first) first = child;
    last = child;
}


void QSettingsNode::removeChild(QSettingsNode *child)
{
    if (child->parent != this ||
	(! child->next &&
	 ! child->prev &&
	 first != child))
	return;

    if (child->next)
	child->next->prev = child->prev;
    if (child->prev)
	child->prev->next = child->next;

    if (last == child)
	last = child->prev;
    if (first == child)
	first = child->next;

    child->parent = child->prev = child->next = 0;
}


void QSettingsNode::setAttributes(const QXmlAttributes &a)
{
    for (int i = 0; i < a.length(); i++)
	attributes[a.qName(i)] = a.value(i);
}


QString QSettingsNode::attrString() const
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

    return as;
}


class QSettingsXmlHandler : public QXmlDefaultHandler
{
public:
    QSettingsXmlHandler() : node(0) { }

    bool startDocument();
    bool endDocument();
    bool startElement(const QString &ns, const QString &ln, const QString &qName,
		      const QXmlAttributes &attr);
    bool endElement(const QString &ns, const QString &ln, const QString &qName);
    bool characters(const QString &ch);

    QSettingsNode *node, *tree;
};


bool QSettingsXmlHandler::startDocument()
{
    tree = node = new QSettingsNode;
    node->tagName = "QSettings";

    return TRUE;
}


bool QSettingsXmlHandler::endDocument()
{
    if (node != tree)
	return FALSE;

    return TRUE;
}

bool QSettingsXmlHandler::startElement(const QString &, const QString &,
				       const QString &qName, const QXmlAttributes &attr)
{
    QSettingsNode *nnode = new QSettingsNode;
    nnode->setAttributes(attr);
    nnode->tagName = qName;

    node->addChild(nnode);
    node = nnode;

    return TRUE;
}


bool QSettingsXmlHandler::endElement(const QString &, const QString &, const QString &)
{
    if (node == tree)
	return FALSE;

    node = node->parent;
    return TRUE;
}


bool QSettingsXmlHandler::characters(const QString &ch)
{
    node->data += ch;

    return TRUE;
}


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


static QByteArray encodeBase64 (const QByteArray &ina)
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


static QByteArray decodeBase64(const QByteArray &ina)
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


// QPoint

static QPoint readPoint(QSettingsNode *e)
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

static void writePoint(QSettingsNode *e, const QPoint &p)
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

static QFont readFont(QSettingsNode *e)
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

static void writeFont(QSettingsNode *e, const QFont &font)
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

static QRect readRect(QSettingsNode *e)
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

static void writeRect(QSettingsNode *e, const QRect &r)
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

static QCString readCString(QSettingsNode *e)
{
    return e->data.utf8();
}

static void writeCString(QSettingsNode *e, const QCString &s)
{
    e->tagName = "cstring";
    e->data = QString::fromUtf8(s);
}


// QString

static QString readString(QSettingsNode *e)
{
    return e->data;
}

static void writeString(QSettingsNode *e, const QString &s)
{
    e->tagName = "string";
    e->data = s;
}


// DOUBLE

static double readDouble(QSettingsNode *e)
{
    return e->data.toDouble();
}

static void writeDouble(QSettingsNode *e, double d)
{
    e->tagName = "double";
    e->data = QString::number(d, 'g', 10);
}


// int

static int readInt(QSettingsNode *e)
{
    return e->data.toInt();
}

static void writeInt(QSettingsNode *e, int i)
{
    e->tagName = "int";
    e->data = QString::number(i);
}


// uint

static uint readUInt(QSettingsNode *e)
{
    return e->data.toUInt();
}

static void writeUInt(QSettingsNode *e, uint u)
{
    e->tagName = "uint";
    e->data = QString::number(u);
}


// bool

static bool readBool(QSettingsNode *e)
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

static void writeBool(QSettingsNode *e, bool b)
{
    e->tagName = "bool";
    e->data = QString(b ? "true" : "false");
}


// QSize

static QSize readSize(QSettingsNode *e)
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

static void writeSize(QSettingsNode *e, const QSize &s)
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

static QColor readColor(QSettingsNode *e)
{
    int r = 0, g = 0, b = 0;

    while (e && e->first) {
	if (e->tagName == "red") {
	    r = e->first->data.toInt();
	} else if (e->tagName == "green") {
	    g = e->first->data.toInt();
	} else if (e->tagName == "blue") {
	    b = e->first->data.toInt();
	}

	e = e->next;
    }

    return QColor(r, g, b);
}

static void writeColor(QSettingsNode *e, const QColor &c)
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

static QColorGroup readColorGroup(QSettingsNode *e)
{
    QColorGroup cg;

    QSettingsNode *n = e->first;

    for (int i = 0; i < QColorGroup::NColorRoles && n; i++) {
	if (n->tagName == "color") {
	    cg.setColor((QColorGroup::ColorRole) i, readColor(n));
	}

	n = n->next;
    }

    return cg;
}

static void writeColorGroup(QSettingsNode *e, const QColorGroup &cg)
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

static QPalette readPalette(QSettingsNode *e)
{
    QColorGroup a, i, d;

    QSettingsNode *n = e->first;

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

static void writePalette(QSettingsNode *e, const QPalette &p)
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

static QStringList readStringList(QSettingsNode *e)
{
    QStringList list;
    QSettingsNode *n = e->first;

    while (n) {
	if (n->tagName == "string") {
	    list.append(readString(n));
	}

	n = n->next;
    }

    return list;
}

static void writeStringList(QSettingsNode *e, const QStringList &strlist)
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

static QPixmap readPixmap(QSettingsNode *e)
{
    QSize sz;
    int depth = 0;
    QByteArray data;

    QSettingsNode *n = e->first;

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

static void writePixmap(QSettingsNode *e, const QPixmap &pixmap)
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
    ba.duplicate((const char *) img.bits(), img.width() * img.height() * (img.depth() / 8));
    n->data = "\n" + QString::fromLocal8Bit(encodeBase64(ba).data());

    if (! n->next)
	e->addChild(new QSettingsNode);
    n = n->next;
    n->tagName = "depth";
    n->data = QString::number(img.depth());
}


// QBrush

static QBrush readBrush(QSettingsNode *e)
{
    QBrush brush;
    QSettingsNode *n = e->first;

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

static void writeBrush(QSettingsNode *e, const QBrush &brush)
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

static QIconSet readIconSet(QSettingsNode *e)
{
    QIconSet iconset;
    int mode = 0, size = 0;
    QSettingsNode *n = e->first;

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

	    iconset.setPixmap(readPixmap(n), (QIconSet::Size) size, (QIconSet::Mode) mode);
	}

	n = n->next;
    }

    return iconset;
}

static void writeIconSet(QSettingsNode *e, const QIconSet &iconset)
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

static QImage readImage(QSettingsNode *e)
{
    QSize sz;
    int depth = 0;
    QByteArray data;

    QSettingsNode *n = e->first;

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

static void writeImage(QSettingsNode *e, const QImage &img)
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
    ba.duplicate((const char *) img.bits(), img.width() * img.height() * (img.depth() / 8));
    n->data = "\n" + QString::fromLocal8Bit(encodeBase64(ba).data());
}


// QPointArray

static QPointArray readPointArray(QSettingsNode *e)
{
    QPointArray array;
    QSettingsNode *n = e->first;

    unsigned int i = 0;
    while (n) {
	if (n->tagName == "count")
	    array.resize(n->first->data.toUInt());
	else if (n->tagName == "point")
	    array[i++] = readPoint(n);

	n = n->next;
    }

    return array;
}

static void writePointArray(QSettingsNode *e, const QPointArray &array)
{
    e->tagName = "pointarray";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    for (unsigned int i = 0; i < array.count(); i++) {
	writePoint(n, array[i]);

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// QRegion

static QRegion readRegion(QSettingsNode *e)
{
    QRegion region;
    QSettingsNode *n = e->first;

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

static void writeRegion(QSettingsNode *e, const QRegion &region)
{
    e->tagName = "region";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;

    for (uint i = 0; i < region.rects().count(); i++) {
	writeRect(n, region.rects()[i]);

	if (! n->next)
	    e->addChild(new QSettingsNode);
	n = n->next;
    }

    e->removeChild(n);
    delete n;
}


// QBitmap

static QBitmap readBitmap(QSettingsNode *e)
{
    QSize sz;
    QByteArray data;

    QSettingsNode *n = e->first;

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

static void writeBitmap(QSettingsNode *e, const QBitmap &bitmap)
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

static QCursor readCursor(QSettingsNode *e)
{
    QBitmap bitmap, mask;
    QPoint hotSpot(-1, -1);
    int shape = -1;

    QSettingsNode *n = e->first;

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
		shape = ArrowCursor;
	    } else if (n->data == "UpArrowCursor") {
		shape = UpArrowCursor;
	    } else if (n->data == "CrossCursor") {
		shape = CrossCursor;
	    } else if (n->data == "WaitCursor") {
		shape = WaitCursor;
	    } else if (n->data == "IbeamCursor") {
		shape = IbeamCursor;
	    } else if (n->data == "SizeVerCursor") {
		shape = SizeVerCursor;
	    } else if (n->data == "SizeHorCursor") {
		shape = SizeHorCursor;
	    } else if (n->data == "SizeBDiagCursor") {
		shape = SizeBDiagCursor;
	    } else if (n->data == "SizeFDiagCursor") {
		shape = SizeFDiagCursor;
	    } else if (n->data == "SizeAllCursor") {
		shape = SizeAllCursor;
	    } else if (n->data == "BlankCursor") {
		shape = BlankCursor;
	    } else if (n->data == "SplitVCursor") {
		shape = SplitVCursor;
	    } else if (n->data == "SplitHCursor") {
		shape = SplitHCursor;
	    } else if (n->data == "PointingHandCursor") {
		shape = PointingHandCursor;
	    } else if (n->data == "ForbiddenCursor") {
		shape = ForbiddenCursor;
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

static void writeCursor(QSettingsNode *e, const QCursor &cursor)
{
    e->tagName = "cursor";

    if (! e->first)
	e->addChild(new QSettingsNode);
    QSettingsNode *n = e->first;
    n->tagName = "shape";

    switch (cursor.shape()) {
    case ArrowCursor:
	n->data = "ArrowCursor";
	break;

    case UpArrowCursor:
	n->data = "UpArrowCursor";
	break;

    case CrossCursor:
	n->data = "CrossCursor";
	break;

    case WaitCursor:
	n->data = "WaitCursor";
	break;

    case  IbeamCursor:
	n->data = "IbeamCursor";
	break;

    case SizeVerCursor:
	n->data = "SizeVerCursor";
	break;

    case SizeHorCursor:
	n->data = "SizeHorCursor";
	break;

    case SizeBDiagCursor:
	n->data = "SizeBDiagCursor";
	break;

    case SizeFDiagCursor:
	n->data = "SizeFDiagCursor";
	break;

    case SizeAllCursor:
	n->data = "SizeAllCursor";
	break;

    case BlankCursor:
	n->data = "BlankCursor";
	break;

    case SplitVCursor:
	n->data = "SplitVCursor";
	break;

    case SplitHCursor:
	n->data = "SplitHCursor";
	break;

    case PointingHandCursor:
	n->data = "PointingHandCursor";
	break;

    case ForbiddenCursor:
	n->data = "ForbiddenCursor";
	break;

    case BitmapCursor:
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

static QSizePolicy readSizePolicy(QSettingsNode *e)
{
    QSizePolicy sizepolicy;
    QSettingsNode *n = e->first;

    while (n) {
	if (n->tagName == "hordata") {
	    if (n->data == "Fixed") sizepolicy.setHorData(QSizePolicy::Fixed);
	    else if (n->data == "Minimum") sizepolicy.setHorData(QSizePolicy::Minimum);
	    else if (n->data == "Maximum") sizepolicy.setHorData(QSizePolicy::Maximum);
	    else if (n->data == "Preferred") sizepolicy.setHorData(QSizePolicy::Preferred);
	    else if (n->data == "Expanding") sizepolicy.setHorData(QSizePolicy::Expanding);
	    else if (n->data == "MinimumExpanding")
		sizepolicy.setHorData(QSizePolicy::MinimumExpanding);
	} else if (n->tagName == "verdata") {
	    if (n->data == "Fixed") sizepolicy.setVerData(QSizePolicy::Fixed);
	    else if (n->data == "Minimum") sizepolicy.setVerData(QSizePolicy::Minimum);
	    else if (n->data == "Maximum") sizepolicy.setVerData(QSizePolicy::Maximum);
	    else if (n->data == "Preferred") sizepolicy.setVerData(QSizePolicy::Preferred);
	    else if (n->data == "Expanding") sizepolicy.setVerData(QSizePolicy::Expanding);
	    else if (n->data == "MinimumExpanding")
		sizepolicy.setVerData(QSizePolicy::MinimumExpanding);
	}

	n = n->next;
    }

    return sizepolicy;
}

static void writeSizePolicy(QSettingsNode *e, const QSizePolicy &sizepolicy)
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
    QValueList<QVariant> list;
    QSettingsNode *n = e->first;

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
    QMap<QString,QVariant> map;
    QSettingsNode *n = e->first;

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


static QSettingsNode *load(QIODevice *device)
{
    QTextStream ts(device);
    QXmlInputSource inputsource(ts);
    QXmlSimpleReader reader;

    QSettingsXmlHandler handler;

    reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData",
		      FALSE);
    reader.setContentHandler(&handler);
    reader.parse(inputsource);

    return handler.tree;
}


static QSettingsNode *load(const QString &filename)
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


void QSettings::cleanup()
{
     if (tree) delete tree;
}


void QSettings::write()
{
    if (! writable() || pathMap_p[Unix].isNull()) return;
    
    if (! tree) {
	qDebug("QSettings::write: doing initial read...");

	if (! pathMap_p[Unix].isNull()) {
	    tree = load(pathMap_p[Unix]);

	    if (tree && (tree->tagName == "QSettings") &&
		tree->first && (tree->first->tagName == "RC"))
		node = tree->first->first;

	    if (! node) {
		qDebug("QSettings::write: failed to load settings");
	    }
	}

	if (! tree) {
	    qDebug("QSettings::write: generating empty tree");
	    
	    tree = new QSettingsNode;
	    tree->tagName = "QSettings";

	    QSettingsNode *n = new QSettingsNode;
	    n->tagName = "RC";
	    tree->addChild(n);
	}
    }

    QFile file(pathMap_p[Unix]);
    if (! file.open(IO_WriteOnly)) return;
    QTextStream ts(&file);
    int level = 1, indent;
    ts << "<!DOCTYPE RC>" << endl;
    
    
    QSettingsNode *n = tree->first;
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

    file.close();
}


void QSettings::writeEntry(const QString &key, const QVariant &value)
{
    if (! writable()) return;

    if (key[0] != '/' || key[key.length() - 1] == '/') {
	qDebug("QSettings::writeEntry: malformed key '%s'", key.latin1());
	return;
    }

    if (! tree) {
	qDebug("QSettings::writeEntry: doing initial read...");

	if (! pathMap_p[Unix].isNull()) {
	    tree = load(pathMap_p[Unix]);

	    if (tree && (tree->tagName == "QSettings") &&
		tree->first && (tree->first->tagName == "RC"))
		node = tree->first->first;

	    if (! node) {
		qDebug("QSettings::writeEntry: failed to load settings");
	    }
	}

	if (! tree) {
	    qDebug("QSettings::writeEntry: generating empty tree");
	    
	    tree = new QSettingsNode;
	    tree->tagName = "QSettings";

	    QSettingsNode *n = new QSettingsNode;
	    n->tagName = "RC";
	    tree->addChild(n);
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

    if (! node && tree->first) node = tree->first->first;

    QSettingsNode *n = node, *p = node ? node->parent : tree->first;
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

    QSettingsNode *d = 0;

    while (n) {
	if (n && n->tagName == "entry") {
	    if (n->first &&
		n->first->tagName == "key" &&
		n->first->data == entry) {
		d = n->first->next;
		break;
	    }
	}

	n = n->next;
    }

    if (! d) {
	QSettingsNode *e = new QSettingsNode;
	e->tagName = "entry";
	p->addChild(e);

	QSettingsNode *k = new QSettingsNode;
	k->tagName = "key";
	k->data = entry;
	e->addChild(k);

	d = new QSettingsNode;
	e->addChild(d);
    }

    switch (value.type()) {
    case QVariant::Color:
	writeColor(d, value.toColor());
	break;

    case QVariant::CString:
	writeCString(d, value.toCString());
	break;

    case QVariant::Point:
	writePoint(d, value.toPoint());
	break;

    case QVariant::Size:
	writeSize(d, value.toSize());
	break;

    case QVariant::Font:
	writeFont(d, value.toFont());
	break;

    case QVariant::Pixmap:
	writePixmap(d, value.toPixmap());
	break;

    case QVariant::Brush:
	writeBrush(d, value.toBrush());
	break;

    case QVariant::Rect:
	writeRect(d, value.toRect());
	break;

    case QVariant::Palette:
	writePalette(d, value.toPalette());
	break;

    case QVariant::ColorGroup:
	writeColorGroup(d, value.toColorGroup());
	break;

    case QVariant::IconSet:
	writeIconSet(d, value.toIconSet());
	break;

    case QVariant::Image:
	writeImage(d, value.toImage());
	break;

    case QVariant::Int:
	writeInt(d, value.toInt());
	break;

    case QVariant::UInt:
	writeUInt(d, value.toUInt());
	break;

    case QVariant::Bool:
	writeBool(d, value.toBool());
	break;

    case QVariant::Double:
	writeDouble(d, value.toDouble());
	break;

    case QVariant::PointArray:
	writePointArray(d, value.toPointArray());
	break;

    case QVariant::Region:
	writeRegion(d, value.toRegion());
	break;

    case QVariant::Bitmap:
	writeBitmap(d, value.toBitmap());
	break;

    case QVariant::Cursor:
	writeCursor(d, value.toCursor());
	break;

    case QVariant::List:
	writeList(d, value.toList());
	break;

    case QVariant::StringList:
	writeStringList(d, value.toStringList());
	break;

    case QVariant::Map:
	writeMap(d, value.toMap());
	break;

    case QVariant::SizePolicy:
	writeSizePolicy(d, value.toSizePolicy());
	break;

    case QVariant::String:
    default:
	writeString(d, value.toString());
    }
}


QVariant QSettings::readEntry(const QString &key)
{
    if (override_p) {
	QVariant v = override_p->readEntry(key);
	if (v.isValid()) return v;
    }
    
    if (key[0] != '/') {
	qDebug("QSettings::readEntry: malformed key '%s'", key.latin1());
	return QVariant();
    }

    if (! node && ! pathMap_p[Unix].isNull()) {
	qDebug("QSettings::readEntry: doing initial read...");

	tree = load(pathMap_p[Unix]);

	if (tree && (tree->tagName == "QSettings") &&
	    tree->first && (tree->first->tagName == "RC"))
	    node = tree->first->first;

	if (! node)
	    qDebug("QSettings::readEntry: failed to load settings");
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

    QSettingsNode *n = node;
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

    return QVariant();
}


void QSettings::removeEntry(const QString &key)
{
    if (! writable()) return;

    if (key[0] != '/') {
	qDebug("QSettings::removeEntry: malformed key '%s'", key.latin1());
	return;
    }

    if (! node && ! pathMap_p[Unix].isNull()) {
	qDebug("QSettings::removeEntry: doing initial read...");

	tree = load(pathMap_p[Unix]);

	if (tree && (tree->tagName == "QSettings") &&
	    tree->first && (tree->first->tagName == "RC"))
	    node = tree->first->first;

	if (node)
	    qDebug("QSettings::removeEntry: successfully loaded settings");
	else
	    qDebug("QSettings::removeEntry: failed to load settings");
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

    QSettingsNode *n = node;
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
	    qDebug("found '%s' '%s' ('%s')",
		   (n->tagName + "-" + n->data).latin1(),
		   (n->first->tagName + "-" + n->first->data).latin1(),
		   key.latin1());
	    n->parent->removeChild(n);
	    delete n;
	    break;
	}
	
	n = n->next;
    }
}
