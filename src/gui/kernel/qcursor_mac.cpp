/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qcursor_p.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <string.h>
#include <unistd.h>

/*****************************************************************************
  Externals
 *****************************************************************************/
extern QCursorData *qt_cursorTable[Qt::LastCursor + 1];
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern GrafPtr qt_mac_qd_context(const QPaintDevice *); //qpaintdevice_mac.cpp
extern bool qt_sendSpontaneousEvent(QObject *, QEvent *); //qapplication_mac.cpp

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
# define QMAC_NO_FAKECURSOR
#endif

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/
#include <qpainter.h>
class QMacCursorWidget : public QWidget
{
    Q_OBJECT
    QBitmap bitmap;
public:
    QMacCursorWidget(const QBitmap *b, const QBitmap *m) :
        QWidget(0, Qt::WType_Dialog | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_StaysOnTop)
        {
            setObjectName("fake_cursor");
            setAttribute(Qt::WA_TransparentForMouseEvents);
            setAcceptDrops(true);
            hide();
            int attribs = kWindowNoShadowAttribute;
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_3)
            attribs |= kWindowIgnoreClicksAttribute;
#endif
            ChangeWindowAttributes(qt_mac_window_for(this), attribs, 0);
            int w = b->width(), h = b->height();
            resize(w, h);

            QImage bi, mi;
            bitmap = *b;
            bi = bitmap.toImage();
            mi = m->toImage();
            for(int y = 0; y < bi.height(); y++) {
                for(int x = 0; x < bi.width(); x++)
                    mi.setPixel(x, y, !(bi.pixel(x, y) && mi.pixel(x, y)));
            }
            QBitmap mask;
            mask = mi;
            setMask(mask);
        }
    ~QMacCursorWidget() { }
protected:
    void paintEvent(QPaintEvent *) { QPainter p(this); p.drawPixmap(0, 0, bitmap); }
};
#include "qcursor_mac.moc"

class QMacAnimateCursor : public QObject
{
    int timerId, step;
    ThemeCursor curs;
public:
    QMacAnimateCursor() : QObject(), timerId(-1) { }
    void start(ThemeCursor c) {
        step = 1;
        if(timerId != -1)
            killTimer(timerId);
        timerId = startTimer(300);
        curs = c;
    }
    void stop() {
        if(timerId != -1) {
            killTimer(timerId);
            timerId = -1;
        }
    }
protected:
    void timerEvent(QTimerEvent *e) {
        if(e->timerId() == timerId) {
            if(SetAnimatedThemeCursor(curs, step++) == themeBadCursorIndexErr)
                stop();
        }
    }
};

static QCursorData *currentCursor = 0; //current cursor
#ifndef QMAC_NO_FAKECURSOR
static Point currentPoint = { 0, 0 };
#endif
void qt_mac_set_cursor(const QCursor *c, const QPoint &p)
{
    if(!c) {
#ifndef QMAC_NO_FAKECURSOR
        if(currentCursor && currentCursor->type == QCursorData::TYPE_FakeCursor)
            currentCursor->curs.fc.widget->hide();
#endif
        currentCursor = 0;
        return;
    }
    c->handle(); //force the cursor to get loaded, if it's not

#ifndef QMAC_NO_FAKECURSOR
    if(c->d->type == QCursorData::TYPE_FakeCursor &&
       (currentCursor != c->d || currentPoint.h != p->x() || currentPoint.v != p->y())) {
        /* That's right folks, I want nice big cursors - if apple won't give them to me, why
           I'll just take them!!! */
        c->d->curs.fc.widget->move(p->x() - c->d->curs.fc.empty_curs->hotSpot.h,
                                   p->y() - c->d->curs.fc.empty_curs->hotSpot.v);
        SetCursor(c->d->curs.fc.empty_curs);
        if(currentCursor && currentCursor != c->d
                && currentCursor->type == QCursorData::TYPE_FakeCursor)
            currentCursor->curs.fc.widget->hide();
        if(!c->d->curs.fc.widget->isVisible())
            c->d->curs.fc.widget->show();
    } else
#else
        Q_UNUSED(p);
#endif
    if(currentCursor != c->d) {
        if(currentCursor && currentCursor->type == QCursorData::TYPE_ThemeCursor
                && currentCursor->curs.tc.anim)
            currentCursor->curs.tc.anim->stop();

        if(c->d->type == QCursorData::TYPE_CursPtr) {
            SetCursor(c->d->curs.cp.hcurs);
        } else if(c->d->type == QCursorData::TYPE_CursorImage) {

        } else if(c->d->type == QCursorData::TYPE_ThemeCursor) {
            if(SetAnimatedThemeCursor(c->d->curs.tc.curs, 0) == themeBadCursorIndexErr) {
                SetThemeCursor(c->d->curs.tc.curs);
            } else {
                if(!c->d->curs.tc.anim)
                    c->d->curs.tc.anim = new QMacAnimateCursor;
                c->d->curs.tc.anim->start(c->d->curs.tc.curs);
            }
#ifdef QMAC_USE_BIG_CURSOR_API
        } else if(c->d->type == QCursorData::TYPE_BigCursor) {
            QDSetNamedPixMapCursor(c->d->curs.big_cursor_name);
#endif
        } else {
//            qDebug("Qt: internal: WH0A. Unexpected condition reached!");
        }
    }
    currentCursor = c->d;
}

static int nextCursorId = Qt::BitmapCursor;

QCursorData::QCursorData(Qt::CursorShape s)
    : cshape(s), bm(0), bmm(0), hx(-1), hy(-1), id(s), type(TYPE_None)
{
    ref = 1;
    memset(&curs, '\0', sizeof(curs));
}

QCursorData::~QCursorData()
{
    if (type == TYPE_CursPtr) {
        if (curs.cp.hcurs && curs.cp.my_cursor)
            free(curs.cp.hcurs);
    } else if (type == TYPE_CursorImage) {
        free(curs.ci);
#ifdef QMAC_USE_BIG_CURSOR_API
    } else if(type == TYPE_BigCursor) {
        QDUnregisterNamedPixMapCursur(curs.big_cursor_name);
        free(curs.big_cursor_name);
#endif
    } else if(type == TYPE_FakeCursor) {
#ifndef QMAC_NO_FAKECURSOR
        free(curs.fc.empty_curs);
        delete curs.fc.widget;
#endif
    } else if(type == TYPE_ThemeCursor) {
        delete curs.tc.anim;
    }
    type = TYPE_None;

    delete bm;
    delete bmm;
    if(currentCursor == this)
        currentCursor = 0;
}

void QCursor::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
        qWarning("Qt: QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
        QCursorData *c = qt_cursorTable[0];
        c->ref.ref();
        d = c;
        return;
    }
    // This is silly, but this is apparently called outside the constructor, so we have
    // to be ready for that case.
    QCursorData *x;
    if (!d) {
        x = new QCursorData;
        x->ref = 1;
        x->id = ++nextCursorId;
    } else {
        x = d;
    }
    d = x;
    x->bm  = new QBitmap(bitmap);
    x->bmm = new QBitmap(mask);
    x->cshape = Qt::BitmapCursor;
    x->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
    x->hy = hotY >= 0 ? hotY : bitmap.height() / 2;
}

Qt::HANDLE QCursor::handle() const
{
    if(!QCursorData::initialized)
        QCursorData::initialize();
    if(d->type == QCursorData::TYPE_None)
        update();
    return (Qt::HANDLE)d->id;
}

QPoint QCursor::pos()
{
    Point p;
    GetGlobalMouse(&p);
    return QPoint(p.h, p.v);
}

void QCursor::setPos(int x, int y)
{
    CGWarpMouseCursorPosition(CGPointMake(x, y));

    /* I'm not too keen on doing this, but this makes it a lot easier, so I just
       send the event back through the event system and let it get propagated correctly
       ideally this would not really need to be faked --Sam
    */
    QWidget *widget = 0;
    if(QWidget *grb = QWidget::mouseGrabber())
        widget = grb;
    else
        widget = QApplication::widgetAt(QPoint(x, y));
    if(widget) {
        QMouseEvent me(QMouseEvent::MouseMove, widget->mapFromGlobal(QPoint(x, y)), Qt::NoButton,
                       QApplication::mouseButtons(), QApplication::keyboardModifiers());
        qt_sendSpontaneousEvent(widget, &me);
    }
}

void QCursor::update() const
{
    if(!QCursorData::initialized)
        QCursorData::initialize();
    if(d->type != QCursorData::TYPE_None)
        return;

    /* Note to self... ***
     * mask x data
     * 0xFF x 0x00 == fully opaque white
     * 0x00 x 0xFF == xor'd black
     * 0xFF x 0xFF == fully opaque black
     * 0x00 x 0x00 == fully transparent
     */

    switch(d->cshape) {                        // map Q cursor to MAC cursor
    case Qt::BitmapCursor: {
        if(d->bm->width() == 16 && d->bm->height() == 16) {
            d->type = QCursorData::TYPE_CursPtr;
            d->curs.cp.my_cursor = true;
            d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
            QImage bmi, bmmi;
            bmi = d->bm->toImage();
            bmmi = d->bmm->toImage();

            memset(d->curs.cp.hcurs->mask, 0, 32);
            memset(d->curs.cp.hcurs->data, 0, 32);
            for(int y = 0; y < 16; y++) {
                for(int x = 0; x < 16; x++) {
                    int bmi_val = 0, bmmi_val = 0;
                    if(!bmmi.pixel(x, y)) {
                        if(bmi.pixel(x, y))
                            bmmi_val = 1;
                        else
                            bmi_val = bmmi_val = 1;
                    }
                    if(bmmi_val)
                        *(((uchar*)d->curs.cp.hcurs->mask) + (y*2) + (x / 8)) |= (1 << (7 - (x % 8)));
                    if(bmi_val)
                        *(((uchar*)d->curs.cp.hcurs->data) + (y*2) + (x / 8)) |= (1 << (7 - (x % 8)));
                }
            }
#ifdef QMAC_USE_BIG_CURSOR_API
        } else if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_2 && d->bm->width() < 64
                  && d->bm->height() < 64) {
            d->curs.big_cursor_name = (char *)malloc(128);
            static int big_cursor_cnt = 0;
            sprintf(d->curs.big_cursor_name, "qt_QCursor_%d_%d", getpid(), big_cursor_cnt++);
            Point hotspot;
            if((hotspot.h = d->hx) < 0)
                hotspot.h = 0;
            if((hotspot.v = d->hy) < 0)
                hotspot.v = 0;
            OSStatus ret = QDRegisterNamedPixMapCursor(GetGWorldPixMap(qt_mac_qd_context(d->bm)),
                                                       GetGWorldPixMap(qt_mac_qd_context(d->bmm)),
                                                       hotspot, d->curs.big_cursor_name);
            if(ret == noErr)
                d->type = QCursorData::TYPE_BigCursor;
            else
                free(d->curs.big_cursor_name);
#endif
        }
        if(d->type == QCursorData::TYPE_None) {
#ifndef QMAC_NO_FAKECURSOR
            d->type = QCursorData::TYPE_FakeCursor;
            d->curs.fc.widget = new QMacCursorWidget(d->bm, d->bmm);
            //make an empty cursor
            d->curs.fc.empty_curs = (CursPtr)malloc(sizeof(Cursor));
            memset(d->curs.fc.empty_curs->data, 0x00, sizeof(d->curs.fc.empty_curs->data));
            memset(d->curs.fc.empty_curs->mask, 0x00, sizeof(d->curs.fc.empty_curs->mask));
            int hx = d->hx, hy = d->hy;
            if(hx < 0)
                hx = 8;
            else if(hx > 15)
                hx = 15;
            if(hy < 0)
                hy = 8;
            else if(hy > 15)
                hy = 15;
            d->curs.fc.empty_curs->hotSpot.h = hx;
            d->curs.fc.empty_curs->hotSpot.v = hy;
#else
            d->type = QCursorData::TYPE_CursorImage;
            d->curs.ci = (CursorImageRec*)malloc(sizeof(CursorImageRec));
            d->curs.ci->majorVersion = kCursorImageMajorVersion;
            d->curs.ci->minorVersion = kCursorImageMinorVersion;
            d->curs.ci->cursorPixMap = GetGWorldPixMap(qt_mac_qd_context(d->bm));
            d->curs.ci->cursorBitMask = (BitMap **)GetGWorldPixMap(qt_mac_qd_context(d->bmm));
#endif
        }
        break; }
    case Qt::ArrowCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemeArrowCursor;
        break; }
    case Qt::CrossCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemeCrossCursor;
        break; }
    case Qt::WaitCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemeWatchCursor;
        break; }
    case Qt::IBeamCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemeIBeamCursor;
        break; }
    case Qt::SizeAllCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemePlusCursor;
        break; }
    case Qt::WhatsThisCursor: { //for now just use the pointing hand
    case Qt::PointingHandCursor:
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemePointingHandCursor;
        break; }
    case Qt::BusyCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemeSpinningCursor;
        break; }
    case Qt::SplitVCursor: {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if(qMacVersion() >= QSysInfo::MV_10_3) {
            d->type = QCursorData::TYPE_ThemeCursor;
            d->curs.tc.curs = kThemeResizeUpDownCursor;
            break;
        }
#endif
#if defined( QMAC_NO_FAKECURSOR ) || 1
	static const unsigned char cur_vsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x01, 0x00, 0x01, 0x00,
	    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x01, 0x00, 0x01, 0x00,
	    0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const unsigned char mcur_vsplit_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x01, 0x00, 0x01, 0x00,
	    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x01, 0x00, 0x01, 0x00,
	    0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = true;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_vsplit_bits, sizeof(cur_vsplit_bits));
        memcpy(d->curs.cp.hcurs->mask, mcur_vsplit_bits, sizeof(mcur_vsplit_bits));
#else
	static uchar cur_vsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
	    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
	    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
            0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static uchar mcur_vsplit_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	    0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
	    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
            0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
	    0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
	    0x80, 0xff, 0xff, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
	    0x00, 0xc0, 0x01, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
            0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	QBitmap bm(32, 32, cur_vsplit_bits, true), bmm(32, 32, mcur_vsplit_bits, true);
	setBitmap(bm, bmm, 16, 16);
#endif
        break; }
    case Qt::SplitHCursor: {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if(qMacVersion() >= QSysInfo::MV_10_3) {
            d->type = QCursorData::TYPE_ThemeCursor;
            d->curs.tc.curs = kThemeResizeLeftRightCursor;
            break;
        }
#endif
#if defined( QMAC_NO_FAKECURSOR ) || 1
	static const unsigned char cur_hsplit_bits[] = {
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x24, 0x90, 0x7c, 0xf8, 0x24, 0x90, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x00, 0x00 };
	static const unsigned char mcur_hsplit_bits[] = {
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x24, 0x90, 0x7c, 0xf8, 0x24, 0x90, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = true;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_hsplit_bits, sizeof(cur_hsplit_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_hsplit_bits, sizeof(mcur_hsplit_bits));
#else
	static uchar cur_hsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
	    0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
	    0x00, 0x41, 0x82, 0x00, 0x80, 0x41, 0x82, 0x01, 0xc0, 0x7f, 0xfe, 0x03,
            0x80, 0x41, 0x82, 0x01, 0x00, 0x41, 0x82, 0x00, 0x00, 0x40, 0x02, 0x00,
	    0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
	    0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static uchar mcur_hsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
	    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe2, 0x47, 0x00, 0x00, 0xe3, 0xc7, 0x00,
	    0x80, 0xe3, 0xc7, 0x01, 0xc0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
            0xc0, 0xff, 0xff, 0x03, 0x80, 0xe3, 0xc7, 0x01, 0x00, 0xe3, 0xc7, 0x00,
	    0x00, 0xe2, 0x47, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
	    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        QBitmap bm(32, 32, cur_hsplit_bits, true), bmm(32, 32, mcur_hsplit_bits, true);
	setBitmap(bm, bmm, 16, 16);
#endif
        break; }

#define QT_USE_APPROXIMATE_CURSORS
#ifdef QT_USE_APPROXIMATE_CURSORS
    case Qt::SizeVerCursor: {
        static const uchar cur_ver_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
            0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0f, 0xf0,
            0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00 };
        static const uchar mcur_ver_bits[] = {
            0x00, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8,
            0x7f, 0xfc, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x7f, 0xfc, 0x3f, 0xf8,
            0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80 };

        d->type = QCursorData::TYPE_CursPtr;
        d->curs.cp.my_cursor = true;
        d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(d->curs.cp.hcurs->data, cur_ver_bits, sizeof(cur_ver_bits));
        memcpy(d->curs.cp.hcurs->mask, mcur_ver_bits, sizeof(mcur_ver_bits));
        break; }

    case Qt::SizeHorCursor: {
        static const uchar cur_hor_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x18, 0x30,
            0x38, 0x38, 0x7f, 0xfc, 0x7f, 0xfc, 0x38, 0x38, 0x18, 0x30, 0x08, 0x20,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static const uchar mcur_hor_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x0c, 0x60, 0x1c, 0x70, 0x3c, 0x78,
            0x7f, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0x7f, 0xfc, 0x3c, 0x78,
            0x1c, 0x70, 0x0c, 0x60, 0x04, 0x40, 0x00, 0x00 };

        d->type = QCursorData::TYPE_CursPtr;
        d->curs.cp.my_cursor = true;
        d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(d->curs.cp.hcurs->data, cur_hor_bits, sizeof(cur_hor_bits));
        memcpy(d->curs.cp.hcurs->mask, mcur_hor_bits, sizeof(mcur_hor_bits));
        break; }

    case Qt::SizeBDiagCursor: {
        static const uchar cur_fdiag_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0xf8, 0x00, 0x78,
            0x00, 0xf8, 0x01, 0xd8, 0x23, 0x88, 0x37, 0x00, 0x3e, 0x00, 0x3c, 0x00,
            0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static const uchar mcur_fdiag_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00, 0xfc,
            0x41, 0xfc, 0x63, 0xfc, 0x77, 0xdc, 0x7f, 0x8c, 0x7f, 0x04, 0x7e, 0x00,
            0x7f, 0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x00, 0x00 };

        d->type = QCursorData::TYPE_CursPtr;
        d->curs.cp.my_cursor = true;
        d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(d->curs.cp.hcurs->data, cur_fdiag_bits, sizeof(cur_fdiag_bits));
        memcpy(d->curs.cp.hcurs->mask, mcur_fdiag_bits, sizeof(mcur_fdiag_bits));
        break; }
    case Qt::SizeFDiagCursor: {
        static const uchar cur_bdiag_bits[] = {
            0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e, 0x00,
            0x37, 0x00, 0x23, 0x88, 0x01, 0xd8, 0x00, 0xf8, 0x00, 0x78, 0x00, 0xf8,
            0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static const uchar mcur_bdiag_bits[] = {
            0x00, 0x00, 0x7f, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7e, 0x00, 0x7f, 0x04,
            0x7f, 0x8c, 0x77, 0xdc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01, 0xfc,
            0x03, 0xfc, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00 };

        d->type = QCursorData::TYPE_CursPtr;
        d->curs.cp.my_cursor = true;
        d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(d->curs.cp.hcurs->data, cur_bdiag_bits, sizeof(cur_bdiag_bits));
        memcpy(d->curs.cp.hcurs->mask, mcur_bdiag_bits, sizeof(mcur_bdiag_bits));
        break; }
    case Qt::ForbiddenCursor: {
        d->type = QCursorData::TYPE_ThemeCursor;
        d->curs.tc.curs = kThemeNotAllowedCursor;
        break; }
    case Qt::BlankCursor: {
        d->type = QCursorData::TYPE_CursPtr;
        d->curs.cp.my_cursor = true;
        d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memset(d->curs.cp.hcurs->data, 0x00, sizeof(d->curs.cp.hcurs->data));
        memset(d->curs.cp.hcurs->mask, 0x00, sizeof(d->curs.cp.hcurs->data));
        break; }
    case Qt::UpArrowCursor: {
        static const unsigned char cur_up_arrow_bits[] = {
            0x00, 0x80, 0x01, 0x40, 0x01, 0x40, 0x02, 0x20, 0x02, 0x20, 0x04, 0x10,
            0x04, 0x10, 0x08, 0x08, 0x0f, 0x78, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40,
            0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0xc0 };
        static const unsigned char mcur_up_arrow_bits[] = {
            0x00, 0x80, 0x01, 0xc0, 0x01, 0xc0, 0x03, 0xe0, 0x03, 0xe0, 0x07, 0xf0,
            0x07, 0xf0, 0x0f, 0xf8, 0x0f, 0xf8, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0,
            0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0 };

        d->type = QCursorData::TYPE_CursPtr;
        d->curs.cp.my_cursor = true;
        d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(d->curs.cp.hcurs->data, cur_up_arrow_bits, sizeof(cur_up_arrow_bits));
        memcpy(d->curs.cp.hcurs->mask, mcur_up_arrow_bits, sizeof(mcur_up_arrow_bits));
        break; }
#endif
    default:
        qWarning("Qt: QCursor::update: Invalid cursor shape %d", d->cshape);
        return;
    }

    if(d->type == QCursorData::TYPE_CursPtr && d->curs.cp.hcurs && d->curs.cp.my_cursor) {
        d->curs.cp.hcurs->hotSpot.h = d->hx >= 0 ? d->hx : 8;
        d->curs.cp.hcurs->hotSpot.v = d->hy >= 0 ? d->hy : 8;
    }
}
