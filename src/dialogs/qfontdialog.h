/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfontdialog.h#10 $
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

//
//  W A R N I N G
//  -------------
//
//  This class is under development and is currently unstable.
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

#ifndef QT_H
#include "qdialog.h"
#include "qfont.h"
#endif // QT_H

class  QListBox;
class  QComboBox;
struct QFontDialogPrivate;


class Q_EXPORT QFontDialog: public QDialog
{
    Q_OBJECT
public:
    QFontDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		 WFlags f=0 );
   ~QFontDialog();

    static QFont getFont( bool *ok, const QFont &def,
			    QWidget *parent = 0, const char* name = 0);

    static QFont getFont( bool *ok, QWidget *parent = 0, const char* name = 0);


    QFont font() const;
    void setFont( const QFont &font );

protected:
    bool eventFilter( QObject *, QEvent * );

    QListBox * familyListBox() const;
    virtual void updateFamilies();

    QListBox * styleListBox() const;
    virtual void updateStyles();

    QListBox * sizeListBox() const;
    virtual void updateSizes();

    QComboBox * scriptCombo() const;
    virtual void updateScripts();

protected slots:
    void familySelected();
    void scriptSelected();
    void styleSelected();
    void sizeSelected();

private slots:
    void familyHighlighted( const QString &);
    void scriptHighlighted( const QString &);
    void styleHighlighted( const QString &);
    void sizeHighlighted( const QString &);
    void updateSample();

private:
    static QFont getFont( bool *ok, const QFont *def,
			  QWidget *parent = 0, const char* name = 0);

    QFontDialogPrivate * d;
};


#endif // QFONTDIALOG_H
