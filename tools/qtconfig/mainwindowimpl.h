#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H

#include "mainwindow.h"


class MainWindow : public MainWindowBase
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();


public slots:
    virtual void buildPalette();
    virtual void buildFont();
    virtual void tunePalette();
    virtual void paletteSelected(int);
    virtual void styleSelected(const QString &);
    virtual void familySelected(const QString &);
    virtual void substituteSelected(const QString &);
    virtual void removeSubstitute();
    virtual void addSubstitute();
    virtual void downSubstitute();
    virtual void upSubstitute();
    virtual void removeLibpath();
    virtual void addLibpath();
    virtual void downLibpath();
    virtual void upLibpath();
    virtual void browseLibpath();
    virtual void fileSave();
    virtual void fileExit();
    virtual void somethingModified();
    virtual void helpAbout();
    virtual void helpAboutQt();


private:
    void buildActive();
    void buildActiveEffect();
    void buildInactive();
    void buildInactiveEffect();
    void buildDisabled();
    void buildDisabledEffect();

    void updateColorButtons();
    void updateFontSample();

    void setPreviewPalette(const QPalette &);

    void setModified(bool);

    QPalette editPalette, previewPalette;
    QStyle *previewstyle;
    bool modified;
};


#endif // MAINWINDOWIMPL_H
