#ifndef QFORM_H
#define QFORM_H

#ifndef QT_H
#include <qframe.h>
#endif

class QScrollBar;

class QForm : public QFrame
{
    Q_OBJECT
public:
    QForm( QWidget* _parent = 0, const char* _name = 0 );
    virtual ~QForm();
  
    QWidget* viewport() { return view; };
    QScrollBar* horizontalScrollBar() { return hbar; }
    QScrollBar* verticalScrollBar() { return vbar; }

private slots:
    void scrollHor( int );
    void scrollVer( int );
  
protected:
    virtual void resizeEvent( QResizeEvent* _ev );
    virtual bool event( QEvent *e );
    virtual void wheelEvent( QWheelEvent *e );
  
private:
    void updateLayout();
  
    QScrollBar *hbar;
    QScrollBar *vbar;
    QWidget *view;
};

#endif
