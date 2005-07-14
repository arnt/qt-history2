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

#include "QtGui/qdialog.h"

QT_MODULE(Gui)

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_GUI_EXPORT QColorDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QColorDialog)

public:
    static QColor getColor(const QColor& init = Qt::white, QWidget* parent=0);
    static QRgb getRgba(QRgb, bool* ok = 0, QWidget* parent=0);

    static int customCount();
    static QRgb customColor(int);
    static void setCustomColor(int, QRgb);
    static void setStandardColor(int, QRgb);

#ifdef QT3_SUPPORT
    static QColor getColor(const QColor& init, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getColor(init, parent); }
    static QRgb getRgba(QRgb rgba, bool* ok, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getRgba(rgba, ok, parent); }
#endif

private:
    ~QColorDialog();
    explicit QColorDialog(QWidget* parent=0, bool modal=false);

    void setColor(const QColor&);
    QColor color() const;

    bool selectColor(const QColor&);

    void setSelectedAlpha(int);
    int selectedAlpha() const;

    void showCustom(bool=true);

private:
    Q_DISABLE_COPY(QColorDialog)

    Q_PRIVATE_SLOT(d_func(), void addCustom())

    Q_PRIVATE_SLOT(d_func(), void newHsv(int h, int s, int v))
    Q_PRIVATE_SLOT(d_func(), void newColorTypedIn(QRgb rgb))
    Q_PRIVATE_SLOT(d_func(), void newCustom(int, int))
    Q_PRIVATE_SLOT(d_func(), void newStandard(int, int))

    friend class QColorShower;
};

#endif

#endif //QCOLORDIALOG_H
