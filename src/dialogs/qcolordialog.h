/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qcolordialog.h#9 $
**
** Definition of QColorDialog class
**
** Created : 990222
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#ifndef QT_H
#include <qdialog.h>
#endif // QT_H

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_EXPORT QColorDialog : public QDialog
{
    Q_OBJECT

public:
    static QColor getColor( QColor, QWidget *parent=0, const char* name=0 );
    static QRgb getRgba( QRgb, bool* ok = 0,
			 QWidget *parent=0, const char* name=0 );


    static int customCount();
    static QRgb customColor( int );
    static void setCustomColor( int, QRgb );

private:
    ~QColorDialog();

    QColorDialog( QWidget* parent=0, const char* name=0, bool modal=FALSE );
    void setColor( QColor );
    QColor color() const;

private:
    void setSelectedAlpha( int );
    int selectedAlpha() const;

    void showCustom( bool=TRUE );
private:
    QColorDialogPrivate *d;
    friend class QColorDialogPrivate;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QColorDialog( const QColorDialog & );
    QColorDialog& operator=( const QColorDialog & );
#endif
};

#endif

#endif //QCOLORDIALOG_H
