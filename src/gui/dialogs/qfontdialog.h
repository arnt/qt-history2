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

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

#include "qwindowdefs.h"

#ifndef QT_NO_FONTDIALOG

//
//  W A R N I N G
//  -------------
//
//  This class is under development and has private constructors.
//
//  You may use the public static getFont() functions which are guaranteed
//  to be available in the future.
//

#ifndef QT_H
#include "qdialog.h"
#include "qfont.h"
#endif // QT_H

class QFontDialogPrivate;

class Q_GUI_EXPORT QFontDialog: public QDialog
{
    Q_OBJECT

public:
    static QFont getFont(bool *ok, const QFont &def,
                          QWidget* parent=0, const char* name=0);
    static QFont getFont(bool *ok, QWidget* parent=0, const char* name=0);

private:
    static QFont getFont(bool *ok, const QFont *def,
                          QWidget* parent=0, const char* name=0);

    QFontDialog(QWidget* parent=0, const char* name=0, bool modal=false,
                 Qt::WFlags f=0);
    ~QFontDialog();

    QFont font() const;
    void setFont(const QFont &font);

    bool eventFilter(QObject *, QEvent *);

    void updateFamilies();
    void updateStyles();
    void updateSizes();

private slots:
    void sizeChanged(const QString &);
    void familyHighlighted(int);
    void scriptHighlighted(int);
    void styleHighlighted(int);
    void sizeHighlighted(int);
    void updateSample();

private:
    friend class QFontDialogPrivate;
    QFontDialogPrivate * d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFontDialog(const QFontDialog &);
    QFontDialog& operator=(const QFontDialog &);
#endif
};

#endif

#endif // QFONTDIALOG_H
