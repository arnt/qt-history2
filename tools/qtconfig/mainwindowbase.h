#ifndef MAINWINDOWBASE_H
#define MAINWINDOWBASE_H

#include "ui_mainwindowbase.h"
#include <qvariant.h>

class ColorButton;
class PreviewFrame;

class MainWindowBase : public Q3MainWindow, public Ui::MainWindowBase
{
    Q_OBJECT

public:
    MainWindowBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::WType_TopLevel);
    ~MainWindowBase();

public slots:
    virtual void addFontpath();
    virtual void addSubstitute();
    virtual void browseFontpath();
    virtual void buildFont();
    virtual void buildPalette();
    virtual void downFontpath();
    virtual void downSubstitute();
    virtual void familySelected( const QString & );
    virtual void fileExit();
    virtual void fileSave();
    virtual void helpAbout();
    virtual void helpAboutQt();
    virtual void new_slot();
    virtual void pageChanged( QWidget * );
    virtual void paletteSelected( int );
    virtual void removeFontpath();
    virtual void removeSubstitute();
    virtual void somethingModified();
    virtual void styleSelected( const QString & );
    virtual void substituteSelected( const QString & );
    virtual void tunePalette();
    virtual void upFontpath();
    virtual void upSubstitute();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();


};

#endif // MAINWINDOWBASE_H
