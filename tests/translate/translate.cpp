#include "translate.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qmultilinedit.h>
#include <qlabel.h>

#include <qtranslator.h>
#include <qtranslatordialog.h>

Main::Main(QWidget* parent, const char* name) :
    QVBox(parent, name)
{
    connect( new QPushButton(  tr("Quit"), this ), SIGNAL(clicked()), qApp, SLOT(quit()) );

    (void)new QLabel( tr("Text"), this );
    (new QMultiLineEdit( this ))->setText( tr( "dishwasher" ) );
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*)
{
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());

    // ...
}









class MessageShower : public QMessageParser
{
public:
    MessageShower();
    ~MessageShower();
protected:
    void add( const char*, const char*, const char* );
private:
    QTranslatorDialog *translator;
};






MessageShower::MessageShower()
    :QMessageParser()
{
    translator = new QTranslatorDialog;
}

MessageShower::~MessageShower()
{
    delete translator;
}
void MessageShower::add( const char *scope, const char *key , const char *trans )
{
    translator->addTranslation( scope, key, trans );
}



class MessageCompiler : public QMessageParser
{
public:
    MessageCompiler();
    ~MessageCompiler();
    saveAsMessageFile( const QString &outFileName );
    saveAsTextFile( const QString &outFileName );
protected:
    void add( const char*, const char*, const char* );
private:
    QTranslator mess;
    //    QString name;
};


// we require forward slashed in filename
MessageCompiler::MessageCompiler()
    :QMessageParser( ), mess(0)
{
    // name = outFileName;
}

MessageCompiler::~MessageCompiler()
{
    //    mess.save( name );
}

void MessageCompiler::add( const char *scope, const char *key , const char *trans )
{
    mess.insert( mess.hash(scope, key), trans );
}




/*

      f = new QFile( filename );

    if ( f->open( IO_ReadOnly ) ) {
	istr = new QTextStream( f );
	istr->setEncoding( QTextStream::Latin1 ); //#########
    }
*/



main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    //QMessageFileSignaller mess(0);
    //app.installMessageFile(&mess);
    QAppTranslator trd;

    //    QObject::connect(&mess, SIGNAL(triedToFind( const char*, const char*)),
    //    		     &trd, SLOT(add( const char*, const char*)));


    //    MessageShower pa;
    //MessageCompiler pa;
    //pa.parse( "pat.qpo" );


    QTranslator mf(0);
    mf.load("test.tr",".");
    app.installTranslator(&mf);

    //    int hash = mf.hash("Main","Quit");
    //    QString s = mf.find( hash, "Main", "Quit" );
    //    debug( "mf: %d -> %s", hash, (const char*)s );

    Main m;

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    m.show();

    return app.exec();
}
