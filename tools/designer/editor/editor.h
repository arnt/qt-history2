/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include <qtextedit.h>

struct Config;
class ParenMatcher;
class EditorCompletion;
class EditorBrowser;
class QAccel;

class Editor : public QTextEdit
{
    Q_OBJECT

public:
    enum Selection {
	Error = 3,
	Step = 4
    };

    Editor( const QString &fn, QWidget *parent, const char *name );
    ~Editor();
    virtual void load( const QString &fn );
    virtual void save( const QString &fn );
    Q3TextDocument *document() const { return QTextEdit::document(); }
    void placeCursor( const QPoint &p, Q3TextCursor *c ) { QTextEdit::placeCursor( p, c ); }
    void setDocument( Q3TextDocument *doc ) { QTextEdit::setDocument( doc ); }
    Q3TextCursor *textCursor() const { return QTextEdit::textCursor(); }
    void repaintChanged() { QTextEdit::repaintChanged(); }

    virtual EditorCompletion *completionManager() { return 0; }
    virtual EditorBrowser *browserManager() { return 0; }
    virtual void configChanged();

    Config *config() { return cfg; }

    void setErrorSelection( int line );
    void setStepSelection( int line );
    void clearStepSelection();
    void clearSelections();

    virtual bool supportsErrors() const { return TRUE; }
    virtual bool supportsBreakPoints() const { return TRUE; }
    virtual void makeFunctionVisible( Q3TextParagraph * ) {}

    void drawCursor( bool b ) { QTextEdit::drawCursor( b ); }

    QPopupMenu *createPopupMenu( const QPoint &p );
    bool eventFilter( QObject *o, QEvent *e );

    void setEditable( bool b ) { editable = b; }

protected:
    void doKeyboardAction( KeyboardAction action );
    void keyPressEvent( QKeyEvent *e );

signals:
    void clearErrorMarker();
    void intervalChanged();

private slots:
    void cursorPosChanged( Q3TextCursor *c );
    void doChangeInterval();
    void commentSelection();
    void uncommentSelection();

protected:
    ParenMatcher *parenMatcher;
    QString filename;
    Config *cfg;
    bool hasError;
    QAccel *accelComment, *accelUncomment;
    bool editable;

};

#endif
