#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#include <qscrollview.h>

class QPainter;
class QTextEditDocument;
class QTextEditCursor;
class QKeyEvent;
class QResizeEvent;
class QMouseEvent;
class QTimer;
class QTextEditString;
class QVBox;
class QListBox;
class QTextEditCommand;
class QTextEditParag;
class QTextEditFormat;
class QFont;
class QColor;

class QTextEdit : public QScrollView
{
    Q_OBJECT

public:
    QTextEdit( QWidget *parent, const QString &fn, bool tabify = FALSE );
    QTextEdit( QWidget *parent, const QString &text );
    virtual ~QTextEdit();

    QTextEditDocument *document() const;
    void insert( const QString &text, bool indent = FALSE, bool checkNewLine = FALSE );

    void undo();
    void redo();

    void cut();
    void copy();
    void paste();

    void indent();

    void setItalic( bool b );
    void setBold( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setColor( const QColor &c );
    void setFont( const QFont &f );

    void setParagType( int );
    void setAlignment( int );

    QString text() const;
    QString text( int parag, bool formatted = FALSE ) const;
    virtual void setText( const QString &txt );

    bool find( const QString &expr, bool cs, bool wo, bool forward = TRUE,
	       int *parag = 0, int *index = 0 );

signals:
    void currentFontChanged( const QFont &f );
    void currentColorChanged( const QColor &c );
    void currentAlignmentChanged( int );
    void currentParagTypeChanged( int );

protected:
    void setFormat( QTextEditFormat *f, int flags );
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void keyPressEvent( QKeyEvent *e );
    void resizeEvent( QResizeEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    bool eventFilter( QObject *o, QEvent *e );
    bool focusNextPrevChild( bool next );

private slots:
    void formatMore();
    void doResize();
    void doAutoScroll();
    void doChangeInterval();
    void blinkCursor();
    
private:
    enum MoveDirection {
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	MoveHome,
	MoveEnd,
	MovePgUp,
	MovePgDown
    };
    enum KeyboardAction {
	ActionBackspace,
	ActionDelete,
	ActionReturn
    };

    struct UndoRedoInfo {
	enum Type { Invalid, Insert, Delete, Backspace, Return, RemoveSelected };
	UndoRedoInfo( QTextEditDocument *d ) : type( Invalid ), doc( d )
	{ text = QString::null; id = -1; index = -1; }
	void clear();
	inline bool valid() const { return !text.isEmpty() && id >= 0&& index >= 0; }

    	QString text;
	int id;
	int index;
	Type type;
	QTextEditDocument *doc;
    };

private:
    QPixmap *bufferPixmap( const QSize &s );
    void init();
    void ensureCursorVisible();
    void drawCursor( bool visible );
    void placeCursor( const QPoint &pos, QTextEditCursor *c = 0 );
    void moveCursor( int direction, bool shift, bool control );
    void moveCursor( int direction, bool control );
    void removeSelectedText();
    void doKeyboardAction( int action );
    bool doCompletion();
    void checkUndoRedoInfo( UndoRedoInfo::Type t );
    void repaintChanged();
    void updateCurrentFormat();

private:
    QTextEditDocument *doc;
    QTextEditCursor *cursor;
    bool drawAll;
    bool mousePressed;
    QTimer *formatTimer, *scrollTimer, *changeIntervalTimer, *blinkTimer;
    QTextEditParag *lastFormatted;
    int interval;
    QVBox *completionPopup;
    QListBox *completionListBox;
    int completionOffset;
    UndoRedoInfo undoRedoInfo;
    QTextEditFormat *currentFormat;
    QPainter painter;
    QPixmap *doubleBuffer;
    int currentAlignment, currentParagType;
    bool inDoubleClick;
    QPoint oldMousePos, mousePos;
    QPixmap *buf_pixmap;
    bool cursorVisible, blinkCursorVisible;

};

inline QTextEditDocument *QTextEdit::document() const
{
    return doc;
}

#endif
