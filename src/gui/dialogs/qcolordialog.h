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

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#include "qdialog.h"

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_GUI_EXPORT QColorDialog : public QDialog
{
    Q_OBJECT

public:
    static QColor getColor(const QColor& init = Qt::white, QWidget* parent=0);
    static QRgb getRgba(QRgb, bool* ok = 0, QWidget* parent=0);

    static int customCount();
    static QRgb customColor(int);
    static void setCustomColor(int, QRgb);
    static void setStandardColor(int, QRgb);

#ifdef QT_COMPAT
    static QColor getColor(const QColor& init, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getColor(init, parent); }
    static QRgb getRgba(QRgb rgba, bool* ok, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getRgba(rgba, ok, parent); }
#endif

private:
    ~QColorDialog();
    QColorDialog(QWidget* parent=0, bool modal=false);

    void setColor(const QColor&);
    QColor color() const;

    bool selectColor(const QColor&);

    void setSelectedAlpha(int);
    int selectedAlpha() const;

    void showCustom(bool=true);

private:
    Q_DISABLE_COPY(QColorDialog)

    QColorDialogPrivate *d;

    friend class QColorDialogPrivate;
    friend class QColorShower;
};

#endif

#endif //QCOLORDIALOG_H
