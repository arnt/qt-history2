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
#include <qbitmap.h>
#include <qcursor.h>
#include <qimage.h>
#include <qt_windows.h>

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

QCursorData::QCursorData(Qt::CursorShape s)
  : cshape(s), bm(0), bmm(0), hx(0), hy(0), hcurs(0)
{
    ref = 1;
}

QCursorData::~QCursorData()
{
    delete bm;
    delete bmm;
#ifndef Q_OS_TEMP
    if (hcurs)
        DestroyCursor(hcurs);
#endif
}


void QCursor::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
        qWarning("QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
        QCursorData *c = qt_cursorTable[0];
        c->ref.ref();
        d = c;
        return;
    }
    d = new QCursorData;
    d->bm  = new QBitmap(bitmap);
    d->bmm = new QBitmap(mask);
    d->hcurs = 0;
    d->cshape = Qt::BitmapCursor;
    d->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    d->hy = hotY >= 0 ? hotY : bitmap.height()/2;
}

HCURSOR QCursor::handle() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (!d->hcurs)
        update();
    return d->hcurs;
}

QCursor::QCursor(HCURSOR handle)
{
    d = new QCursorData;
    d->hcurs = handle;
}

QPoint QCursor::pos()
{
    POINT p;
    GetCursorPos(&p);
    return QPoint(p.x, p.y);
}

void QCursor::setPos(int x, int y)
{
    SetCursorPos(x, y);
}


void QCursor::update() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (d->hcurs)
        return;

    // Non-standard Windows cursors are created from bitmaps

    static const uchar vsplit_bits[] = {
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
    static const uchar vsplitm_bits[] = {
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
    static const uchar hsplit_bits[] = {
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
    static const uchar hsplitm_bits[] = {
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
    static const uchar phand_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x80, 0x04, 0x00, 0x00,
        0x80, 0x04, 0x00, 0x00, 0x80, 0x04, 0x00, 0x00, 0x80, 0x04, 0x00, 0x00,
        0x80, 0x1c, 0x00, 0x00, 0x80, 0xe4, 0x00, 0x00, 0x80, 0x24, 0x03, 0x00,
        0x80, 0x24, 0x05, 0x00, 0xb8, 0x24, 0x09, 0x00, 0xc8, 0x00, 0x09, 0x00,
        0x88, 0x00, 0x08, 0x00, 0x90, 0x00, 0x08, 0x00, 0xa0, 0x00, 0x08, 0x00,
        0x20, 0x00, 0x08, 0x00, 0x40, 0x00, 0x08, 0x00, 0x40, 0x00, 0x04, 0x00,
        0x80, 0x00, 0x04, 0x00, 0x80, 0x00, 0x04, 0x00, 0x00, 0x01, 0x02, 0x00,
        0x00, 0x01, 0x02, 0x00, 0x00, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

   static const uchar phandm_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00,
        0x80, 0x07, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00,
        0x80, 0x1f, 0x00, 0x00, 0x80, 0xff, 0x00, 0x00, 0x80, 0xff, 0x03, 0x00,
        0x80, 0xff, 0x07, 0x00, 0xb8, 0xff, 0x0f, 0x00, 0xf8, 0xff, 0x0f, 0x00,
        0xf8, 0xff, 0x0f, 0x00, 0xf0, 0xff, 0x0f, 0x00, 0xe0, 0xff, 0x0f, 0x00,
        0xe0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x07, 0x00,
        0x80, 0xff, 0x07, 0x00, 0x80, 0xff, 0x07, 0x00, 0x00, 0xff, 0x03, 0x00,
        0x00, 0xff, 0x03, 0x00, 0x00, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    static const uchar * const cursor_bits32[] = {
        vsplit_bits, vsplitm_bits, hsplit_bits, hsplitm_bits,
        phand_bits, phandm_bits
    };

    unsigned short *sh;
    switch (d->cshape) {                        // map to windows cursor
    case Qt::ArrowCursor:
        sh = (unsigned short*)IDC_ARROW;
        break;
    case Qt::UpArrowCursor:
        sh = (unsigned short*)IDC_UPARROW;
        break;
    case Qt::CrossCursor:
        sh = (unsigned short*)IDC_CROSS;
        break;
    case Qt::WaitCursor:
        sh = (unsigned short*)IDC_WAIT;
        break;
    case Qt::IBeamCursor:
        sh = (unsigned short*)IDC_IBEAM;
        break;
    case Qt::SizeVerCursor:
        sh = (unsigned short*)IDC_SIZENS;
        break;
    case Qt::SizeHorCursor:
        sh = (unsigned short*)IDC_SIZEWE;
        break;
    case Qt::SizeBDiagCursor:
        sh = (unsigned short*)IDC_SIZENESW;
        break;
    case Qt::SizeFDiagCursor:
        sh = (unsigned short*)IDC_SIZENWSE;
        break;
    case Qt::SizeAllCursor:
        sh = (unsigned short*)IDC_SIZEALL;
        break;
    case Qt::ForbiddenCursor:
        sh = (unsigned short*)IDC_NO;
        break;
    case Qt::WhatsThisCursor:
        sh = (unsigned short*)IDC_HELP;
        break;
    case Qt::BusyCursor:
        sh = (unsigned short*)IDC_APPSTARTING;
        break;
    case Qt::PointingHandCursor:
        if ((QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) > QSysInfo::WV_95 ||
            (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based) > QSysInfo::WV_NT) {
            sh = (unsigned short*)IDC_HAND;
            break;
        }
        // fall through
    case Qt::BlankCursor:
    case Qt::SplitVCursor:
    case Qt::SplitHCursor:
    case Qt::BitmapCursor: {
        QImage bbits, mbits;
        bool invb, invm;
        if (d->cshape == Qt::BlankCursor) {
            bbits.create(32, 32, 1, 2, QImage::BigEndian);
            bbits.fill(0);                // ignore color table
            mbits = bbits.copy();
            d->hx = d->hy = 16;
            invb = invm = false;
        } else if (d->cshape != Qt::BitmapCursor) {
            int i = d->cshape - Qt::SplitVCursor;
            QBitmap cb(32, 32, cursor_bits32[i * 2], true);
            QBitmap cm(32, 32, cursor_bits32[i * 2 + 1], true);
            bbits = cb.toImage();
            mbits = cm.toImage();
            if (d->cshape == Qt::PointingHandCursor) {
                d->hx = 7;
                d->hy = 0;
            } else
                d->hx = d->hy = 16;
            invb = invm = false;
        } else {
            bbits = d->bm->toImage();
            mbits = d->bmm->toImage();
            invb = bbits.numColors() > 1 && qGray(bbits.color(0)) < qGray(bbits.color(1));
            invm = mbits.numColors() > 1 && qGray(mbits.color(0)) < qGray(mbits.color(1));
        }
        int n = qMax(1, bbits.width() / 8);
        int h = bbits.height();
        uchar* xBits = new uchar[h * n];
        uchar* xMask = new uchar[h * n];
        int x = 0;
        for (int i = 0; i < h; ++i) {
            uchar *bits = bbits.scanLine(i);
            uchar *mask = mbits.scanLine(i);
            for (int j = 0; j < n; ++j) {
                uchar b = bits[j];
                uchar m = mask[j];
                if (invb)
                    b ^= 0xff;
                if (invm)
                    m ^= 0xff;
                xBits[x] = ~m;
                xMask[x] = b ^ m;
                ++x;
            }
        }
#ifndef Q_OS_TEMP
        d->hcurs = CreateCursor(qWinAppInst(), d->hx, d->hy, bbits.width(), bbits.height(),
                                   xBits, xMask);
#endif
	delete [] xBits;
	delete [] xMask;
	return;
    }
    default:
        qWarning("QCursor::update: Invalid cursor shape %d", d->cshape);
        return;
    }
    // ### From MSDN:
    // ### LoadCursor has been superseded by the LoadImage function.
    QT_WA({
        d->hcurs = LoadCursorW(0, reinterpret_cast<const TCHAR *>(sh));
    } , {
        d->hcurs = LoadCursorA(0, reinterpret_cast<const char*>(sh));
    });
}
