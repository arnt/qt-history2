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
#if defined(Q_OS_WIN32)
#include "winintropage.h"
#endif
#include "../globalinformation.h"

class Page
{
public:
    virtual QString title() const = 0;
    virtual QString shortTitle() const = 0;
};

class BuildPageImpl : public BuildPage, public Page
{
    Q_OBJECT
public:
    BuildPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~BuildPageImpl() {}
    QString title() const
    {
#if defined(EVAL)
	return "Building Qt Examples and Tutorial";
#else
	return "Building Qt";
#endif
    }
    QString shortTitle() const
    {
#if defined(EVAL)
	return "Build Qt Examples";
#else
	return "Build Qt";
#endif
    }
};

class ConfigPageImpl : public ConfigPage, public Page
{
    Q_OBJECT
public:
    ConfigPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ConfigPageImpl() {}
    QString title() const
    {
	if( globalInformation.reconfig() )
	    return "Reconfigure Qt";
	else
	    return "Configuration";
    }
    QString shortTitle() const
    { return "Configure Qt"; }
};

class FinishPageImpl : public FinishPage, public Page
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

class FoldersPageImpl : public FoldersPage, public Page
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

class LicenseAgreementPageImpl : public LicenseAgreementPage, public Page
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

class LicensePageImpl : public LicensePage, public Page
{
    Q_OBJECT
public:
    LicensePageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LicensePageImpl() {}
    QString title() const
    { return QString("License Information to Install Qt %1").arg(globalInformation.qtVersionStr()); }
    QString shortTitle() const
    { return "License information"; }

#if defined(EVAL)
    QLineEdit* evalName;
    QLineEdit* evalCompany;
    QLineEdit* evalSerialNumber;
#endif
};

class OptionsPageImpl : public OptionsPage, public Page
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

class ProgressPageImpl : public ProgressPage, public Page
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

#if defined(Q_OS_WIN32)
class WinIntroPageImpl : public WinIntroPage, public Page
{
    Q_OBJECT
public:
    WinIntroPageImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~WinIntroPageImpl() {}
    QString title() const
    { return "Introduction"; }
    QString shortTitle() const
    { return "Introduction"; }
};
#endif

#endif // PAGES_H
