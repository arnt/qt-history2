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
class QStyleSheetItem;

class QTextEdit : public QScrollView
{
    Q_OBJECT

public:
    enum ParagType {
	Normal = 0,
	BulletList,
	EnumList
    };

    QTextEdit( QWidget *parent, const QString &fn, bool tabify = FALSE );
    QTextEdit( QWidget *parent = 0, const char *name = 0 );
    virtual ~QTextEdit();

#if defined(QTEXTEDIT_OPEN_API)
    QTextEditDocument *document() const;
#endif

    QString text() const;
    QString text( int parag, bool formatted = FALSE ) const;
    Qt::TextFormat textFormat() const;
    QString fileName() const;

    void cursorPosition( int &parag, int &index );
    void selection( int &parag_from, int &index_from,
		    int &parag_to, int &index_to );
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward = TRUE,
		       int *parag = 0, int *index = 0 );
    void insert( const QString &text, bool indent = FALSE, bool checkNewLine = FALSE );

    int paragraphs() const;
    int lines() const;
    int linesOfParagraph( int parag ) const;
    int lineOfChar( int parag, int chr );

    bool isReadOnly() const;
    bool isModified() const;

    bool italic() const;
    bool bold() const;
    bool underline() const;
    QString family() const;
    int pointSize() const;
    QColor color() const;
    QFont font() const;
    ParagType paragType() const;
    int alignment() const;

public slots:
    virtual void undo();
    virtual void redo();

    virtual void cut();
    virtual void copy();
    virtual void paste();

    virtual void indent();

    virtual void setItalic( bool b );
    virtual void setBold( bool b );
    virtual void setUnderline( bool b );
    virtual void setFamily( const QString &f );
    virtual void setPointSize( int s );
    virtual void setColor( const QColor &c );
    virtual void setFont( const QFont &f );
    virtual void setFormat( QStyleSheetItem *f );

    virtual void setParagType( ParagType t );
    virtual void setAlignment( int );

    virtual void setTextFormat( Qt::TextFormat f );
    virtual void setText( const QString &txt ) { setText( txt, FALSE ); }
    virtual void setText( const QString &txt, bool tabify );

    virtual void load( const QString &fn ) { load( fn, FALSE ); }
    virtual void load( const QString &fn, bool tabify );
    virtual void save() { save( QString::null ); }
    virtual void save( const QString &fn );

    virtual void setCursorPosition( int parag, int index );
    virtual void setSelection( int parag_from, int index_from,
			       int parag_to, int index_to );

    virtual void setReadOnly( bool ro );
    virtual void setModified( bool m );

signals:
    void currentFontChanged( const QFont &f );
    void currentColorChanged( const QColor &c );
    void currentAlignmentChanged( int );
    void currentParagTypeChanged( QTextEdit::ParagType );
    void textChanged();

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
    void setModified();

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
#if !defined(QTEXTEDIT_OPEN_API)
    QTextEditDocument *document() const;
#endif

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
    int currentAlignment;
    ParagType currentParagType;
    bool inDoubleClick;
    QPoint oldMousePos, mousePos;
    QPixmap *buf_pixmap;
    bool cursorVisible, blinkCursorVisible;
    bool readOnly, modified;

};

inline QTextEditDocument *QTextEdit::document() const
{
    return doc;
}

#endif
