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

#include "qdialog.h"
#include "qfont.h"

class QFontDialogPrivate;

class Q_GUI_EXPORT QFontDialog: public QDialog
{
    Q_OBJECT

public:
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent=0);
    static QFont getFont(bool *ok, QWidget* parent=0);

#ifdef QT_COMPAT
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getFont(ok, def, parent); }
    static QFont getFont(bool *ok, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getFont(ok, parent); }
#endif

private:
    static QFont getFont(bool *ok, const QFont *def, QWidget* parent=0);

    QFontDialog(QWidget* parent=0, bool modal=false, Qt::WFlags f=0);
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
    Q_DISABLE_COPY(QFontDialog)

    QFontDialogPrivate *d;

    friend class QFontDialogPrivate;
};

#endif

#endif // QFONTDIALOG_H
