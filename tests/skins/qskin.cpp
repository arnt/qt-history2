#include "qskin.h"
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qrect.h>
#include <qdict.h>
#include <qvaluestack.h>
#include <qvaluevector.h>
#include <qptrvector.h>
#include <qtextstream.h>
#include <qptrlist.h>
#include <qxml.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qpalette.h>
#include <qslider.h>
#include <qcolor.h>

class QSkinStyleItem
{
public:
    QSkinStyleItem(QString t, QString n) : name(n), type(t), hasMask(FALSE),
	isDragable(FALSE) {}
    ~QSkinStyleItem() {}

    QString name;
    QString type;

    QRect geom;
    QDict<QPixmap> images;
    QDict<QRect> clips;

    QDict<QPixmap> imageSets;
    QDict< QVector< QRect > > clipSets;

    QValueVector<QPoint> line; /* just one of those things we might need for
				  a widget */

    bool hasMask;
    QBitmap mask;

    QDict<QRect> children;

    bool isDragable;
};

class QSkinStylePrivate
{
public:
    QSkinStylePrivate() : hasColors(FALSE)
    {
        backgroundPixmap = 0;
    }
    QPtrList<QSkinStyleItem> specials;

    // FIXME.  Should only use the 'class-only' item if no class
    // and name item exists.
    QSkinStyleItem * getItem(const QWidget *w) 
    { 
	if(specials.count() > 0) {
	    specials.first();
	    while(specials.current()) {
		if (specials.current()->type == w->className()) {
		    if (!specials.current()->name 
			    || (specials.current()->name == w->name()))
			return specials.current();
		}
		specials.next();
	    }
	}

	return (QSkinStyleItem *)0;
    }

    QSkinStyleItem * getItem(const QLayout *w) 
    { 
	if(specials.count() > 0) {
	    specials.first();
	    while(specials.current()) {
		if (specials.current()->name == w->name())
		    return specials.current();
		specials.next();
	    }
	}

	return (QSkinStyleItem *)0;
    }

    QString name;
    QDict<QPixmap> images; /* unique store.  clean up from here */

    QPixmap *backgroundPixmap;
    QRect backgroundClip;

    bool hasColors;
    QColor bColor;
    QColor fColor;
    QColor tColor;
};

class QSkinStyleHandler : public QXmlDefaultHandler
{
public:
    QSkinStyleHandler();
    ~QSkinStyleHandler();

    QString errorProtocol();

    bool startDocument();
    bool startElement(const QString& namespaceURI, const QString & localName,
	    const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName,
	    const QString &qName);
    bool characters(const QString &ch);

    QString errorString();

    bool warning(const QXmlParseException & exception);
    bool error(const QXmlParseException & exception);
    bool fatalError(const QXmlParseException & exception);

    QSkinStylePrivate *data() const { return d; }
private:

    /* construction variables */
    QSkinStylePrivate *d;
    QSkinStyleItem *i;
    QRect r;

    //Stack ??
    QString errorProt;

    enum State {
	S_Init,
	S_Main,
	S_Widget,
	S_Layout,
	S_Background,
	S_Color,
	S_Red,
	S_Blue,
	S_Green,
	S_ForegroundColor,
	S_BackgroundColor,
	S_TextColor,
	S_Element,
	S_Pos,
	S_Mask,
	S_Pixmap,
	S_PixmapSet,
	S_Offset,
	S_Count,
	S_Source,
	S_Rect,
	S_Size,
	S_X,
	S_Y,
	S_Width,
	S_Height,
    };

    QValueStack<State> state;
};

static int last_x = 0;
static int last_y = 0;
static int previous_x = 0;
static int previous_y = 0;
static int last_width = 0;
static int last_height = 0;
static int count = 0;
static QString last_string;
static QString eName;

QSkinStyleHandler::QSkinStyleHandler() 
{
    d = new QSkinStylePrivate;
}

QSkinStyleHandler::~QSkinStyleHandler() {}

QString QSkinStyleHandler::errorProtocol()
{
    qWarning("Error reading skin");
    return errorProt;
}

bool QSkinStyleHandler::startDocument()
{
    errorProt = "";
    state.push(S_Init); 
    return TRUE;
}

bool QSkinStyleHandler::startElement(const QString &, const QString &, 
	const QString &qName, const QXmlAttributes &atts) 
{
    switch(state.top()) {
	case S_Init:
	    /* only should find <skin> tag */
	    if (qName != "skin") {
		errorProt += "skin tag expected: ";
		return FALSE;
	    }
	    d->name = atts.value("name");
	    state.push(S_Main);
	    break;
	case S_Main:
	    if (qName == "widget") {

		QString name = atts.value("name");
		QString type = atts.value("type");

		if(!name && !type) {
		    errorProt += "missing attributes: ";
		    return FALSE;
		}

		i = new QSkinStyleItem(type, name);

		if(atts.value("dragable") == "true")
		    i->isDragable = TRUE;
		state.push(S_Widget);
		return TRUE;
	    }
	    if (qName == "layout") {
		if(!atts.value("name")) {
		    errorProt += "missing attributes: ";
		    return FALSE;
		}
		i = new QSkinStyleItem("QSkinLayout", atts.value("name"));
		state.push(S_Layout);
		return TRUE;
	    }
	    if (qName == "background") {
	        state.push(S_Background);
		return TRUE;
	    }
	    if (qName == "color") {
		state.push(S_Color);
		return TRUE;
	    }
	    /* not a valid tag */
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Color:
	    if (qName == "button") {
		state.push(S_ForegroundColor);
		return TRUE;
	    }
	    if (qName == "background") {
		state.push(S_BackgroundColor);
		return TRUE;
	    }
	    if (qName == "text") {
		state.push(S_TextColor);
		return TRUE;
	    }
	    /* not a valid tag */
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_TextColor:
	case S_BackgroundColor:
	case S_ForegroundColor:
	    if (qName == "red") {
		state.push(S_Red);
		return TRUE;
	    }
	    if (qName == "green") {
		state.push(S_Green);
		return TRUE;
	    }
	    if (qName == "blue") {
		state.push(S_Blue);
		return TRUE;
	    }
	    /* not a valid tag */
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Background:
	    if (qName == "source") {
		state.push(S_Source);
		return TRUE;
	    }
	    if (qName == "rect") {
		state.push(S_Rect);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Widget:
	    if (qName == "image") {
		eName = atts.value("name");
		if(!eName) {
		    errorProt += "missing attributes: ";
		    return FALSE;
		}
		state.push(S_Pixmap);
		return TRUE;
	    }
	    if (qName == "imageset") {
		eName = atts.value("name");
		if(!eName) {
		    errorProt += "missing attributes: ";
		    return FALSE;
		}
		state.push(S_PixmapSet);
		return TRUE;
	    }
	    if (qName == "mask") {
		state.push(S_Mask);
		return TRUE;
	    }
	    if (qName == "size") {
		state.push(S_Size);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Layout:
	    if (qName == "element") {
		last_string = atts.value("name");
		if (!last_string) {
		    errorProt += "missing attributes: ";
		    return FALSE;
		}
		state.push(S_Element);
		return TRUE;
	    }
	    if (qName == "size") {
		state.push(S_Size);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Element:
	    if (qName == "rect") {
		state.push(S_Rect);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Mask:
	case S_PixmapSet:
	    if (qName == "offset") {
		state.push(S_Offset);
		return TRUE;
	    }
	    if (qName == "count") {
		state.push(S_Count);
		return TRUE;
	    }
	case S_Pixmap:
	    if (qName == "source") {
		state.push(S_Source);
		return TRUE;
	    }
	    if (qName == "rect") {
		state.push(S_Rect);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Rect:
	    if (qName == "width") {
		state.push(S_Width);
		return TRUE;
	    }
	    if (qName == "height") {
		state.push(S_Height);
		return TRUE;
	    }
	case S_Offset:
	case S_Pos:
	    if (qName == "x") {
		state.push(S_X);
		return TRUE;
	    }
	    if (qName == "y") {
		state.push(S_Y);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	case S_Size:
	    if (qName == "width") {
		state.push(S_Width);
		return TRUE;
	    }
	    if (qName == "height") {
		state.push(S_Height);
		return TRUE;
	    }
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
	default:
	    errorProt += "tag " + qName + " not expected: ";
	    return FALSE;
    }
    return TRUE;
}

bool QSkinStyleHandler::endElement(const QString &, const QString &,
	const QString &qName)
{
    switch(state.top()) {
	 case S_Init:
	     errorProt += "invalid end tag found: ";
	     return FALSE;
	 case S_Main:
	     /* done with document */
	     state.pop();
	     break;
	 case S_Widget:
	     /* done with 'i' */
	     d->specials.append(i);
	     state.pop();
	     break;
	 case S_Layout:
	     d->specials.append(i);
	     state.pop();
	     break;
	 case S_Mask:
	     {
		 /* push image and clipping onto 'i' */
		 QPixmap *im = d->images.find(last_string);
		 if(!im) {
		     /* new image */
		     im = new QPixmap(last_string);
		     d->images.insert(last_string, im);
		 }
		 if (im->isNull()) {
		     qWarning(QString("could not find image %1").arg(last_string));
		     return FALSE;
		 }
		 // The next line is slow but only done once per widget
		 i->mask = im->convertToImage().copy(last_x, last_y, last_width, last_height).createAlphaMask();
		 i->hasMask = TRUE;

		 /* re-init for next round */
		 last_x = last_y = last_width = last_height = 0;
		 last_string = QString::null;
		 state.pop();
	     }
	     break;
	 case S_Pixmap: 
	     {
		 /* push image and clipping onto 'i' */
		 QPixmap *im = d->images.find(last_string);
		 if(!im) {
		     /* new image */
		     im = new QPixmap(last_string);
		     d->images.insert(last_string, im);
		 }
		 if (im->isNull()) {
		     qWarning(QString("could not find image %1").arg(last_string));
		     return FALSE;
		 }
		 if (i->images.count() == 0) {
		     i->images.replace("Default", im);
		     i->clips.replace("Defualt", new QRect(last_x, last_y, 
				 last_width, last_height));
		 }
		 i->images.replace(eName, im);
		 i->clips.replace(eName, new QRect(last_x, last_y, 
			     last_width, last_height));

		 /* re-init for next round */
		 last_x = last_y = last_width = last_height = 0;
		 eName = last_string = QString::null;
		 state.pop();
	     }
	     break;
	 case S_PixmapSet:
	     {
		 /* push image and clipping onto 'i' */
		 QPixmap *im = d->images.find(last_string);
		 if(!im) {
		     /* new image */
		     im = new QPixmap(last_string);
		     d->images.insert(last_string, im);
		 }
		 if (im->isNull()) {
		     qWarning(QString("could not find image %1").arg(last_string));
		     return FALSE;
		 }

		 i->imageSets.replace(eName, im);

		 int j;
		 QVector<QRect> *v = new QVector<QRect>(count);
		 for(j = 0; j < count; j++) {
		     v->insert(j, new QRect( previous_x, previous_y, 
				 last_width, last_height));
		     previous_x += last_x;
		     previous_y += last_y;
		 }

		 i->clipSets.replace(eName, v);

		 /* re-init for next round */
		 previous_x = previous_y =last_x = last_y 
		     = last_width = last_height = 0;
		 last_string = QString::null;
		 state.pop();
	     }
	     break;
	 case S_Background: 
	     {
		 /* push image and clipping onto 'i' */
		 QPixmap *im = d->images.find(last_string);
		 if(!im) {
		     /* new image */
		     im = new QPixmap(last_string);
		     d->images.insert(last_string, im);
		 }
		 if (im->isNull()) {
		     qWarning(QString("could not find image %1").arg(last_string));
		     return FALSE;
		 }

		 d->backgroundPixmap = new QPixmap();
		 d->backgroundPixmap->convertFromImage(
			 im->convertToImage()
			 .copy(last_x, last_y, last_width, last_height));

		 d->backgroundClip = QRect(last_x, last_y, 
			 last_width, last_height);
		 /* re-init for next round */
		 last_x = last_y = last_width = last_height = 0;
		 last_string = QString::null;
		 state.pop();
	     }
	     break;
	 case S_ForegroundColor:
	     d->hasColors = TRUE;
	     if (last_string.isEmpty())
		 d->fColor = QColor(last_x, last_y, last_width);
	     else
		 d->fColor = QColor(last_string);

	     last_x = last_y = last_width = last_height = 0;
	     last_string = QString::null;
	     state.pop();
	     break;
	 case S_BackgroundColor:
	     d->hasColors = TRUE;
	     if (last_string.isEmpty())
		 d->bColor = QColor(last_x, last_y, last_width);
	     else
		 d->bColor = QColor(last_string);

	     last_x = last_y = last_width = last_height = 0;
	     last_string = QString::null;
	     state.pop();
	     break;
	 case S_TextColor:
	     d->hasColors = TRUE;
	     if (last_string.isEmpty())
		 d->tColor = QColor(last_x, last_y, last_width);
	     else
		 d->tColor = QColor(last_string);

	     last_x = last_y = last_width = last_height = 0;
	     last_string = QString::null;
	     state.pop();
	     break;
	 case S_Size:
	    state.pop();
	    if ((state.top() == S_Widget) || (state.top() == S_Layout)) {
		/* we just got the size of the widget */
		i->geom = QRect(0, 0, last_width, last_height);
		last_width = last_height = 0;
	    }
	    break;
	 case S_Element:
	    state.pop();
	    i->children.insert(last_string, new QRect(last_x, last_y, 
			last_width, last_height));
	    last_x = last_y = 0;
	    last_string = QString::null;
	    break;
	 case S_Rect:
	    state.pop();
	    if (state.top() == S_PixmapSet) {
		/* need to save the last x and last y */
		previous_x = last_x;
		previous_y = last_y;
	    }
	    break;
	 case S_Color:
	 case S_Offset:
	 case S_Count:
	 case S_Pos:
	 case S_X:
	 case S_Y:
	 case S_Width:
	 case S_Height:
	 case S_Source:
	 case S_Red:
	 case S_Blue:
	 case S_Green:
	    state.pop();
	    break;
    }
    return TRUE;
}

bool QSkinStyleHandler::characters(const QString &ch)
{
    QString simple = ch.simplifyWhiteSpace();

    int tmp = simple.toInt();

    switch(state.top()) {
	case S_Red:
	case S_X:
	    last_x = tmp;
	    break;
	case S_Green:
	case S_Y:
	    last_y = tmp;
	    break;
	case S_Blue:
	case S_Width:
	    last_width = tmp;
	    break;
	case S_Height:
	    last_height = tmp;
	    break;
	case S_Count:
	    count = tmp;
	    break;
	case S_ForegroundColor:
	case S_BackgroundColor:
	case S_Source:
	    last_string = simple;
	    break;
	default:
	    break;
    }
    return TRUE;
}

QString QSkinStyleHandler::errorString()
{
    return "skin syntax error";
}

bool QSkinStyleHandler::warning(const QXmlParseException &e)
{
    errorProt += QString("Warning parsing error: %1 in line %2, column %3")
	.arg(e.message())
	.arg(e.lineNumber())
	.arg(e.columnNumber());

    qWarning(errorProt);
    return QXmlDefaultHandler::warning(e);
}

bool QSkinStyleHandler::error(const QXmlParseException &e)
{
    errorProt += QString("Warning parsing error: %1 in line %2, column %3")
	.arg(e.message())
	.arg(e.lineNumber())
	.arg(e.columnNumber());

    qWarning(errorProt);
    return QXmlDefaultHandler::error(e);
}

bool QSkinStyleHandler::fatalError(const QXmlParseException &e)
{
    errorProt += QString("Warning parsing error: %1 in line %2, column %3")
	.arg(e.message())
	.arg(e.lineNumber())
	.arg(e.columnNumber());

    qWarning(errorProt);
    return QXmlDefaultHandler::fatalError(e);
}

/*********************************************************************
**                                                                  **
**   Start of Style Code                                            **
**                                                                  **
*********************************************************************/


/* later, remove this function */
QSkinStyle::QSkinStyle() : QWindowsStyle()
{
    d = 0;
}

/* later will use this */
QSkinStyle::QSkinStyle( QTextStream &desc ) : QWindowsStyle() 
{
    QSkinStyleHandler h;

    QXmlInputSource source(desc);
    QXmlSimpleReader reader;
    reader.setContentHandler(&h);
    reader.setErrorHandler(&h);

    if(reader.parse(source))
	d = h.data();
    else 
	d = 0;
}

QSkinStyle::~QSkinStyle()
{
}

bool QSkinStyle::eventFilter(QObject *o, QEvent *e) 
{
    static QPoint clickPos;
    static QPoint newPos;

    QMouseEvent *me;
    switch (e->type()) {
	case QEvent::MouseButtonPress:
	    me = (QMouseEvent *)e;
	    clickPos = me->pos();
	    return TRUE;
	case QEvent::MouseMove:
	    me = (QMouseEvent *)e;
	    newPos = me->globalPos() - clickPos;
	    ((QWidget *)o)->move(newPos);
	    return TRUE;
	default:
	    return FALSE;
    }
    return FALSE;
}

void QSkinStyle::setSkin( QTextStream &desc ) {}

QString QSkinStyle::skinName()
{
    return "none";
}

bool QSkinStyle::defined(const QWidget *w) const
{
    if (!d)
	return FALSE;
    if (!d->getItem(w))
	return FALSE;

    return TRUE;
}

bool QSkinStyle::defined(const QLayout *w) const
{
    if (!d)
	return FALSE;
    if (!d->getItem(w))
	return FALSE;

    return TRUE;
}

bool QSkinStyle::defined(const QLayout *l, const QWidget *w) const
{
    if (!defined(l))
	return FALSE;

    if(d->getItem(l)->children.find(w->name()))
	return TRUE;

    return FALSE;
}


QRect QSkinStyle::getGeometry(const QWidget *w) const
{
    if (!d)
	return QRect(0,0,50,20);

    QSkinStyleItem *i = d->getItem(w);

    if(!i)
	return QRect(0,0,50,20);

    return i->geom;
}

QRect QSkinStyle::getGeometry(const QLayout *w) const
{
    if (!d)
	return QRect(0,0,50,20);

    QSkinStyleItem *i = d->getItem(w);

    if(!i)
	return QRect(0,0,50,20);

    return i->geom;
}

QRect QSkinStyle::getGeometry(const QLayout *l, const QWidget *w) const
{
    if(!d)
	return QRect(0,0,50,20);

    QRect r = *(d->getItem(l)->children.find(w->name()));
    return r;
}

int QSkinStyle::countImages(const QWidget *w, const QString &n) const 
{
   if(!defined(w))
       return 0;

    QSkinStyleItem *i = d->getItem(w);
    if(i->clipSets.find(n))
	return i->clipSets.find(n)->count();

    return 0;
}

QPixmap QSkinStyle::image(const QWidget *w, const QString &i) const
{
   if(!defined(w))
       return 0;

   QPixmap retval(d->getItem(w)->images.find(i)->convertToImage().copy(*d->getItem(w)->clips.find(i)));
   return retval;
}

void QSkinStyle::drawImage(QPainter *p, const QWidget *w, const QString &i ) const
{
    if(!defined(w))
	return;

    if (!d->getItem(w)->images.find(i))
	return;

    p->drawPixmap(QPoint(0,0), *d->getItem(w)->images.find(i), 
	    *d->getItem(w)->clips.find(i));
}

void QSkinStyle::drawImage(QPainter *p, const QWidget *w, const QString &n, int ind) const
{
    QSkinStyleItem *i = d->getItem(w);
    if (!i) 
	return;

    if(!i->clipSets.find(n))
	return;

    if (ind < 0 || ind >= i->clipSets.find(n)->count())
	return;

    p->drawPixmap(QPoint(0,0), *i->imageSets.find(n), 
	    *i->clipSets.find(n)->at(ind));
}


/* many flaws, the first of which is ignore all the flags */
void QSkinStyle::drawComplexControl(ComplexControl element,
	    QPainter *p,
	    const QWidget *widget,
	    const QRect &r,
	    const QColorGroup &cg,
	    SFlags flags,
	    SCFlags sub,
	    SCFlags subActive,
	    const QStyleOption &data) const
{
    /* haven't successfully loaded the skin */
    if(!d) {
	QWindowsStyle::drawComplexControl(element, p, widget, r, cg, flags, 
		sub, subActive, data);
	return;
    }
    QSkinStyleItem *i = d->getItem(widget);
    if(i) {
	switch(element) {
	    case CC_Slider: 
		{
		    QSlider *sl = (QSlider *)widget;
		    QPixmap *pix = i->imageSets.find("Base");
		    QVector<QRect> *set = i->clipSets.find("Base");

		    if (pix && set) {
			int steps = set->size();
			if( steps == 0 )
			    break;

			/* scale between value the number of images we have */
			int scale = sl->maxValue() - sl->minValue();
			int pos = sl->value() - sl->minValue();

			pos = (pos * steps) / scale;

			if (pos >= steps)
			    pos = steps - 1;
			if (pos < 0)
			    pos = 0;

			QRect *clip = set->at(pos);

			if (!(pix && clip)) {
			    break;
			}
			p->drawPixmap(r.topLeft(), *pix, *clip);
		    } else {
			/* no 'set' of images.  try the individual approach */
			/* images
			   Groove
			   Handle
			   Line--- not image, but generated from one */
			QPixmap *pixGroove = i->images.find("Groove");
			QRect  *clipGroove = i->clips.find("Groove");
			QPixmap *pixHandle = i->images.find("Handle");
			QRect  *clipHandle = i->clips.find("Handle");

			if(pixGroove && clipGroove && pixHandle && clipHandle
				&& i->line.size() > 3) {

			    QPoint handlePos = querySubControlMetrics(element, widget, SC_SliderHandle).topLeft();


			    QPixmap buffer(r.size());
			    QPainter pb(&buffer);
			    QRect rb = r;
			    rb.moveTopLeft(QPoint(0,0));
			    if (sub & SC_SliderGroove) {
				// do this for the sake of shaped handles.
				pb.fillRect(rb, cg.brush(QColorGroup::Background));
				pb.drawPixmap(QPoint(0,0), *pixGroove, *clipGroove);
			    }

			    pb.drawPixmap(handlePos, *pixHandle, *clipHandle);
			    p->drawPixmap(r.topLeft(), buffer);

			} else {
			    break;
			}
		    }
		}
		return;
	}
    }

    /* fall back */
    QWindowsStyle::drawComplexControl(element, p, widget, r, cg, flags, 
	    sub, subActive, data);
}

void QSkinStyle::drawControl(ControlElement element,
	    QPainter *p,
	    const QWidget *widget,
	    const QRect &r,
	    const QColorGroup &cg,
	    SFlags flags,
	    const QStyleOption &data) const
{
    /* haven't successfully loaded the skin */
    if(!d) {
	QWindowsStyle::drawControl(element, p, widget, r, cg, flags, data);
	return;
    }

    QSkinStyleItem *i = d->getItem(widget);
    if(i) {
	switch(element) {
	    case CE_PushButton: 
		{
		    /* flag		named	backup
		       Raised		Up
		       Down		Down
		       On		OnUp	Down
		       On & Down	OnDown	Down
		       Disabled 	Default
		     */
		    QPixmap *pix = 0;
		    QRect   *clip = 0;
		    if(flags & Style_Enabled) {
			if (flags & Style_Down) {
			    pix = i->images.find("Down");
			    clip = i->clips.find("Down");
			    if ((flags & Style_On) 
				    && i->clips.find("OnDown") 
				    && i->images.find("OnDown")) 
			    {
				pix = i->images.find("OnDown");
				clip = i->clips.find("OnDown");
			    }
			} else {
			    if (flags & Style_On) {
				pix = i->images.find("OnUp");
				clip = i->clips.find("OnUp");
				if (!pix || !clip) {
				    pix = i->images.find("Down");
				    clip = i->clips.find("Down");
			        }
			    } else {
				pix = i->images.find("Up");
				clip = i->clips.find("Up");
			    }
			}
		    } else {
			pix = i->images.find("Disabled");
			clip = i->clips.find("Disabled");
		    }

		    /* fall backs */
		    if (!(pix && clip)) {
			pix = i->images.find("Default");
			clip = i->clips.find("Default");
		    }
		    if (!(pix && clip)) {
			QWindowsStyle::drawControl(element, p, widget, r, cg, 
				flags, data);
			return;
		    }

		    p->drawPixmap(r.topLeft(), *pix, *clip);
		}
		break;
	    case CE_PushButtonLabel:
		/* we don't draw a label */
		if (i->images.count() == 0) {
		    /* Unless of course we didn't draw the button */
		    QWindowsStyle::drawControl(element, p, widget, r, cg, 
			    flags, data);
			return;
	        }
		break;
	    case CE_CheckBox: 
		{
		    QPixmap *pix = 0;
		    QRect   *clip = 0;
		    if(flags & Style_Enabled) {
			if (flags & Style_On) {
			    pix = i->images.find("On");
			    clip = i->clips.find("On");
			} else if (flags & Style_Off) {
			    pix = i->images.find("Off");
			    clip = i->clips.find("Off");
			} else {
			    pix = i->images.find("NoChange");
			    clip = i->clips.find("NoChange");
			}
		    } else {
			pix = i->images.find("Disabled");
			clip = i->clips.find("Disabled");
		    }

		    /* fall backs */
		    if (!(pix && clip)) {
			pix = i->images.find("Default");
			clip = i->clips.find("Default");
		    }
		    if (!(pix && clip)) {
			QWindowsStyle::drawControl(element, p, widget, r, cg, 
				flags, data);
			return;
		    }

		    p->drawPixmap(r.topLeft(), *pix, *clip);
		}
		break;
	    case CE_RadioButton: 
		{
		    QPixmap *pix = 0;
		    QRect   *clip = 0;
		    if(flags & Style_Enabled) {
			if (flags & Style_On) {
			    pix = i->images.find("On");
			    clip = i->clips.find("On");
			} else {
			    pix = i->images.find("Off");
			    clip = i->clips.find("Off");
			}
		    } else {
			pix = i->images.find("Disabled");
			clip = i->clips.find("Disabled");
		    }

		    /* fall backs */
		    if (!(pix && clip)) {
			pix = i->images.find("Default");
			clip = i->clips.find("Default");
		    }
		    if (!(pix && clip)) {
			QWindowsStyle::drawControl(element, p, widget, r, cg, 
				flags, data);
			return;
		    }

		    p->drawPixmap(r.topLeft(), *pix, *clip);
		}
		break;
	    default:
		/* don't know how to draw this */
		QWindowsStyle::drawControl(element, p, widget, r, cg, 
			flags, data);
	}
    } else {
	QWindowsStyle::drawControl(element, p, widget, r, cg, flags, data);
    }
}

int QSkinStyle::pixelMetric( PixelMetric pm, const QWidget *widget ) const
{
    if(d && widget) {
	QSkinStyleItem *i = d->getItem(widget);
	if(i) {
	    switch(pm) {
		case PM_SliderSpaceAvailable:
		{
		    QSlider *sl = (QSlider *)widget;
		    QVector<QRect> *set = i->clipSets.find("Base");
		    if (set) {
			break;  /* we do the same as before */
		    } else {
			//QRect  *clipHandle = i->clips.find("Handle");
			if (i->line.size() > 3) {
			    return (i->line.back().x() - i->line.at(0).x());

			}
		    }
		}
	    }
	}
    }
    return QWindowsStyle::pixelMetric(pm, widget);
}
    
QRect QSkinStyle::querySubControlMetrics( ComplexControl cc, 
	const QWidget *widget, SubControl sc, 
	const QStyleOption &opt ) const
{
    /* have we successfully loaded the skin? */
    if(d) {
	QSkinStyleItem *i = d->getItem(widget);
	if(i) {
	    switch(sc) {
		case SC_SliderHandle:
		{
		    QSlider *sl = (QSlider *)widget;
		    QVector<QRect> *set = i->clipSets.find("Base");
		    if (set) {
			return *(set->at(0));
		    } else {
			QRect  *clipHandle = i->clips.find("Handle");
			if (clipHandle) {
			    //int steps = i->line.back().x() - i->line[0].x();
			    //if( steps == 0 )
				//break;

			    /* scale between value the number of images we have */
			    //int scale = sl->maxValue() - sl->minValue();
			    //int pos = sl->value() - sl->minValue();

			    //pos = (pos * steps) / scale;

			    //if (pos >= i->line.size())
				//pos = i->line.size() - 1;
			    //if (pos < 0)
				//pos = 0;
			    int pos = sl->sliderStart();
			    bool ok;
			    i->line.at(pos, &ok);
			    if (!ok)
				break;

			    QPoint handlePos = i->line[pos];
			    handlePos -= QPoint(clipHandle->width() / 2, clipHandle->height() / 2);
			    return QRect(handlePos.x(), handlePos.y(), 
				    clipHandle->width(), clipHandle->height());
			}
		    }
		}
	    }
	}
    }
    return QWindowsStyle::querySubControlMetrics(cc, widget, sc, opt);
}

QStyle::SubControl QSkinStyle::querySubControl( ComplexControl cc, 
	const QWidget *widget, const QPoint &pos, 
	const QStyleOption &opt ) const
{
    return QWindowsStyle::querySubControl(cc, widget, pos, opt);
}


void QSkinStyle::polish( QWidget *widget )
{
    if (!d)
	return;
    QSkinStyleItem *i = d->getItem(widget);

    if (widget->isTopLevel())
	widget->setBackgroundOrigin(QWidget::WindowOrigin);
    else
	widget->setBackgroundOrigin(QWidget::AncestorOrigin);

    if(i) {
	if (i->hasMask) {
	    widget->setMask(i->mask);
	}

	QPixmap *pix = i->images.find("Background");
	if (pix) {
	    QPalette palette = widget->palette();
	    palette.setBrush( QColorGroup::Background, QBrush(d->tColor, *pix));
	    widget->setPalette(palette);

	    widget->setBackgroundOrigin(QWidget::WidgetOrigin);
	}

	if (i->geom.width() > 0 && i->geom.height() > 0)
	    widget->setFixedSize(i->geom.size());

	if (i->isDragable) {
	    /* install the event filter on its ass. */
	    widget->installEventFilter(this);
	}

	// lots of setting up to do for sliders
	if (!qstrcmp(i->type, "QSlider")) {
	    widget->setBackgroundMode(NoBackground);
	    /* one of the images may be a 'Line' for positional information */
	    QPixmap *p = i->images.find("Line");
	    QRect *r = i->clips.find("Line");
	    if (p && r) {
		/* generate a line */
		QValueVector<QPoint> line;
		line.resize(p->width());
		QImage im = p->convertToImage();
		int j = 0;
		int k = 0;
		while (j < r->width()) {
		    int l = 0;
		    while (l < r->height()) 
		    {
			if (qAlpha(im.pixel(r->x() + j, r->y() + l)) == 255)
			    break;
			l++;
		    }
		    if (l != r->height()) 
		    {
			line[k] =  QPoint(j, l);
			k++;
		    }
		    j++;

		}
		line.resize(k);
		i->line = line;
	    }
	    
	}
    }
}

void QSkinStyle::polish( QApplication *app ) 
{
    if (!d)
	return;

    if (d->hasColors) {
	QPalette palette = app->palette();

	palette = QPalette(d->fColor, d->bColor);

	QColorGroup g =  palette.active();
	g.setColor(QColorGroup::ButtonText, d->tColor);
	g.setColor(QColorGroup::Text, d->tColor);
	g.setColor(QColorGroup::Foreground, d->tColor);
	if (d->backgroundPixmap)
	    g.setBrush(QColorGroup::Background, 
		    QBrush(d->tColor, *(d->backgroundPixmap)));
	palette.setActive(g);

	g =  palette.inactive();
	g.setColor(QColorGroup::ButtonText, d->tColor);
	g.setColor(QColorGroup::Text, d->tColor);
	g.setColor(QColorGroup::Foreground, d->tColor);
	if (d->backgroundPixmap)
	    g.setBrush(QColorGroup::Background, 
		    QBrush(d->tColor, *(d->backgroundPixmap)));
	palette.setInactive(g);

	g =  palette.normal();
	g.setColor(QColorGroup::ButtonText, d->tColor);
	g.setColor(QColorGroup::Text, d->tColor);
	g.setColor(QColorGroup::Foreground, d->tColor);
	if (d->backgroundPixmap)
	    g.setBrush(QColorGroup::Background, 
		    QBrush(d->tColor, *(d->backgroundPixmap)));
	palette.setNormal(g);

	app->setPalette(palette);
    }
    
}

/*********************************************************************
**                                                                  **
**   Start of Layout Code                                           **
**                                                                  **
*********************************************************************/

class QSkinLayoutIterator : public QGLayoutIterator
{
public:
    QSkinLayoutIterator(QPtrList<QLayoutItem> *l)
	: idx(0), list(l) {}

    QLayoutItem *current()
    { return idx < int(list->count()) ? list->at(idx) : 0; }

    QLayoutItem *next()
    { idx++; return current(); }

    QLayoutItem *takeCurrent()
    { return list->take(idx); }

private:
    int idx;
    QPtrList<QLayoutItem> *list;
};

QSkinLayout::QSkinLayout( QWidget *parent, const char *name ) 
    : QLayout(parent, 0, -1, name) {}
QSkinLayout::QSkinLayout( QLayout *parent, const char *name ) 
    : QLayout(parent, 0, name) {}

QSkinLayout::~QSkinLayout()
{
    deleteAllItems();
}

QLayoutIterator QSkinLayout::iterator()
{
    return QLayoutIterator(new QSkinLayoutIterator(&list) );
}

void QSkinLayout::addItem(QLayoutItem *item )
{
    list.append(item);
}

void QSkinLayout::setGeometry(const QRect &rect )
{
    QStyle &s = QApplication::style();
    
    QLayout::setGeometry(rect);

    if(qstrcmp(s.className(), "QSkinStyle"))
	return;

    QSkinStyle *ss = (QSkinStyle *)&s;

    if(!ss->defined(this))
	return;

    QLayoutIterator it = iterator();

    if (!it.current()) {
	return;
    }

    QLayoutItem *o;

    int i = 0;

    // XXX
    while ( (o = it.current()) != 0) {
	if(o->widget()) {
	    if(ss->defined(this, o->widget())) {
		QRect r = ss->getGeometry(this, o->widget());
		r.moveTopLeft( QPoint(rect.x() + r.x(), rect.y() + r.y()));
		o->widget()->setFixedSize(r.size());
		o->setGeometry(r);
	    } else {
		o->widget()->hide();
	    }

	}
	++it;
	++i;
    }
}

/* the next two functions are a hack.  They should base themselves of
   the range of contained items if the layout is not defined */
QSize QSkinLayout::sizeHint() const
{
    QStyle &s = QApplication::style();
    
    if(qstrcmp(s.className(), "QSkinStyle"))
	return QSize(100,100);

    QSkinStyle *ss = (QSkinStyle *)&s;

    /* QLayout::mainWidget() isn't const.. (why), but its body method appears
       to satisfy */


    if(ss->defined(this)) {
	return ss->getGeometry(this).size();
    }

    return QSize(100,100);
}

QSize QSkinLayout::minimumSize() const
{
    QStyle &s = QApplication::style();
    
    if(qstrcmp(s.className(), "QSkinStyle"))
	return QSize(100, 100);

    QSkinStyle *ss = (QSkinStyle *)&s;

    if(ss->defined(this)) {
	return ss->getGeometry(this).size();
    }

    return QSize(330, 130);
}

/***********************
 * Helper classes 
 **********************/

void QSkinDial::repaintScreen( const QRect *cr = 0 )
{
//    QDial::repaintScreen(cr);
    QStyle &s = QApplication::style();
   
    /* if not skin style */
    if(qstrcmp(s.className(), "QSkinStyle")) {
        QDial::repaintScreen(cr);
	return;
    }

    QSkinStyle *ss = (QSkinStyle *)&s;

    int steps = ss->countImages(this, "Base");
    if( steps == 0 ) {
        QDial::repaintScreen(cr);
	return;
    }

    QPainter p;
    p.begin(this);

    if(cr)
        p.setClipRect(*cr);
	

    int scale = (maxValue() - minValue());
    int i = value() - minValue();

    i *= steps;
    i /= scale;

    if (i >= steps)
       i = steps - 1;
    if (i < 0)
       i = 0;

    ss->drawImage(&p, this, "Base", i);
}
