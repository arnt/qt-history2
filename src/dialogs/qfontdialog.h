/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfontdialog.h#4 $
**
** Definition of QFontDialog
**
** Created : 970605
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#include "qfont.h"
#endif // QT_H

class  QListBox;
struct QFontDialogPrivate;


class QFontDialog: public QDialog
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
    void familyHighlighted( const char * );
    void styleHighlighted( const char * );
    void sizeHighlighted( const char * );

private:
    QFontDialogPrivate * d;
};


#endif // QFONTDIALOG_H
