/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtGui/QMainWindow>
#include <QtGui/QTreeWidget>

#include "torrentclient.h"

class QAction;
class QCloseEvent;
class QLabel;
class QProgressDialog;
class QSlider;
class TorrentView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);

    QSize sizeHint() const;
    const TorrentClient *clientForRow(int row) const;
    
protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void loadSettings();
    void saveSettings();

    bool addTorrent();
    void removeTorrent();
    void torrentStopped();
    void torrentError(TorrentClient::Error error);

    void updateState(TorrentClient::State state);
    void updatePeerInfo();
    void updateProgress(int percent);
    void updateDownloadRate(int bytesPerSecond);
    void updateUploadRate(int bytesPerSecond);

    void pauseTorrent();
    void moveTorrentUp();
    void moveTorrentDown();
    void setUploadLimit(int bytes);
    void setDownloadLimit(int bytes);

    void about();
    void setActionsEnabled();
    void acceptFileDrop(const QString &fileName);
    
private:
    int rowOfClient(TorrentClient *client) const;
    bool addTorrent(const QString &fileName, const QString &destinationFolder,
		    const QByteArray &resumeState = QByteArray());
    
    TorrentView *torrentView;
    QAction *removeActionMenu;
    QAction *removeActionTool;
    QAction *pauseActionMenu;
    QAction *pauseActionTool;
    QAction *upActionTool;
    QAction *downActionTool;
    QSlider *uploadLimitSlider;
    QSlider *downloadLimitSlider;
    QLabel *uploadLimitLabel;
    QLabel *downloadLimitLabel;

    int uploadLimit;
    int downloadLimit;

    struct Job {
	TorrentClient *client;
	QString torrentFileName;
	QString destinationDirectory;
    };
    QList<Job> jobs;
    int jobsStopped;
    int jobsToStop;

    QString lastDirectory;
    QProgressDialog *quitDialog;
    
    bool saveChanges;
};

#endif
