#ifndef PAGES_H
#define PAGES_H
#include "buildpage.h"
#include "configpage.h"
#include "finishpage.h"
#include "folderspage.h"
#include "licenseagreementpage.h"
#include "licensepage.h"
#include "optionspage.h"
#include "progresspage.h"

class Page
{
public:
    virtual QString title() const = 0;
    virtual QString shortTitle() const = 0;
};

class BuildPageImpl : public BuildPage, Page
{
    Q_OBJECT
public:
    BuildPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~BuildPageImpl() {}
    QString title() const
    { return "Building Qt"; }
    QString shortTitle() const
    { return "Build Qt"; }
};

class ConfigPageImpl : public ConfigPage, Page
{
    Q_OBJECT
public:
    ConfigPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ConfigPageImpl() {}
    QString title() const
    { return "Configuration"; }
    QString shortTitle() const
    { return "Configure Qt"; }
};

class FinishPageImpl : public FinishPage, Page
{
    Q_OBJECT
public:
    FinishPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~FinishPageImpl() {}
    QString title() const
    { return "Finished"; }
    QString shortTitle() const
    { return "Finish"; }
};

class FoldersPageImpl : public FoldersPage, Page
{
    Q_OBJECT
public:
    FoldersPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~FoldersPageImpl() {}
    QString title() const
    { return "Folders"; }
    QString shortTitle() const
    { return "Choose folders"; }
};

class LicenseAgreementPageImpl : public LicenseAgreementPage, Page
{
    Q_OBJECT
public:
    LicenseAgreementPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LicenseAgreementPageImpl() {}
    QString title() const
    { return "License agreement"; }
    QString shortTitle() const
    { return "License agreement"; }
};

class LicensePageImpl : public LicensePage, Page
{
    Q_OBJECT
public:
    LicensePageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LicensePageImpl() {}
    QString title() const
    { return "License information"; }
    QString shortTitle() const
    { return "License information"; }
};

class OptionsPageImpl : public OptionsPage, Page
{
    Q_OBJECT
public:
    OptionsPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~OptionsPageImpl() {}
    QString title() const
    { return "Options"; }
    QString shortTitle() const
    { return "Choose options"; }
};

class ProgressPageImpl : public ProgressPage, Page
{
    Q_OBJECT
public:
    ProgressPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ProgressPageImpl() {}
    QString title() const
    { return "Installing"; }
    QString shortTitle() const
    { return "Install files"; }
};

#endif // PAGES_H
