/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfontdialog.h#9 $
**
** Definition of QFontDialog
**
** Created : 970605
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#include "qfont.h"
#endif // QT_H

class  QListBox;
struct QFontDialogPrivate;


class Q_EXPORT QFontDialog: public QDialog
{
    Q_OBJECT
public:
    QFontDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		 WFlags f=0 );
   ~QFontDialog();

    void show();

    bool eventFilter( QObject *, QEvent * );

protected:
    QListBox * fontFamilyListBox() const;
    virtual void updateFontFamilies();

    QListBox * fontStyleListBox() const;
    virtual void updateFontStyles();

    QListBox * fontSizeListBox() const;
    virtual void updateFontSizes();

    void updateGeometry();

protected slots:
    void familySelected();
    void styleSelected();
    void sizeSelected();

private slots:
    void familyHighlighted( const QString &);
    void styleHighlighted( const QString &);
    void sizeHighlighted( const QString &);

private:
    QFontDialogPrivate * d;
};


#endif // QFONTDIALOG_H
