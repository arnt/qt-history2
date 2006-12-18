#include "../../scripts/mac-binary/package/InstallerPane/keydec.cpp"
#include "tools.h"

#include <QDir>
#include <QFile>
#include <QByteArray>


// std stuff ------------------------------------
#include <iostream>
#include <windows.h>
#include <conio.h>
std::ostream &operator<<(std::ostream &s, const QString &val); // defined in configureapp.cpp
using namespace std;
// ----------------------------------------------

QString Tools::specialRot13(const QString &original)
{
    QString result(original.length(), QChar(' '));

    int i = original.length();
    while( i-- ) {
        int ch = original[i].toLatin1();
        if (ch >= 'A' && ch <= 'M' || ch >= 'a' && ch <= 'm')
            result[i] = ch + 13;
        else if (ch >= 'N' && ch <= 'Z' || ch >= 'n' && ch <= 'z')
            result[i] = ch - 13;
        else if(ch == '+')
            result[i] = ' ';
    }

    return result;
}

bool Tools::checkInternalLicense(QMap<QString,QString> &dictionary)
{
    QString licenseFile = dictionary["QT_SOURCE_TREE"] + "/LICENSE.TROLL";
    if (QFile::exists(licenseFile)) {
        QFile internalLicenseFile(licenseFile);
        bool ok = internalLicenseFile.open(QFile::ReadOnly);
        if (ok) {
            QString buffer = internalLicenseFile.readLine(1024);
            QString checkFor = "Gebyygrpu+rzcyblrrf+naq+ntragf"; //SR13:"Trolltech employees and agents"
            ok = buffer.startsWith(specialRot13(checkFor));
        }
        internalLicenseFile.close();

        if(!ok) {
            cout << "Configuration aborted since license was not accepted";
            dictionary["DONE"] = "error";
            return false;
        }

        cout << endl << "This is the Qt/Windows Trolltech Edition." << endl << endl;
        dictionary["EDITION"] = "Trolltech";
        dictionary["LICENSE FILE"] = licenseFile;
        dictionary["QT_EDITION"] = "QT_EDITION_DESKTOP";
        dictionary["QMAKE_INTERNAL"] = "yes";
        return true;
    }
    return false;
}

void Tools::checkLicense(QMap<QString,QString> &dictionary, QMap<QString,QString> &licenseInfo,
                         const QString &path)
{
    QString tpLicense = dictionary["QT_SOURCE_TREE"] + "/LICENSE.PREVIEW.OPENSOURCE";
    if (QFile::exists(tpLicense)) {
        dictionary["EDITION"] = "Preview";
        dictionary["LICENSE FILE"] = tpLicense;
        dictionary["QT_EDITION"] = "QT_EDITION_OPENSOURCE";
        return; // No license key checking in Tech Preview
    }
    tpLicense = dictionary["QT_SOURCE_TREE"] + "/LICENSE.PREVIEW.COMMERCIAL";
    if (QFile::exists(tpLicense)) {
        dictionary["EDITION"] = "Preview";
        dictionary["LICENSE FILE"] = tpLicense;
        dictionary["QT_EDITION"] = "QT_EDITION_DESKTOP";
        return; // No license key checking in Tech Preview
    }
    tpLicense = dictionary["QT_SOURCE_TREE"] + "/LICENSE.SNAPSHOT.OPENSOURCE";
    if (QFile::exists(tpLicense)) {
        dictionary["EDITION"] = "Snapshot";
        dictionary["LICENSE FILE"] = tpLicense;
        dictionary["QT_EDITION"] = "QT_EDITION_OPENSOURCE";
        return; // No license key checking in snapshot
    }
    tpLicense = dictionary["QT_SOURCE_TREE"] + "/LICENSE.SNAPSHOT.COMMERCIAL";
    if (QFile::exists(tpLicense)) {
        dictionary["EDITION"] = "Snapshot";
        dictionary["LICENSE FILE"] = tpLicense;
        dictionary["QT_EDITION"] = "QT_EDITION_DESKTOP";
        return; // No license key checking in snapshot
    }

    // Read in the license file
    QFile licenseFile(path);
    if( !path.isEmpty() && licenseFile.open( QFile::ReadOnly ) ) {
        cout << "Reading license file in....." << qPrintable(path) << endl;

        QString buffer = licenseFile.readLine(1024);
        while (!buffer.isEmpty()) {
            if( buffer[ 0 ] != '#' ) {
                QStringList components = buffer.split( '=' );
                if ( components.size() >= 2 ) {
                    QStringList::Iterator it = components.begin();
                    QString key = (*it++).trimmed().replace( "\"", QString() ).toUpper();
                    QString value = (*it++).trimmed().replace( "\"", QString() );
                    licenseInfo[ key ] = value;
                }
            }
            // read next line
            buffer = licenseFile.readLine(1024);
        }
        licenseFile.close();
    } else {
        cout << "License file not found in " << QDir::homePath() << endl;
        cout << "Please put the Qt license file, '.qt-license' in your home "
             << "directory and run configure again.";
        dictionary["DONE"] = "error";
        return;
    }

    // Verify license info...
    QString licenseKey = licenseInfo["LICENSEKEYEXT"];

    KeyDecoder keyDec(licenseKey.toLatin1());
    if (!keyDec.IsValid()) {
        cout << "License file does not contain proper license key." << endl;
        dictionary["DONE"] = "error";
        return;
    } else if (QString::number(keyDec.getLicenseID()) != licenseInfo["LICENSEID"]) {
        CDate expiryDate = keyDec.getExpiryDate();
        QString dateStr;
        dateStr.sprintf("%04d%02d%02d", expiryDate.year(), expiryDate.month(), expiryDate.day());
        cout << "License file does not contain proper license key." << endl;
        cout << dateStr << endl;
        cout << licenseInfo["EXPIRYDATE"];
        dictionary["DONE"] = "error";
        return;
    }

    uint products = keyDec.getProducts();;
    uint platforms = keyDec.getPlatforms();
    uint licenseSchema = keyDec.getLicenseSchema();
    uint licenseFeatures = keyDec.getLicenseFeatures();

    // determine which edition we are licensed to use
    QString licenseType;
    switch (licenseSchema) {
    case KeyDecoder::FullCommercial:
        licenseType = "Commercial";
        if (products & KeyDecoder::QtUniversal) {
            dictionary["EDITION"] = "Universal";
            dictionary["QT_EDITION"] = "QT_EDITION_UNIVERSAL";
        } else if (products & KeyDecoder::QtDesktop) {
            dictionary["EDITION"] = "Desktop";
            dictionary["QT_EDITION"] = "QT_EDITION_DESKTOP";
        } else if (products & KeyDecoder::QtDesktopLight) {
            dictionary["EDITION"] = "DesktopLight";
            dictionary["QT_EDITION"] = "QT_EDITION_DESKTOPLIGHT";
        } else if (products & KeyDecoder::QtConsole) {
            dictionary["EDITION"] = "Console";
            dictionary["QT_EDITION"] = "QT_EDITION_CONSOLE";
        }
        break;
    case KeyDecoder::SupportedEvaluation:
    case KeyDecoder::UnsupportedEvaluation:
    case KeyDecoder::FullSourceEvaluation:
        licenseType = "Evaluation";
        if (products & KeyDecoder::QtDesktop) {
            dictionary["EDITION"] = "Evaluation";
            dictionary["QT_EDITION"] = "QT_EDITION_EVALUATION";
        }
        break;
    case KeyDecoder::Academic:
        licenseType = "Academic";
        if (products & KeyDecoder::QtDesktop) {
            dictionary["EDITION"] = "Academic";
            dictionary["QT_EDITION"] = "QT_EDITION_ACADEMIC";
        }
        break;
    case KeyDecoder::Educational:
        licenseType = "Educational";
        if (products & KeyDecoder::QtDesktop) {
            dictionary["EDITION"] = "Educational";
            dictionary["QT_EDITION"] = "QT_EDITION_EDUCATIONAL";
        }
        break;
    }

    if (licenseType.isEmpty()
        || dictionary["EDITION"].isEmpty()
        || dictionary["QT_EDITION"].isEmpty()) {
        cout << "License file does not contain proper license key." << endl;
        dictionary["DONE"] = "error";
        return;
    }

    // verify that we are licensed to use Qt on this platform
    if (!(platforms & KeyDecoder::Windows)) {
        cout << "You are not licensed for the Qt/Windows platform." << endl << endl;
        cout << "Please contact sales@trolltech.com to upgrade your license" << endl;
        cout << "to include the Qt/Windows platform, or install the" << endl;
        cout << "Qt Open Source Edition if you intend to develop free software." << endl;
        dictionary["DONE"] = "error";
        return;
    }

    // copy one of .LICENSE-*(-US) to .LICENSE
    QString toLicenseFile = dictionary["QT_SOURCE_TREE"] + "/LICENSE";
    QString fromLicenseFile;
    switch (licenseSchema) {
    case KeyDecoder::FullCommercial:
        fromLicenseFile = dictionary["QT_SOURCE_TREE"] + "/.LICENSE";
        break;
    case KeyDecoder::SupportedEvaluation:
    case KeyDecoder::UnsupportedEvaluation:
    case KeyDecoder::FullSourceEvaluation:
        fromLicenseFile = dictionary["QT_SOURCE_TREE"] + "/.LICENSE-EVALUATION";
        break;
    case KeyDecoder::Academic:
        fromLicenseFile = dictionary["QT_SOURCE_TREE"] + "/.LICENSE-ACADEMIC";
        break;
    case KeyDecoder::Educational:
        fromLicenseFile = dictionary["QT_SOURCE_TREE"] + "/.LICENSE-EDUCATIONAL";
        break;
    }

    if (licenseFeatures & KeyDecoder::USCustomer)
        fromLicenseFile += "-US";

    if (licenseFeatures & KeyDecoder::FloatingLicense)
        dictionary["METERED LICENSE"] = "true";

    if (!QFileInfo(toLicenseFile).exists()) {
        if (!CopyFileA(QDir::toNativeSeparators(fromLicenseFile).toLocal8Bit(),
            QDir::toNativeSeparators(toLicenseFile).toLocal8Bit(), FALSE)) {
            cout << "Failed to copy license file (" << fromLicenseFile << ")";
            dictionary["DONE"] = "error";
            return;
        }
    }
    dictionary["LICENSE FILE"] = toLicenseFile;
}

