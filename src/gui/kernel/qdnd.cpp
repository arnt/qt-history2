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

#include "qplatformdefs.h"

#include "qbitmap.h"
#include "qdrag.h"
#include "qpixmap.h"
#include "qevent.h"
#include "qfile.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qregexp.h"
#include "qdir.h"
#include "qdnd_p.h"
#include "qimagereader.h"
#include "qimagewriter.h"
#include <ctype.h>

// These pixmaps approximate the images in the Windows User Interface Guidelines.

// XPM

static const char * const move_xpm[] = {
"11 20 3 1",
".        c None",
#if defined(Q_WS_WIN)
"a        c #000000",
"X        c #FFFFFF", // Windows cursor is traditionally white
#else
"a        c #FFFFFF",
"X        c #000000", // X11 cursor is traditionally black
#endif
"aa.........",
"aXa........",
"aXXa.......",
"aXXXa......",
"aXXXXa.....",
"aXXXXXa....",
"aXXXXXXa...",
"aXXXXXXXa..",
"aXXXXXXXXa.",
"aXXXXXXXXXa",
"aXXXXXXaaaa",
"aXXXaXXa...",
"aXXaaXXa...",
"aXa..aXXa..",
"aa...aXXa..",
"a.....aXXa.",
"......aXXa.",
".......aXXa",
".......aXXa",
"........aa."};

/* XPM */
static const char * const copy_xpm[] = {
"24 30 3 1",
".        c None",
"a        c #000000",
"X        c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
"aa......................",
"aXa.....................",
"aXXa....................",
"aXXXa...................",
"aXXXXa..................",
"aXXXXXa.................",
"aXXXXXXa................",
"aXXXXXXXa...............",
"aXXXXXXXXa..............",
"aXXXXXXXXXa.............",
"aXXXXXXaaaa.............",
"aXXXaXXa................",
"aXXaaXXa................",
"aXa..aXXa...............",
"aa...aXXa...............",
"a.....aXXa..............",
"......aXXa..............",
".......aXXa.............",
".......aXXa.............",
"........aa...aaaaaaaaaaa",
#else
"XX......................",
"XaX.....................",
"XaaX....................",
"XaaaX...................",
"XaaaaX..................",
"XaaaaaX.................",
"XaaaaaaX................",
"XaaaaaaaX...............",
"XaaaaaaaaX..............",
"XaaaaaaaaaX.............",
"XaaaaaaXXXX.............",
"XaaaXaaX................",
"XaaXXaaX................",
"XaX..XaaX...............",
"XX...XaaX...............",
"X.....XaaX..............",
"......XaaX..............",
".......XaaX.............",
".......XaaX.............",
"........XX...aaaaaaaaaaa",
#endif
".............aXXXXXXXXXa",
".............aXXXXXXXXXa",
".............aXXXXaXXXXa",
".............aXXXXaXXXXa",
".............aXXaaaaaXXa",
".............aXXXXaXXXXa",
".............aXXXXaXXXXa",
".............aXXXXXXXXXa",
".............aXXXXXXXXXa",
".............aaaaaaaaaaa"};

/* XPM */
static const char * const link_xpm[] = {
"24 30 3 1",
".        c None",
"a        c #000000",
"X        c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
"aa......................",
"aXa.....................",
"aXXa....................",
"aXXXa...................",
"aXXXXa..................",
"aXXXXXa.................",
"aXXXXXXa................",
"aXXXXXXXa...............",
"aXXXXXXXXa..............",
"aXXXXXXXXXa.............",
"aXXXXXXaaaa.............",
"aXXXaXXa................",
"aXXaaXXa................",
"aXa..aXXa...............",
"aa...aXXa...............",
"a.....aXXa..............",
"......aXXa..............",
".......aXXa.............",
".......aXXa.............",
"........aa...aaaaaaaaaaa",
#else
"XX......................",
"XaX.....................",
"XaaX....................",
"XaaaX...................",
"XaaaaX..................",
"XaaaaaX.................",
"XaaaaaaX................",
"XaaaaaaaX...............",
"XaaaaaaaaX..............",
"XaaaaaaaaaX.............",
"XaaaaaaXXXX.............",
"XaaaXaaX................",
"XaaXXaaX................",
"XaX..XaaX...............",
"XX...XaaX...............",
"X.....XaaX..............",
"......XaaX..............",
".......XaaX.............",
".......XaaX.............",
"........XX...aaaaaaaaaaa",
#endif
".............aXXXXXXXXXa",
".............aXXXaaaaXXa",
".............aXXXXaaaXXa",
".............aXXXaaaaXXa",
".............aXXaaaXaXXa",
".............aXXaaXXXXXa",
".............aXXaXXXXXXa",
".............aXXXaXXXXXa",
".............aXXXXXXXXXa",
".............aaaaaaaaaaa"};

#ifndef QT_NO_DRAGANDDROP

//#define QDND_DEBUG

#ifdef QDND_DEBUG
QString dragActionsToString(Qt::DropActions actions)
{
    QString str;
    if (actions == Qt::IgnoreAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "IgnoreAction";
    }
    if (actions & Qt::LinkAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "LinkAction";
    }
    if (actions & Qt::CopyAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "CopyAction";
    }
    if (actions & Qt::MoveAction) {
        if (!str.isEmpty())
            str += " | ";
        str += "MoveAction";
    }
    if ((actions & Qt::TargetMoveAction) == Qt::TargetMoveAction ) {
        if (!str.isEmpty())
            str += " | ";
        str += "TargetMoveAction";
    }
    return str;
}

QString KeyboardModifiersToString(Qt::KeyboardModifiers moderfies)
{
    QString str;
    if (moderfies & Qt::ControlModifier) {
        if (!str.isEmpty())
            str += " | ";
        str += Qt::ControlModifier;
    }
    if (moderfies & Qt::AltModifier) {
        if (!str.isEmpty())
            str += " | ";
        str += Qt::AltModifier;
    }
    if (moderfies & Qt::ShiftModifier) {
        if (!str.isEmpty())
            str += " | ";
        str += Qt::ShiftModifier;
    }
    return str;
}
#endif


// the universe's only drag manager
QDragManager *QDragManager::instance = 0;


QDragManager::QDragManager()
    : QObject(qApp)
{
    Q_ASSERT(!instance);
    n_cursor = 3;
    pm_cursor = new QPixmap[n_cursor];
    pm_cursor[0] = QPixmap((const char **)move_xpm);
    pm_cursor[1] = QPixmap((const char **)copy_xpm);
    pm_cursor[2] = QPixmap((const char **)link_xpm);
    object = 0;
    beingCancelled = false;
    restoreCursor = false;
    willDrop = false;
    eventLoop = 0;
    dropData = new QDropData();
}


QDragManager::~QDragManager()
{
#ifndef QT_NO_CURSOR
    if (restoreCursor)
        QApplication::restoreOverrideCursor();
#endif
    instance = 0;
    delete [] pm_cursor;
    delete dropData;
}

QDragManager *QDragManager::self()
{
    if (!instance && qApp && !qApp->closingDown())
        instance = new QDragManager;
    return instance;
}

QPixmap QDragManager::dragCursor(Qt::DropAction action) const
{
    QDragPrivate * d = dragPrivate();
    if (d && d->customCursors.contains(action))
        return d->customCursors[action];
    else if (action == Qt::MoveAction)
        return pm_cursor[0];
    else if (action == Qt::CopyAction)
        return pm_cursor[1];
    else if (action == Qt::LinkAction)
        return pm_cursor[2];
    return 0;
}

bool QDragManager::hasCustomDragCursors() const
{
    QDragPrivate * d = dragPrivate();
    return d && !d->customCursors.isEmpty();
}

Qt::DropAction QDragManager::defaultAction(Qt::DropActions possibleActions,
                                           Qt::KeyboardModifiers modifiers) const
{
    Qt::DropAction defaultAction = Qt::CopyAction;

#ifdef QDND_DEBUG
    qDebug("QDragManager::defaultAction(Qt::DropActions possibleActions)");
    qDebug("keyboard modifiers : %s", KeyboardModifiersToString(modifiers).latin1());
#endif

#ifdef Q_WS_MAC
    if (modifiers & Qt::ControlModifier && modifiers & Qt::AltModifier)
        defaultAction = Qt::LinkAction;
    else if (modifiers & Qt::AltModifier)
        defaultAction = Qt::CopyAction;
    else
        defaultAction = Qt::MoveAction;
#else
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier)
        defaultAction = Qt::LinkAction;
    else if (modifiers & Qt::ControlModifier)
        defaultAction = Qt::CopyAction;
    else if (modifiers & Qt::ShiftModifier)
        defaultAction = Qt::MoveAction;
    else if (modifiers & Qt::AltModifier)
        defaultAction = Qt::LinkAction;
#endif

    // if the object is set take the list of possibles from it
    if (object)
        possibleActions = object->d_func()->possible_actions;

#ifdef QDND_DEBUG
    qDebug("possible actions : %s", dragActionsToString(possibleActions).latin1());
#endif

    // Check if the action determined is allowed
    if (!(possibleActions & defaultAction))
        defaultAction = Qt::CopyAction;

#ifdef QDND_DEBUG
    qDebug("default action : %s", dragActionsToString(defaultAction).latin1());
#endif

    return defaultAction;
}

#endif

static QStringList imageReadMimeFormats()
{
    QStringList formats;
    QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
    for (int i = 0; i < imageFormats.size(); ++i) {
        QString format = QLatin1String("image/");
        format += QString::fromLatin1(imageFormats.at(i).toLower());
        formats.append(format);
    }

    //put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);

    return formats;
}


static QStringList imageWriteMimeFormats()
{
    QStringList formats;
    QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();
    for (int i = 0; i < imageFormats.size(); ++i) {
        QString format = QLatin1String("image/");
        format += QString::fromLatin1(imageFormats.at(i).toLower());
        formats.append(format);
    }

    //put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);

    return formats;
}

QInternalMimeData::QInternalMimeData()
    : QMimeData()
{
}

QInternalMimeData::~QInternalMimeData()
{
}

bool QInternalMimeData::hasFormat(const QString &mimeType) const
{
    bool foundFormat = hasFormat_sys(mimeType);
    if (!foundFormat && mimeType == QLatin1String("application/x-qt-image")) {
        QStringList imageFormats = imageReadMimeFormats();
        for (int i = 0; i < imageFormats.size(); ++i) {
            if ((foundFormat = hasFormat_sys(imageFormats.at(i))))
                break;
        }
    }
    return foundFormat;
}

QStringList QInternalMimeData::formats() const
{
    QStringList realFormats = formats_sys();
    if (!realFormats.contains(QLatin1String("application/x-qt-image"))) {
        QStringList imageFormats = imageReadMimeFormats();
        for (int i = 0; i < imageFormats.size(); ++i) {
            if (realFormats.contains(imageFormats.at(i))) {
                realFormats += QLatin1String("application/x-qt-image");
                break;
            }
        }
    }
    return realFormats;
}

QVariant QInternalMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
    QVariant data = retrieveData_sys(mimeType, type);
    if (mimeType == QLatin1String("application/x-qt-image")) {
	if (data.isNull()) {
	    // try to find an image
            QStringList imageFormats = imageReadMimeFormats();
            for (int i = 0; i < imageFormats.size(); ++i) {
               data = retrieveData_sys(imageFormats.at(i), QVariant::ByteArray);
		if (!data.isNull())
		    break;
	    }
	}
	if (type != data.type() && data.type() == QVariant::ByteArray) {
	    QImage image = QImage::fromData(data.toByteArray());
	    if (type == QVariant::Bitmap)
		data = QPixmap::fromImage(image);
	    else if (type == QVariant::Pixmap)
	        data = QBitmap::fromImage(image);
	    else
	        data = image;
	}
    } else if (data.type() != type && data.type() == QVariant::ByteArray) {
	// try to use mime data's internal conversion stuf.
	QInternalMimeData *that = const_cast<QInternalMimeData *>(this);
	that->setData(mimeType, data.toByteArray());
	data = QMimeData::retrieveData(mimeType, type);
	that->clear();
    }
    return data;
}

bool QInternalMimeData::canReadData(const QString &mimeType)
{
    return imageReadMimeFormats().contains(mimeType);
}

// helper functions for rendering mimedata to the system, this is needed because QMimeData is in core.
QStringList QInternalMimeData::formatsHelper(const QMimeData *data)
{
    QStringList realFormats = data->formats();
    if (realFormats.contains(QLatin1String("application/x-qt-image"))) {
        // add all supported image formats
        QStringList imageFormats = imageWriteMimeFormats();
        for (int i = 0; i < imageFormats.size(); ++i) {
            if (!realFormats.contains(imageFormats.at(i)))
                realFormats.append(imageFormats.at(i));
        }
    }
    return realFormats;
}

bool QInternalMimeData::hasFormatHelper(const QString &mimeType, const QMimeData *data)
{

    bool foundFormat = data->hasFormat(mimeType);
    if (!foundFormat) {
       if (mimeType == QLatin1String("application/x-qt-image")) {
	    // check all supported image formats
	    QStringList imageFormats = imageWriteMimeFormats();
	    for (int i = 0; i < imageFormats.size(); ++i) {
		if ((foundFormat = data->hasFormat(imageFormats.at(i))))
		    break;
	    }
	} else if (mimeType.startsWith(QLatin1String("image/"))) {
		return data->hasImage() && imageWriteMimeFormats().contains(mimeType);
	}
    }
    return foundFormat;
}

QByteArray QInternalMimeData::renderDataHelper(const QString &mimeType, const QMimeData *data)
{
    QByteArray ba = data->data(mimeType);
    if (ba.isEmpty()) {
        if (mimeType == QLatin1String("application/x-qt-image") && data->hasImage()) {
            QImage image = qvariant_cast<QImage>(data->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            // would there not be PNG ??
            image.save(&buf, "PNG");
        } else if (mimeType.startsWith(QLatin1String("image/")) && data->hasImage()) {
            QImage image = qvariant_cast<QImage>(data->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            image.save(&buf, mimeType.mid(mimeType.indexOf('/') + 1).toLatin1().toUpper());
        }
    }
    return ba;
}

QDropData::QDropData()
    : QInternalMimeData()
{
}

QDropData::~QDropData()
{
}
