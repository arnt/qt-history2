/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WIDGETS_H
#define WIDGETS_H

#include <qmainwindow.h>
#include <qmovie.h>
#include <q3listview.h>
class QLabel;
class QCheckBox;
class QProgressBar;
class QTabWidget;
class Q3ButtonGroup;
class QMultiLineEdit;
class QPopupMenu;

class MyListView : public Q3ListView
{
    Q_OBJECT
public:
    MyListView( QWidget * parent = 0, const char *name = 0 )
	: Q3ListView( parent, name ), selected(0)
    {}
    ~MyListView()
    {}
protected:

    void contentsMousePressEvent( QMouseEvent * e )
    {
	selected = selectedItem();
	Q3ListView::contentsMousePressEvent( e );
    }
    void contentsMouseReleaseEvent( QMouseEvent * e )
    {
	Q3ListView::contentsMouseReleaseEvent( e );
	if ( selectedItem() != selected ) {
	    emit mySelectionChanged( selectedItem() );
	    emit mySelectionChanged();
	}
    }

signals:
    void mySelectionChanged();
    void mySelectionChanged( Q3ListViewItem* );

private:
    Q3ListViewItem* selected;

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
    void selectionChanged( Q3ListViewItem* );
    void clicked( Q3ListViewItem* );
    void mySelectionChanged( Q3ListViewItem* );

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

private:
    bool	eventFilter( QObject *, QEvent * );
    QLabel     *msg;
    QCheckBox  *cb[3];
    Q3ButtonGroup* bg;
    QLabel     *movielabel;
    QMovie      movie;
    QWidget *central;
    QProgressBar *prog;
    int progress;
    QTabWidget* tabs;
    QMultiLineEdit* edit;
    QPopupMenu *textStylePopup;
    int plainStyleID;
    QWidget* bla;
};

#endif
