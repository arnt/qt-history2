/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#ifndef STYLEDBUTTON_H
#define STYLEDBUTTON_H

#include <qbutton.h>
#include <qpixmap.h>

class QColor;
class QBrush;
class FormWindow;

class StyledButton : public QButton
{
    Q_OBJECT

    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    Q_PROPERTY( EditorType editor READ editor WRITE setEditor )
    Q_PROPERTY( bool scale READ scale WRITE setScale )

    Q_ENUMS( EditorType )

public:
    enum EditorType { NoEditor, ColorEditor, PixmapEditor };

    StyledButton( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    StyledButton( const QBrush& b, QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    ~StyledButton();

    void setEditor( EditorType );
    EditorType editor() const;

    void setColor( const QColor& );
    void setPixmap( const QPixmap& );

    QPixmap* pixmap() const;
    QColor color() const;

    void setScale( bool );
    bool scale() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setFormWindow( FormWindow *fw ) { formWindow = fw; }
    
signals:
    void changed();

protected:
    void drawButton( QPainter* );
    void drawButtonLabel( QPainter* );
    void resizeEvent( QResizeEvent* );
    void scalePixmap();

private:
    QPixmap* pix;
    QPixmap* spix;  // the pixmap scaled down to fit into the button
    QColor col;
    EditorType edit;
    bool s;
    FormWindow *formWindow;
    
private slots:
    void onEditor();

};

#endif //STYLEDBUTTON_H
