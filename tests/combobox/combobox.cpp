#include <qapplication.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>


class MyDialog : public QDialog
{
public:
    MyDialog( QWidget *parent, bool modal );
private:
    QComboBox *combo;
    QSpinBox  *spin;
    QPushButton *ok;
};

MyDialog::MyDialog( QWidget *parent, bool modal )
    : QDialog( parent, 0, modal )
{
    combo = new QComboBox( TRUE, this );
    combo->insertItem( "First line" );
    combo->insertItem( "Second line" );
    combo->insertItem( "Third line" );
    combo->setFocus();
    combo->setGeometry( 20, 20, 120, 30 );
    spin = new QSpinBox( 10, 20, 1, this );
    spin->setGeometry( 20, 60, 120, 30 );
    ok = new QPushButton( this );
    ok->setText( "OK" );
    ok->setGeometry( 20, 90, 120, 30 );
    ok->setDefault( TRUE );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    adjustSize();
}


class MyWidget : public QLabel
{
public:
    MyWidget();
protected:
    void keyPressEvent( QKeyEvent * );
private:
    MyDialog *dlg;
};

MyWidget::MyWidget()
{
    setText( "Press a key to open the dialog" );
    setAlignment( AlignCenter );
    dlg = 0;
    resize( 200, 200 );
}

void MyWidget::keyPressEvent( QKeyEvent * )
{
    if ( !dlg ) {
	dlg = new MyDialog(this,TRUE);
	dlg->setCaption("MyDialog");
    }
    dlg->exec();
}


int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    MyWidget w;
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
