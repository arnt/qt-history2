/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qcolordialog.h#3 $
**
** Definition of QColorDialog class
**
** Created : 990222
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

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#ifndef QT_H
#include <qdialog.h>
#endif // QT_H

class QColorDialogPrivate;

class Q_EXPORT QColorDialog : public QDialog {
    Q_OBJECT
public:
    QColorDialog( QWidget* parent=0, const char* name=0, bool modal=FALSE );
    ~QColorDialog();
    void setSelectedColor( QColor );
    QColor selectedColor() const;

    static QColor getColor( QColor, QWidget *parent=0, const char* name=0 );
private:
     QColorDialogPrivate *d;
};

#endif //QCOLORDIALOG_H
