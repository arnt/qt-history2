/****************************************************************************
**
** Definition of QColorDialog class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_GUI_EXPORT QColorDialog : public QDialog
{
    Q_OBJECT

public:
    static QColor getColor(const QColor& init = Qt::white, QWidget* parent=0, const char* name=0);
    static QRgb getRgba(QRgb, bool* ok = 0,
                         QWidget* parent=0, const char* name=0);

    static int customCount();
    static QRgb customColor(int);
    static void setCustomColor(int, QRgb);
    static void setStandardColor(int, QRgb);

private:
    ~QColorDialog();
    QColorDialog(QWidget* parent=0, const char* name=0, bool modal=false);

    void setColor(const QColor&);
    QColor color() const;

    bool selectColor(const QColor&);

    void setSelectedAlpha(int);
    int selectedAlpha() const;

    void showCustom(bool=true);

private:        // Disabled copy constructor and operator=
    QColorDialogPrivate *d;
    friend class QColorDialogPrivate;
    friend class QColorShower;

#if defined(Q_DISABLE_COPY)
    QColorDialog(const QColorDialog &);
    QColorDialog& operator=(const QColorDialog &);
#endif
};

#endif

#endif //QCOLORDIALOG_H
