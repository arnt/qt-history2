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

#ifndef QPROGRESSBAR_H
#define QPROGRESSBAR_H

#include "QtGui/qframe.h"

#ifndef QT_NO_PROGRESSBAR


class QProgressBarPrivate;

class Q_GUI_EXPORT QProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool textVisible READ isTextVisible WRITE setTextVisible)

public:
    explicit QProgressBar(QWidget *parent = 0);

    int minimum() const;
    int maximum() const;

    void setRange(int minimum, int maximum);
    int value() const;

    virtual QString text() const;
    void setTextVisible(bool visible);
    bool isTextVisible() const;

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    void reset();
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setValue(int value);

signals:
    void valueChanged(int value);

protected:
    void paintEvent(QPaintEvent *);

private:
    Q_DECLARE_PRIVATE(QProgressBar)
    Q_DISABLE_COPY(QProgressBar)
};

#endif // QT_NO_PROGRESSBAR

#endif // QPROGRESSBAR_H
