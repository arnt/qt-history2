/**********************************************************************
** $Id: $
**
** Definition of QLineEdit widget class
**
** Created : 941011
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QLINEEDIT_H
#define QLINEEDIT_H

struct QLineEditPrivate;

class QValidator;
class QPopupMenu;

#ifndef QT_H
#include "qframe.h"
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_LINEEDIT

class Q_EXPORT QLineEdit : public QFrame
{
    Q_OBJECT
    Q_ENUMS( EchoMode )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int maxLength READ maxLength WRITE setMaxLength )
    Q_PROPERTY( bool frame READ frame WRITE setFrame )
    Q_PROPERTY( EchoMode echoMode READ echoMode WRITE setEchoMode )
    Q_PROPERTY( QString displayText READ displayText )
    Q_PROPERTY( int cursorPosition READ cursorPosition WRITE setCursorPosition )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( bool edited READ edited WRITE setEdited )
    Q_PROPERTY( bool hasMarkedText READ hasMarkedText DESIGNABLE false )
    Q_PROPERTY( bool hasSelectedText READ hasSelectedText )
    Q_PROPERTY( QString markedText READ markedText DESIGNABLE false )
    Q_PROPERTY( QString selectedText READ selectedText )
    Q_PROPERTY( bool dragEnabled READ dragEnabled WRITE setDragEnabled )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool undoAvailable READ isUndoAvailable )
    Q_PROPERTY( bool redoAvailable READ isRedoAvailable )

public:
    QLineEdit( QWidget* parent, const char* name=0 );
    QLineEdit( const QString &, QWidget* parent, const char* name=0 );
    ~QLineEdit();

    QString text() const;

    QString displayText() const;

    int maxLength() const;

    bool frame() const;

    enum EchoMode { Normal, NoEcho, Password };
    EchoMode echoMode() const;

    bool isReadOnly() const;

    const QValidator * validator() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    int cursorPosition() const;
    bool validateAndSet( const QString &, int, int, int );

    int alignment() const;

#ifndef QT_NO_COMPAT
    void cursorLeft( bool mark, int steps = 1 ) { cursorForward( mark, -steps ); }
    void cursorRight( bool mark, int steps = 1 ) { cursorForward( mark, steps ); }
#endif
    void cursorForward( bool mark, int steps = 1 );
    void cursorBackward( bool mark, int steps = 1 );
    void cursorWordForward( bool mark );
    void cursorWordBackward( bool mark );
    void backspace();
    void del();
    void home( bool mark );
    void end( bool mark );

    void setEdited( bool );
    bool edited() const;

    bool hasSelectedText() const;
    QString selectedText() const;
    bool getSelection( int *start, int *end );

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;


#ifndef QT_NO_COMPAT
    bool hasMarkedText() const { return hasSelectedText(); }
    QString markedText() const { return selectedText(); }
#endif

    void 	setPasswordChar( QChar c );
    QChar 	passwordChar() const;

    bool dragEnabled() const;
    int characterAt( int xpos, QChar *chr ) const;

public slots:
    virtual void setText( const QString &);
    virtual void selectAll();
    virtual void deselect();
    virtual void clearValidator();
    virtual void insert( const QString &);
    virtual void clear();
    virtual void undo();
    virtual void redo();
    virtual void setMaxLength( int );
    virtual void setFrame( bool );
    virtual void setEchoMode( EchoMode );
    virtual void setReadOnly( bool );
    virtual void setValidator( const QValidator * );
    virtual void setFont( const QFont & );
    virtual void setPalette( const QPalette & );
    virtual void setSelection( int, int );
    virtual void setCursorPosition( int );
    virtual void setAlignment( int flag );
#ifndef QT_NO_CLIPBOARD
    virtual void cut();
    virtual void copy() const;
    virtual void paste();
#endif
    virtual void setDragEnabled( bool b );

signals:
    void textChanged( const QString &);
    void returnPressed();
    void selectionChanged();

protected:
    bool event( QEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseDoubleClickEvent( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );
    void imStartEvent( QIMEvent * );
    void imComposeEvent( QIMEvent * );
    void imEndEvent( QIMEvent * );
    void focusInEvent( QFocusEvent * );
    void focusOutEvent( QFocusEvent * );
    void drawContents( QPainter *painter );
    void resizeEvent( QResizeEvent * );
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent *e );
    void dragLeaveEvent( QDragLeaveEvent *e );
    void dropEvent( QDropEvent * );
#endif
    void contextMenuEvent( QContextMenuEvent * );

#ifndef QT_NO_COMPAT
    void repaintArea( int, int ) { update(); }
#endif

    virtual QPopupMenu *createPopupMenu();
    void windowActivationChange( bool );

private slots:
    void clipboardChanged();
    void blinkSlot();
#ifndef QT_NO_DRAGANDDROP
    void doDrag();
#endif
    void dragSlot();
    void popupActivated( int r );

private:
    void init();
    void blinkOn();
    void updateOffset();
    void updateSelection();
    void removeSelectedText();
    void delOrBackspace( bool backspace );

    QLineEditPrivate * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLineEdit( const QLineEdit & );
    QLineEdit &operator=( const QLineEdit & );
#endif
};


#endif // QT_NO_LINEEDIT

#endif // QLINEEDIT_H
