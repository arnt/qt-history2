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

#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QColorDialog>
#include <QFileDialog>
#include <QImageReader>
#include <QDebug>

#include "styledbutton.h"

StyledButton::StyledButton(QWidget *parent, ButtonType type)
    : QToolButton(parent), btype(type)
{
    connect(this, SIGNAL(clicked()), this, SLOT(onEditor()));
    mBrush = QBrush(Qt::darkGray);
}

const QBrush &StyledButton::brush() const
{
    return mBrush;
}

void StyledButton::setBrush(const QBrush &b)
{
    mBrush = b;

    if (btype == PixmapButton)
        mBrush.setColor(Qt::darkGray);

    update();
}

void StyledButton::setButtonType(ButtonType type)
{
    btype = type;
    update();
}

void StyledButton::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);
    QStyleOptionToolButton button;
    initStyleOption(&button);
    const QRect contentRect = style()->subControlRect(QStyle::CC_ToolButton, &button, QStyle::SC_ToolButton, this);

    QPainter paint(this);

    if (btype == ColorButton)
        paint.setBrush(QBrush(mBrush.color()));
    else
        paint.setBrush(mBrush);

    paint.drawRect(contentRect.left()+2, contentRect.top()+2, contentRect.width()-5, contentRect.height()-5);
}

QString StyledButton::buildImageFormatList() const
{
    QString filter;

    QString all = tr("All Pixmaps (");
    const QList<QByteArray> formats = QImageReader::supportedImageFormats();

    for (int i=0; i<formats.count(); ++i) {
        QString outputFormat = formats.at(i);
        QString outputExtension;

        if (outputFormat != "JPEG") {
            outputExtension = outputFormat.toLower();
        } else {
            outputExtension = "jpg;*.jpeg";
        }

        filter += tr("%1-Pixmaps (%2)\n").arg(outputFormat).arg("*." + outputExtension);
        all += "*." + outputExtension + ";";
    }
    filter.prepend(all + tr(")\n"));

    filter += tr("All Files (*.*)");

    return filter;
}

bool StyledButton::openPixmap()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), buildImageFormatList());

    if (!fileName.isEmpty()) {
        pixFile = fileName;
        return true;
    }

    return false;
}

QString StyledButton::pixmapFileName() const
{
    return pixFile;
}

void StyledButton::onEditor()
{
    qDebug() << "onEditor";
    if (btype == ColorButton) {
        QColor c = QColorDialog::getColor(mBrush.color(), this);
        if (c.isValid()) {
            mBrush.setColor(c);
            emit changed();
        }
    }
    else if (openPixmap()) {
        emit changed();
    }
}
