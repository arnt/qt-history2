/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfontdialog.h#2 $
**
** Definition of 
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

class QListBox;

#include "qdialog.h"
#include "qfont.h"

class QFontDialogPrivate;

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

#endif
