/*
  dialog.cpp
*/

#define private public

#include <qapplication.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qmessagebox.h>
#include <qprintdialog.h>
#include <qprinter.h>
#include <qprogressdialog.h>
#include <qtabdialog.h>
#include <qwizard.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QMessageBox message( "Save As",
			 "C:\\temp\\resume.html already exists.\n"
			 "Do you want to replace it?",
			 QMessageBox::Warning, QMessageBox::Yes,
			 QMessageBox::No, QMessageBox::NoButton );
    message.show();

    QFileDialog file;
    file.setCaption( "Open" );
    file.setMode( QFileDialog::ExistingFiles );
    file.show();

    QProgressDialog progress( "Converting C:\\database\\customers.dat...", "Cancel", 100 );
    progress.setCaption( "Data Converter" );
    progress.setProgress( 64 );
    progress.show();

    QTabDialog tab;
    tab.show();

    QWizard wizard;
    wizard.setCaption( "..." );
    wizard.show();

    QFontDialog font;
    font.setCaption( "Select Font" );
    font.show();

    QColorDialog color;
    color.setCaption( "Select Color" );
    color.show();

    QPrinter printer;
    QPrintDialog print( &printer );
    print.setCaption( "Setup Printer" );
    print.show();

    QObject::connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
    return app.exec();
}
