/****************************************************************************
** $Id: //depot/qt/main/examples/widgets/widgets.h#3 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef WIDGETS_H
#define WIDGETS_H

#include <qmainwindow.h>
#include <qmovie.h>
#include <qlistview.h>
class QLabel;
class QCheckBox;
class QProgressBar;
class QTabWidget;
class QGroupBox;
class QMultiLineEdit;
class QPopupMenu;

class MyListView : public QListView
{
    Q_OBJECT
public:
    MyListView( QWidget * parent = 0, const char *name = 0 )
	: QListView( parent, name ), selected(0)
    {}
    ~MyListView()
    {}
protected:

    void contentsMousePressEvent( QMouseEvent * e )
    {
	selected = selectedItem();
	QListView::contentsMousePressEvent( e );
    }
    void contentsMouseReleaseEvent( QMouseEvent * e )
    {
	QListView::contentsMouseReleaseEvent( e );
	if ( selectedItem() != selected ) {
	    emit mySelectionChanged( selectedItem() );
	    emit mySelectionChanged();
	}
    }

signals:
    void mySelectionChanged();
    void mySelectionChanged( QListViewItem* );

private:
    QListViewItem* selected;

};
//
// WidgetView contains lots of Qt widgets.
//

class WidgetView : public QMainWindow
{
    Q_OBJECT
public:
    WidgetView( QWidget *parent=0, const char *name=0 );

public slots:
    void	setStatus(const QString&);
    void selectionChanged();
    void selectionChanged( QListViewItem* );
    void clicked( QListViewItem* );
    void mySelectionChanged( QListViewItem* );

protected slots:
   virtual void button1Clicked();
private slots:
    void	checkBoxClicked( int );
    void	radioButtonClicked( int );
    void	sliderValueChanged( int );
    void	listBoxItemSelected( int );
    void	comboBoxItemActivated( int );
    void	edComboBoxItemActivated( const QString& );
    void	lineEditTextChanged( const QString& );
    void	movieStatus( int );
    void	movieUpdate( const QRect& );
    void	spinBoxValueChanged( const QString& );
    void	popupSelected( int );
    
    void	open();
    void	dummy();
    void	showProperties();

private:
    bool	eventFilter( QObject *, QEvent * );
    QLabel     *msg;
    QCheckBox  *cb[3];
    QGroupBox* bg;
    QLabel     *movielabel;
    QMovie      movie;
    QWidget *central;
    QProgressBar *prog;
    int progress;
    QTabWidget* tabs;
    QMultiLineEdit* edit;
    QPopupMenu *textStylePopup;
    int plainStyleID;
};

#endif
