#include <qwidget.h>
#include <qpainter.h>
#include <qapp.h>
#include <qspinbox.h>
#include <qdialog.h>
#include <qchkbox.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <qlined.h>

class Main : public QDialog {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);
	
    QSpinBox* mainBox;
    QSpinBox* decBox;
    QSpinBox* stepBox;
    QSpinBox* minBox;
    QSpinBox* maxBox;
    QSpinBox* valBox;
    QCheckBox* wrapCheck;
    QCheckBox* palCheck;
    QCheckBox* disableCheck;
    QCheckBox* styleCheck;
    QLineEdit* suffixEd;
    QLineEdit* prefixEd;
    QLineEdit* minTxtEd;
    QLabel* textLb;

public slots:
 
    void updateWrap();
    void updateStep();
    void updateRange();
    void updatePalette();
    void updateDisabled();
    void updateStyle();
    void updateSpecValTxt( const char* s );
    void showValue( int i );

};

#include "spin.moc"

Main::Main(QWidget* parent, const char* name, int f)
    : QDialog(parent, name, TRUE, f)
{

    setCaption("QSpinBox test");

    QLabel* mainPre = new QLabel("The main spinbox:", this );
    mainPre->setMinimumSize( mainPre->sizeHint() );
    mainBox = new QSpinBox( 0, 10, 1, this );
    mainBox->setMinimumSize( mainBox->sizeHint() );
    connect( mainBox, SIGNAL(valueChanged(int)), this, SLOT( showValue(int) ) );
    
    /*
    QLabel* decPre = new QLabel("Number of decimals:", this );
    decPre->setMinimumSize( decPre->sizeHint() );
    decBox = new QSpinBox( -1, 9, 1, this );
    decBox->setMinimumSize( decBox->sizeHint() );
    */

    QLabel* stepPre = new QLabel("Step size:", this );
    stepPre->setMinimumSize( stepPre->sizeHint() );
    stepBox = new QSpinBox( -1, 19, 1, this );
    stepBox->setMinimumSize( stepBox->sizeHint() );
    stepBox->setValue( mainBox->lineStep() );
    connect( stepBox, SIGNAL(valueChanged(int)), this, SLOT( updateStep() ) );

    QLabel* textPre = new QLabel("Value Text:", this );
    textPre->setMinimumSize( textPre->sizeHint() );
    QLabel* textLb = new QLabel("(uninit)", this );
    textLb->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    textLb->setMinimumSize( textLb->sizeHint() );
    connect( mainBox, SIGNAL(valueChanged(const char*)), 
	     textLb, SLOT(setText(const char*)) );
    
    wrapCheck = new QCheckBox("Enable Wrapping", this );
    wrapCheck->setMinimumSize( wrapCheck->sizeHint() );
    wrapCheck->setChecked( mainBox->wrapping() );
    connect( wrapCheck, SIGNAL(clicked()), this, SLOT(updateWrap()) );
    
    QLabel* minPre = new QLabel("Minimum value:", this );
    minPre->setMinimumSize( minPre->sizeHint() );
    minBox = new QSpinBox( -10, 10, 1, this );
    minBox->setMinimumSize( minBox->sizeHint() );
    minBox->setValue( mainBox->minValue() );
    connect( minBox, SIGNAL(valueChanged(int)), this, SLOT( updateRange() ) );

    QLabel* maxPre = new QLabel("Maximum value:", this );
    maxPre->setMinimumSize( maxPre->sizeHint() );
    maxBox = new QSpinBox( -10, 10, 1, this );
    maxBox->setMinimumSize( maxBox->sizeHint() );
    maxBox->setValue( mainBox->maxValue() );
    connect( maxBox, SIGNAL(valueChanged(int)), this, SLOT( updateRange() ) );

    QLabel* valPre = new QLabel("Set value:", this );
    valPre->setMinimumSize( valPre->sizeHint() );
    valBox = new QSpinBox( -20, 20, 1, this );
    valBox->setMinimumSize( valBox->sizeHint() );
    valBox->setValue( mainBox->value() );
    connect( valBox, SIGNAL(valueChanged(int)), mainBox, SLOT( setValue(int) ) );


    QLabel* prefixPre = new QLabel("Set Prefix:", this );
    prefixPre->setMinimumSize( prefixPre->sizeHint() );
    prefixEd = new QLineEdit( this );
    prefixEd->setMinimumSize( prefixEd->sizeHint() );
    connect( prefixEd, SIGNAL(textChanged(const char*)), mainBox, SLOT(setPrefix(const char*)));

    QLabel* suffixPre = new QLabel("Set Suffix:", this );
    suffixPre->setMinimumSize( suffixPre->sizeHint() );
    suffixEd = new QLineEdit( this );
    suffixEd->setMinimumSize( suffixEd->sizeHint() );
    connect( suffixEd, SIGNAL(textChanged(const char*)), mainBox, SLOT(setSuffix(const char*)));

    QLabel* minTxtPre = new QLabel("Set specialValueText:", this );
    minTxtPre->setMinimumSize( minTxtPre->sizeHint() );
    minTxtEd = new QLineEdit( this );
    minTxtEd->setMinimumSize( minTxtEd->sizeHint() );
    connect( minTxtEd, SIGNAL(textChanged(const char*)), this, SLOT(updateSpecValTxt(const char*)));

    palCheck = new QCheckBox("Custom palette", this );
    palCheck->setMinimumSize( palCheck->sizeHint() );
    connect( palCheck, SIGNAL(clicked()), this, SLOT(updatePalette()) );

    disableCheck = new QCheckBox("Disabled", this );
    disableCheck->setMinimumSize( disableCheck->sizeHint() );
    connect( disableCheck, SIGNAL(clicked()), this, SLOT(updateDisabled()) );

    styleCheck = new QCheckBox("WinStyle", this );
    styleCheck->setMinimumSize( styleCheck->sizeHint() );
    connect( styleCheck, SIGNAL(clicked()), this, SLOT(updateStyle()) );

    QPushButton* ok = new QPushButton( "Ok", this );
    ok->setMinimumSize( 75, ok->sizeHint().height() );
    ok->setDefault( TRUE );
    QObject::connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );

    QGridLayout* dl = new QGridLayout( this, 12, 3, 5 );
    dl->addWidget( mainPre,	0, 0 );
    dl->addWidget( mainBox,	0, 1 );
    dl->addWidget( stepPre,	1, 0 );
    dl->addWidget( stepBox,	1, 1 );
    dl->addWidget( wrapCheck,	2, 0 );
    dl->addWidget( minPre,	3, 0 );
    dl->addWidget( minBox,	3, 1 );
    dl->addWidget( maxPre,	4, 0 );
    dl->addWidget( maxBox,	4, 1 );
    dl->addWidget( valPre,	5, 0 );
    dl->addWidget( valBox,	5, 1 );
    dl->addWidget( prefixPre,	6, 0 );
    dl->addWidget( prefixEd,	6, 1 );
    dl->addWidget( suffixPre,	7, 0 );
    dl->addWidget( suffixEd,	7, 1 );
    dl->addWidget( minTxtPre,	8, 0 );
    dl->addWidget( minTxtEd,	8, 1 );
    dl->addWidget( textPre,	9, 0 );
    dl->addWidget( textLb,	9, 1 );
    dl->addWidget( palCheck,	10, 0 );
    dl->addWidget( disableCheck,10, 1 );
    dl->addWidget( styleCheck,	10, 2 );
    dl->addWidget( ok,		11, 2 );
    dl->activate();

    resize( 1, 1 );


}

void Main::updateWrap()
{
    mainBox->setWrapping( wrapCheck->isChecked() );
}

void Main::updateStep()
{
    mainBox->setSteps( stepBox->value(), stepBox->value() );
}

void Main::updateRange()
{
    mainBox->setRange( minBox->value(), maxBox->value() );
}

void Main::updateDisabled()
{
    mainBox->setEnabled( !disableCheck->isChecked() );
}

void Main::updateStyle()
{
    mainBox->setStyle( styleCheck->isChecked() ? WindowsStyle : MotifStyle );
}


void Main::updatePalette()
{
    if ( palCheck->isChecked() ) {
	QPalette myPal( green );
	mainBox->setPalette( myPal );
    }
    else {
	mainBox->setPalette( *qApp->palette() );
    }
}

void Main::updateSpecValTxt( const char* s )
{
    mainBox->setSpecialValueText( s );
}

void Main::showValue( int i )
{
    debug("Spin: main box emitted valueChanged( %i )", i);
}


main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Main m;
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}
