/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MSGEDIT_H
#define MSGEDIT_H

#include "trwindow.h"
#include "phrase.h"

#include <metatranslator.h>

#include <qsplitter.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qevent.h>

class QAccel;
class QDockArea;
class QDockWindow;
class QLabel;
class QListView;
class QListViewItem;
class QTextView;
class QVBoxLayout;

class EditorPage;
class MED;
class PhraseLV;

class PageCurl : public QWidget
{
    Q_OBJECT
public:
    PageCurl( QWidget * parent = 0, const char * name = 0,
	      WFlags f = 0 )
	: QWidget( parent, name, f )
    {
	QPixmap px = TrWindow::pageCurl();
	if ( px.mask() ) {
	    setMask( *px.mask() );
	}
	setBackgroundPixmap( px );
	setFixedSize( px.size() );

	QRect r( 34, 0, width()-34, 19 );
	QToolTip::add( this, r, tr("Next unfinished phrase") );
	r.setSize( QSize(width()-34, height()-20) );
	r.setX( 0 );
	r.setY( 20 );
	QToolTip::add( this, r, tr("Previous unfinished phrase") );
    }

protected:
    void mouseReleaseEvent( QMouseEvent * e )
    {
	int x = e->pos().x() - 14;
	int y = e->pos().y() - 8;

	if ( y <= x )
	    emit nextPage();
	else
	    emit prevPage();
    }

signals:
    void prevPage();
    void nextPage();
};

class ShadowWidget : public QWidget
{
public:
    ShadowWidget( QWidget * parent = 0, const char * name = 0 );
    ShadowWidget( QWidget * child, QWidget * parent = 0, const char * name = 0 );

    void setShadowWidth( int width ) { sWidth = width; }
    int  shadowWidth() const { return sWidth; }
    void setMargin( int margin ){ wMargin = margin; }
    int  margin() const { return wMargin; }
    void setWidget( QWidget * child);

protected:
    void resizeEvent( QResizeEvent * e );
    void paintEvent( QPaintEvent * e );

private:
    int sWidth;
    int wMargin;
    QWidget * childWgt;
};

class EditorPage : public QFrame
{
    Q_OBJECT
public:
    EditorPage( QWidget * parent = 0, const char * name = 0 );

protected:
    void resizeEvent( QResizeEvent * );
    void layoutWidgets();
    void updateCommentField();
    void calculateFieldHeight( QTextView * field );
    void fontChange( const QFont & );

private:
    PageCurl * pageCurl;
    QLabel * srcTextLbl;
    QLabel * transLbl;
    QTextView * srcText;
    QTextView * cmtText;
    MED   * translationMed;

    friend class MessageEditor;

private slots:
    void handleTranslationChanges();
    void handleSourceChanges();
    void handleCommentChanges();

signals:
    void pageHeightUpdated( int height );
};

class MessageEditor : public QWidget
{
    Q_OBJECT
public:
    MessageEditor( MetaTranslator * t, QWidget * parent = 0,
		   const char * name = 0 );
    QListView * sourceTextList() const;
    QListView * phraseList() const;

    void showNothing();
    void showContext( const QString& context, bool finished );
    void showMessage( const QString& text, const QString& comment,
		      const QString& fullContext, const QString& translation,
		      MetaTranslatorMessage::Type type,
		      const QList<Phrase>& phrases );
    void setFinished( bool finished );
    bool eventFilter( QObject *, QEvent * );

signals:
    void translationChanged( const QString& translation );
    void finished( bool finished );
    void prevUnfinished();
    void nextUnfinished();
    void updateActions( bool enable );

    void undoAvailable( bool avail );
    void redoAvailable( bool avail );
    void cutAvailable( bool avail );
    void copyAvailable( bool avail );
    void pasteAvailable( bool avail );

    void focusSourceList();
    void focusPhraseList();

public slots:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void del();
    void selectAll();
    void beginFromSource();
    void toggleGuessing();
    void finishAndNext();

private slots:
    void emitTranslationChanged();
    void guessActivated( int accelKey );
    void insertPhraseInTranslation( QListViewItem *item );
    void insertPhraseInTranslationAndLeave( QListViewItem *item );
    void updateButtons();
    void updateCanPaste();
    void toggleFinished();

    void updatePageHeight( int height );

protected:
    void resizeEvent( QResizeEvent * );

private:
    void setTranslation( const QString& translation, bool emitt );
    void setEditionEnabled( bool enabled );

    QListView * srcTextList;
    QDockArea * topDock, * bottomDock;
    QDockWindow * topDockWnd, *bottomDockWnd;
    EditorPage * editorPage;
    QVBoxLayout * v;

    QLabel * phraseLbl;
    PhraseLV * phraseLv;
    QAccel * accel;
    bool itemFinished;

    ShadowWidget * sw;
    QScrollView * sv;

    MetaTranslator *tor;
    QString sourceText;
    QStringList guesses;
    bool mayOverwriteTranslation;
    bool canPaste;
    bool doGuesses;
};

#endif
