/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtaddlangdia.h#2 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTADDLANG
#define QTADDLANG

#include <qdialog.h>
#include <qstring.h>

class QResizeEvent;
class QVBox;
class QComboBox;

/****************************************************************************
 *
 * Class: QTAddLangDia
 *
 ****************************************************************************/

class QTAddLangDia : public QDialog
{
    Q_OBJECT

public:
    QTAddLangDia( QWidget *parent, const char *name = 0 );

protected:
    void resizeEvent( QResizeEvent *e );

    QVBox *back;
    QComboBox *langCombo;

protected slots:
    void slotOK();

signals:
    void newLangChosen( const QString &lang );

};

#endif
