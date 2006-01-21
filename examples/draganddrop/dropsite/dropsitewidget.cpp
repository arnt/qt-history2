/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "dropsitewidget.h"
#include "dropsitewindow.h"

DropSiteWidget::DropSiteWidget(QWidget *parent)
    : QLabel(parent)
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Dark);

    setText(tr("<drop content>"));
    setAlignment(Qt::AlignCenter);
}

void DropSiteWidget::dragEnterEvent(QDragEnterEvent *event)
{
    setText(tr("<drop content>"));
    setBackgroundRole(QPalette::Light);

    event->acceptProposedAction();
    emit changed(event->mimeData());
}

void DropSiteWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QStringList formats = mimeData->formats();

    foreach (QString format, formats)
    {
        if (format.startsWith("image/")) {
            QPixmap pixmap = createPixmap(mimeData->data(format), format);
            if (!pixmap.isNull()) {
                setPixmap(pixmap);
                break;
            }
        }
        QString text = createPlainText(mimeData->data(format), format);
        if (!text.isEmpty()) {
            setText(text);
            break;
        } else {
            setText(tr("No supported format"));
        }
    }

    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}

void DropSiteWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    clear();
    event->accept();
}

void DropSiteWidget::clear()
{
    setText(tr("<drop content>"));
    setBackgroundRole(QPalette::Dark);

    emit changed();
}

QPixmap DropSiteWidget::createPixmap(QByteArray data, QString format)
{
    QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
    QPixmap pixmap;

    foreach (QByteArray imageFormat, imageFormats) {
        if (format.mid(6) == QString(imageFormat)) {
            pixmap.loadFromData(data, imageFormat);
            break;
        }
    }
    return pixmap;
}

QString DropSiteWidget::createPlainText(QByteArray data, QString format)
{
    QString text = "";

    if (format.startsWith("text/plain")) {
        text.append(QString(data));
    } else if (format.startsWith("text/plain;")) {
        int index = format.indexOf('=');
        if (index > 0) {
            QTextCodec *codec = QTextCodec::codecForName(format.mid(index + 1).toAscii());
            text.append(QString(codec->toUnicode(data)));
        }
    }
    return text;
}


