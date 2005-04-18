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

#include "QtGui/qwindowdefs.h"

#ifndef QT_NO_FONTDIALOG

#include "QtGui/qdialog.h"
#include "QtGui/qfont.h"

class QFontDialogPrivate;

class Q_GUI_EXPORT QFontDialog: public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFontDialog)

public:
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent=0);
    static QFont getFont(bool *ok, QWidget* parent=0);

#ifdef QT3_SUPPORT
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getFont(ok, def, parent); }
    static QFont getFont(bool *ok, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getFont(ok, parent); }
#endif

private:
    static QFont getFont(bool *ok, const QFont *def, QWidget* parent=0);

    explicit QFontDialog(QWidget* parent=0, bool modal=false, Qt::WFlags f=0);
    ~QFontDialog();

    QFont font() const;
    void setFont(const QFont &font);

    bool eventFilter(QObject *, QEvent *);

    void updateFamilies();
    void updateStyles();
    void updateSizes();

private:
    Q_PRIVATE_SLOT(d_func(), void sizeChanged(const QString &))
    Q_PRIVATE_SLOT(d_func(), void familyHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void writingSystemHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void styleHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void sizeHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void updateSample())

    Q_DISABLE_COPY(QFontDialog)
};

#endif

#endif // QFONTDIALOG_H
