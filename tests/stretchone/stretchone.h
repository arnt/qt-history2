#include <qframe.h>
#include <qscrollview.h>

class Stretch : public QFrame {
	Q_OBJECT
public:
	Stretch( QWidget* parent=0, const char* name=0 );
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	QSize minimumSize() const;
	QSize maximumSize() const;
	void resize( int w, int h );
};

class MyScrollView : public QScrollView {
	Q_OBJECT
public:
	MyScrollView( QWidget* parent=0, const char* name=0, WFlags f=0 );
	~MyScrollView();
	void setResizePolicy( ResizePolicy r );
	void addChild( QWidget* child, int x, int y );
	void moveChild( QWidget* child, int x, int y );
	void resizeEvent( QResizeEvent* e );
	bool eventFilter( QObject *, QEvent *e );
	void show();

public slots:
	void resizeContents( int w, int h );
    	void setContentsPos( int x, int y );
	void setResizePolicySlot( int id );
};
