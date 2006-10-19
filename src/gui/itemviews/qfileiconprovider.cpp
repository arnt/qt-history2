/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfileiconprovider.h"

#ifndef QT_NO_DIRMODEL
#include <qstyle.h>
#include <qapplication.h>
#include <qdir.h>

/*!
  \class QFileIconProvider

  \brief The QFileIconProvider class provides file icon for the QDirModel class.
*/

/*!
  \enum QFileIconProvider::IconType
  \value Computer
  \value Desktop
  \value Trashcan
  \value Network
  \value Drive
  \value Folder
  \value File
*/

class QFileIconProviderPrivate
{
    Q_DECLARE_PUBLIC(QFileIconProvider)

public:
    QFileIconProviderPrivate();
    QIcon getIcon(QStyle::StandardPixmap name) const;
    QFileIconProvider *q_ptr;
    QString homePath;

private:
    QIcon file;
    QIcon fileLink;
    QIcon directory;
    QIcon directoryLink;
    QIcon harddisk;
    QIcon floppy;
    QIcon cdrom;
    QIcon ram;
    QIcon network;
    QIcon computer;
    QIcon desktop;
    QIcon trashcan;
    QIcon generic;
    QIcon home;
};

QFileIconProviderPrivate::QFileIconProviderPrivate()
{
    QStyle *style = QApplication::style();
    file = style->standardIcon(QStyle::SP_FileIcon);
    directory = style->standardIcon(QStyle::SP_DirIcon);
    fileLink = style->standardIcon(QStyle::SP_FileLinkIcon);
    directoryLink = style->standardIcon(QStyle::SP_DirLinkIcon);
    harddisk = style->standardIcon(QStyle::SP_DriveHDIcon);
    floppy = style->standardIcon(QStyle::SP_DriveFDIcon);
    cdrom = style->standardIcon(QStyle::SP_DriveCDIcon);
    network = style->standardIcon(QStyle::SP_DriveNetIcon);
    computer = style->standardIcon(QStyle::SP_ComputerIcon);
    desktop = style->standardIcon(QStyle::SP_DesktopIcon);
    trashcan = style->standardIcon(QStyle::SP_TrashIcon);
    home = style->standardIcon(QStyle::SP_DirHomeIcon);
    homePath = QDir::home().path();
}

QIcon QFileIconProviderPrivate::getIcon(QStyle::StandardPixmap name) const
{
    switch(name) {
    case QStyle::SP_FileIcon:
        return file;
    case QStyle::SP_FileLinkIcon:
        return fileLink;
    case QStyle::SP_DirIcon:
        return directory;
    case QStyle::SP_DirLinkIcon:
        return directoryLink;
    case QStyle::SP_DriveHDIcon:
        return harddisk;
    case QStyle::SP_DriveFDIcon:
        return floppy;
    case QStyle::SP_DriveCDIcon:
        return cdrom;
    case QStyle::SP_DriveNetIcon:
        return network;
    case QStyle::SP_ComputerIcon:
        return computer;
    case QStyle::SP_DesktopIcon:
        return desktop;
    case QStyle::SP_TrashIcon:
        return trashcan;
    case QStyle::SP_DirHomeIcon:
        return home;
    default:
        return QIcon();
    }
    return QIcon();
}

/*!
  Constructs a file icon provider.
*/

QFileIconProvider::QFileIconProvider()
    : d_ptr(new QFileIconProviderPrivate)
{
}

/*!
  Destroys the file icon provider.

*/

QFileIconProvider::~QFileIconProvider()
{
    delete d_ptr;
}

/*!
  Returns an icon set for the given \a type.
*/

QIcon QFileIconProvider::icon(IconType type) const
{
    Q_D(const QFileIconProvider);
    switch (type) {
    case Computer:
        return d->getIcon(QStyle::SP_ComputerIcon);
    case Desktop:
        return d->getIcon(QStyle::SP_DesktopIcon);
    case Trashcan:
        return d->getIcon(QStyle::SP_TrashIcon);
    case Network:
        return d->getIcon(QStyle::SP_DriveNetIcon);
    case Drive:
        return d->getIcon(QStyle::SP_DriveHDIcon);
    case Folder:
        return d->getIcon(QStyle::SP_DirIcon);
    case File:
        return d->getIcon(QStyle::SP_FileIcon);
    default:
        break;
    };
    return QIcon();
}

/*!
  Returns an icon for the file described by \a info.
*/

QIcon QFileIconProvider::icon(const QFileInfo &info) const
{
    Q_D(const QFileIconProvider);
    if (info.isRoot())
#ifdef Q_OS_WIN
    {
        uint type = DRIVE_UNKNOWN;
	QT_WA({ type = GetDriveTypeW((wchar_t *)info.absoluteFilePath().utf16()); },
        { type = GetDriveTypeA(info.absoluteFilePath().toLocal8Bit()); });

        switch (type) {
	case DRIVE_REMOVABLE:
            return d->getIcon(QStyle::SP_DriveFDIcon);
	case DRIVE_FIXED:
            return d->getIcon(QStyle::SP_DriveHDIcon);
	case DRIVE_REMOTE:
            return d->getIcon(QStyle::SP_DriveNetIcon);
	case DRIVE_CDROM:
            return d->getIcon(QStyle::SP_DriveCDIcon);
	case DRIVE_RAMDISK:
	case DRIVE_UNKNOWN:
	case DRIVE_NO_ROOT_DIR:
        default:
            return d->getIcon(QStyle::SP_DriveHDIcon);
	}
    }
#else
    return d->getIcon(QStyle::SP_DriveHDIcon);
#endif
  if (info.isFile())
    if (info.isSymLink())
      return d->getIcon(QStyle::SP_FileLinkIcon);
    else
      return d->getIcon(QStyle::SP_FileIcon);
  if (info.isDir()) {
    if (info.isSymLink()) {
      return d->getIcon(QStyle::SP_DirLinkIcon);
    } else {
      if (info.filePath() == d->homePath) {
        return d->getIcon(QStyle::SP_DirHomeIcon);
      } else {
        return d->getIcon(QStyle::SP_DirIcon);
      }
    }
  }
  return QIcon();
}

/*!
  Returns the type of the file described by \a info.
*/

QString QFileIconProvider::type(const QFileInfo &info) const
{
    if (info.isRoot())
        return QApplication::translate("QFileDialog", "Drive");
    if (info.isFile()) {
        if (!info.suffix().isEmpty())
            return info.suffix() + QLatin1Char(' ') + QApplication::translate("QFileDialog", "File");
        return QApplication::translate("QFileDialog", "File");
    }

    if (info.isDir())
        return QApplication::translate("QFileDialog",
#ifdef Q_OS_WIN
                                       "File Folder", "Match Windows Explorer"
#else
                                       "Folder", "All other platforms"
#endif
            );
    // Windows   - "File Folder"
    // OS X      - "Folder"
    // Konqueror - "Folder"
    // Nautilus  - "folder"

    if (info.isSymLink())
        return QApplication::translate("QFileDialog",
#ifdef Q_OS_MAC
                                       "Alias", "Match OS X Finder"
#else
                                       "Shortcut", "All other platforms"
#endif
            );
    // OS X      - "Alias"
    // Windows   - "Shortcut"
    // Konqueror - "Folder" or "TXT File" i.e. what it is pointing to
    // Nautilus  - "link to folder" or "link to object file", same as Konqueror

    return QApplication::translate("QFileDialog", "Unknown");
}

#endif

