/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtmessageview.h#2 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTMESSAGEVIEW
#define QTMESSAGEVIEW

#include <qlistview.h>
#include <qmultilinedit.h>

class QTMessageView;

class QKeyEvent;
class QMouseEvent;
class QFocusEvent;

/****************************************************************************
 *
 * Class: QTMessageViewEdit
 *
 ****************************************************************************/

class QTMessageViewEdit : public QMultiLineEdit
{
    Q_OBJECT

public:
    QTMessageViewEdit( QTMessageView *parent );

protected:
    void keyPressEvent( QKeyEvent *e );
    void focusOutEvent( QFocusEvent *e );

signals:
    void ctrlReturnPressed();
    void escapePressed();

};

/****************************************************************************
 *
 * Class: QTMessageViewItem
 *
 ****************************************************************************/

class QTMessageViewItem : public QListViewItem
{
public:
    QTMessageViewItem( QListView *parent );

    virtual int height() const;

};

/****************************************************************************
 *
 * Class: QTMessageView
 *
 ****************************************************************************/

class QTMessageView : public QListView
{
    Q_OBJECT

public:
    QTMessageView( QWidget *parent, const char *name = 0 );

protected:
    void viewportMousePressEvent( QMouseEvent *e );

    QTMessageViewEdit *editBox;
    int editCol;

protected slots:
    void startEdit( int x );
    void editNext();
    void stopEdit();

};

#endif
