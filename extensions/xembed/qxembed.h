/****************************************************************************
** $Id: //depot/qt/main/extensions/xembed/qxembed.h#2 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QXEMBED_H
#define QXEMBED_H

#include <qwidget.h>

class QXEmbed : public QWidget
{
  Q_OBJECT

public:

  QXEmbed(QWidget *parent=0, const char *name=0);

  void embed(WId w);

protected:
  void keyPressEvent( QKeyEvent * );
  void keyReleaseEvent( QKeyEvent * );
  void focusInEvent( QFocusEvent * );
  void focusOutEvent( QFocusEvent * );
  void resizeEvent(QResizeEvent *);
  void showEvent( QShowEvent * );

  bool focusNextPrevChild( bool next );

private:

 void sendFocusIn();
 void sendFocusOut();

  WId  window;

};


#endif
