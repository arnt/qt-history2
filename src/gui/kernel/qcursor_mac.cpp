/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qcursor_p.h>
#include <private/qpixmap_p.h>
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
        QWidget(0, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
        {
            setObjectName(QLatin1String("fake_cursor"));
            setAttribute(Qt::WA_TransparentForMouseEvents);
            setAcceptDrops(true);
            hide();
            int attribs = kWindowNoShadowAttribute;
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
            mask = QPixmap::fromImage(mi);
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
void qt_mac_set_cursor(const QCursor *c, const QPoint &)
{
    if (!c) {
        currentCursor = 0;
        return;
    }
    c->handle(); //force the cursor to get loaded, if it's not

    if(1 || currentCursor != c->d) {
        if(currentCursor && currentCursor->type == QCursorData::TYPE_ThemeCursor
                && currentCursor->curs.tc.anim)
            currentCursor->curs.tc.anim->stop();

        if(c->d->type == QCursorData::TYPE_CursPtr) {
            SetCursor(c->d->curs.cp.hcurs);
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
#ifdef QMAC_USE_BIG_CURSOR_API
    } else if(type == TYPE_BigCursor) {
        QDUnregisterNamedPixMapCursur(curs.big_cursor_name);
        free(curs.big_cursor_name);
#endif
    } else if(type == TYPE_FakeCursor) {
        free(curs.fc.empty_curs);
        delete curs.fc.widget;
    } else if(type == TYPE_ThemeCursor) {
        delete curs.tc.anim;
    }
    type = TYPE_None;

    delete bm;
    delete bmm;
    if(currentCursor == this)
        currentCursor = 0;
}

QCursorData *QCursorData::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
        qWarning("Qt: QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
        QCursorData *c = qt_cursorTable[0];
        c->ref.ref();
        return c;
    }
    // This is silly, but this is apparently called outside the constructor, so we have
    // to be ready for that case.
    QCursorData *x = new QCursorData;
    x->ref = 1;
    x->id = ++nextCursorId;
    x->bm  = new QBitmap(bitmap);
    x->bmm = new QBitmap(mask);
    x->cshape = Qt::BitmapCursor;
    x->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
    x->hy = hotY >= 0 ? hotY : bitmap.height() / 2;
    return x;
}

Qt::HANDLE QCursor::handle() const
{
    if(!QCursorData::initialized)
        QCursorData::initialize();
    if(d->type == QCursorData::TYPE_None)
        d->update();
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

QPixmap qt_mac_unmultiplyPixmapAlpha(const QPixmap &pixmap)
{
    QPixmap fake_pm = pixmap;
    fake_pm.detach();

    QPixmapData *pm_data = fake_pm.data;
    uint numPixels = pm_data->nbytes/4;
    for (uint i = 0; i < numPixels; ++i ) {
        int pixel = pm_data->pixels[i];
        int alpha = qAlpha(pixel);
        if (alpha == 0)
            continue;  // avoid div 0 on Intel

        int red   = qRed(pixel) * 255 / alpha;
        int green = qGreen(pixel) * 255 / alpha;
        int blue  = qBlue(pixel) * 255 / alpha;

        pm_data->pixels[i] = qRgba(red, green, blue, alpha);
#if defined(__i386__)
        pm_data->pixels[i] = CFSwapInt32(pm_data->pixels[i]);
#endif
    }
    return fake_pm;
}

void QCursorData::update()
{
    if(!QCursorData::initialized)
        QCursorData::initialize();
    if(type != QCursorData::TYPE_None)
        return;

    /* Note to self... ***
     * mask x data
     * 0xFF x 0x00 == fully opaque white
     * 0x00 x 0xFF == xor'd black
     * 0xFF x 0xFF == fully opaque black
     * 0x00 x 0x00 == fully transparent
     */

    switch(cshape) {                        // map Q cursor to MAC cursor
    case Qt::BitmapCursor: {
        if(bm->width() == 16 && bm->height() == 16 && pixmap.isNull()) {
            type = QCursorData::TYPE_CursPtr;
            curs.cp.my_cursor = true;
            curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
            QImage bmi, bmmi;
            bmi = bm->toImage().convertToFormat(QImage::Format_Mono);
            bmmi = bmm->toImage().convertToFormat(QImage::Format_Mono);
            memset(curs.cp.hcurs->data, 0, 32);
            memset(curs.cp.hcurs->mask, 0, 32);
            uchar *cursdata = reinterpret_cast<uchar *>(curs.cp.hcurs->data);
            uchar *cursmask = reinterpret_cast<uchar *>(curs.cp.hcurs->mask);

            for(int y = 0; y < bmi.height(); ++y) {
                uchar *dataScanLine = bmi.scanLine(y);
                uchar *maskScanLine = bmmi.scanLine(y);
                for (int x = 0; x < 2; ++x) {
                    *cursdata++ = dataScanLine[x];
                    *cursmask++ = maskScanLine[x];
                }
            }
#ifdef QMAC_USE_BIG_CURSOR_API
        } else if (bm->width() < 64 && bm->height() < 64) {
            curs.big_cursor_name = (char *)malloc(128);
            static int big_cursor_cnt = 0;
            sprintf(curs.big_cursor_name, "qt_QCursor_%d_%d", getpid(), big_cursor_cnt++);
            Point hotspot;
            if((hotspot.h = hx) < 0)
                hotspot.h = 0;
            if((hotspot.v = hy) < 0)
                hotspot.v = 0;
            OSStatus ret;
            if (pixmap.isNull()) {
                ret = QDRegisterNamedPixMapCursor(GetGWorldPixMap(qt_mac_qd_context(bm)),
                                                  GetGWorldPixMap(qt_mac_qd_context(bmm)),
                                                  hotspot, curs.big_cursor_name);
            } else {
                ret = QDRegisterNamedPixMapCursor(
                    GetGWorldPixMap((GWorldPtr)qt_mac_unmultiplyPixmapAlpha(pixmap).macQDHandle()),
                                      0, hotspot, curs.big_cursor_name);
            }
            if (ret == noErr)
                type = QCursorData::TYPE_BigCursor;
            else
                free(curs.big_cursor_name);
#endif
        }
        break; }
    case Qt::ArrowCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeArrowCursor;
        break; }
    case Qt::CrossCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeCrossCursor;
        break; }
    case Qt::WaitCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeWatchCursor;
        break; }
    case Qt::IBeamCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeIBeamCursor;
        break; }
    case Qt::SizeAllCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemePlusCursor;
        break; }
    case Qt::WhatsThisCursor: { //for now just use the pointing hand
    case Qt::PointingHandCursor:
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemePointingHandCursor;
        break; }
    case Qt::BusyCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeSpinningCursor;
        break; }
    case Qt::SplitVCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeResizeUpDownCursor;
        break; }
    case Qt::SplitHCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeResizeLeftRightCursor;
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

        type = QCursorData::TYPE_CursPtr;
        curs.cp.my_cursor = true;
        curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(curs.cp.hcurs->data, cur_ver_bits, sizeof(cur_ver_bits));
        memcpy(curs.cp.hcurs->mask, mcur_ver_bits, sizeof(mcur_ver_bits));
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

        type = QCursorData::TYPE_CursPtr;
        curs.cp.my_cursor = true;
        curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(curs.cp.hcurs->data, cur_hor_bits, sizeof(cur_hor_bits));
        memcpy(curs.cp.hcurs->mask, mcur_hor_bits, sizeof(mcur_hor_bits));
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

        type = QCursorData::TYPE_CursPtr;
        curs.cp.my_cursor = true;
        curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(curs.cp.hcurs->data, cur_fdiag_bits, sizeof(cur_fdiag_bits));
        memcpy(curs.cp.hcurs->mask, mcur_fdiag_bits, sizeof(mcur_fdiag_bits));
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

        type = QCursorData::TYPE_CursPtr;
        curs.cp.my_cursor = true;
        curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(curs.cp.hcurs->data, cur_bdiag_bits, sizeof(cur_bdiag_bits));
        memcpy(curs.cp.hcurs->mask, mcur_bdiag_bits, sizeof(mcur_bdiag_bits));
        break; }
    case Qt::ForbiddenCursor: {
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeNotAllowedCursor;
        break; }
    case Qt::BlankCursor: {
        type = QCursorData::TYPE_CursPtr;
        curs.cp.my_cursor = true;
        curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memset(curs.cp.hcurs->data, 0x00, sizeof(curs.cp.hcurs->data));
        memset(curs.cp.hcurs->mask, 0x00, sizeof(curs.cp.hcurs->data));
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

        type = QCursorData::TYPE_CursPtr;
        curs.cp.my_cursor = true;
        curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
        memcpy(curs.cp.hcurs->data, cur_up_arrow_bits, sizeof(cur_up_arrow_bits));
        memcpy(curs.cp.hcurs->mask, mcur_up_arrow_bits, sizeof(mcur_up_arrow_bits));
        break; }
    case Qt::OpenHandCursor:
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeOpenHandCursor;
        break;
    case Qt::ClosedHandCursor:
        type = QCursorData::TYPE_ThemeCursor;
        curs.tc.curs = kThemeClosedHandCursor;
        break;
#endif
    default:
        qWarning("Qt: QCursor::update: Invalid cursor shape %d", cshape);
        return;
    }

    if(type == QCursorData::TYPE_CursPtr && curs.cp.hcurs && curs.cp.my_cursor) {
        curs.cp.hcurs->hotSpot.h = hx >= 0 ? hx : 8;
        curs.cp.hcurs->hotSpot.v = hy >= 0 ? hy : 8;
    }
}
