/*
  combobox.cpp
*/

#include <qapplication.h>
#include <qaquastyle.h>
#include <qcdestyle.h>
#include <qcombobox.h>
#include <qmotifplusstyle.h>
#include <qmotifstyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>
#include <qgrid.h>
#include <qwindowsstyle.h>

void create_combobox( QWidget *parent, const QString& name, QStyle *style )
{
    QComboBox *combo = new QComboBox( FALSE, parent );
    combo->insertItem( QString("%1 style").arg(name) );
    combo->setStyle( style );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QGrid *grid = new QGrid( 4 );
    grid->setMargin( 11 );
    grid->setSpacing( 6 );

    create_combobox( grid, "Windows", new QWindowsStyle );
    create_combobox( grid, "Motif", new QMotifStyle );
    create_combobox( grid, "MotifPlus", new QMotifPlusStyle );
    create_combobox( grid, "CDE", new QCDEStyle );
    create_combobox( grid, "Platinum", new QPlatinumStyle );
    create_combobox( grid, "SGI", new QSGIStyle );
    create_combobox( grid, "Aqua", new QAquaStyle );

    grid->setBackgroundColor( QColor(0xec, 0xec, 0xec) );

    grid->show();
    app.setMainWidget( grid );
    return app.exec();
}
