/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qapplication.h>
#include <qdebug.h>
#include <q3filedialog.h>
#include <qlabel.h>


//TESTED_CLASS=
//TESTED_FILES=q3filedialog.h

class tst_Q3FileDialog : public QObject
{
Q_OBJECT

public:
    tst_Q3FileDialog();
    virtual ~tst_Q3FileDialog();

private slots:
    void getSetCheck();
};

tst_Q3FileDialog::tst_Q3FileDialog()
{
}

tst_Q3FileDialog::~tst_Q3FileDialog()
{
}

  class Preview : public QLabel, public Q3FilePreview
  {
  public:
      Preview(QWidget *parent=0) : QLabel(parent) {}

      void previewUrl(const Q3Url &u)
      {
          QString path = u.path();
          QPixmap pix(path);
          if (pix.isNull())
              setText("This is not a pixmap");
          else
              setText("This is a pixmap");
      }
  };


// Testing get/set functions
void tst_Q3FileDialog::getSetCheck()
{
    Q3FileDialog obj1;
    // bool Q3FileDialog::showHiddenFiles()
    // void Q3FileDialog::setShowHiddenFiles(bool)
    obj1.setShowHiddenFiles(false);
    QCOMPARE(false, obj1.showHiddenFiles());
    obj1.setShowHiddenFiles(true);
    QCOMPARE(true, obj1.showHiddenFiles());

    // ViewMode Q3FileDialog::viewMode()
    // void Q3FileDialog::setViewMode(ViewMode)
    obj1.setViewMode(Q3FileDialog::ViewMode(Q3FileDialog::Detail));
    QCOMPARE(Q3FileDialog::ViewMode(Q3FileDialog::Detail), obj1.viewMode());
    obj1.setViewMode(Q3FileDialog::ViewMode(Q3FileDialog::List));
    QCOMPARE(Q3FileDialog::ViewMode(Q3FileDialog::List), obj1.viewMode());

    Preview* p = new Preview;
    obj1.setContentsPreviewEnabled(true);
    obj1.setContentsPreview(p, p);
    obj1.setInfoPreviewEnabled(true);
    obj1.setInfoPreview(p, p);
    // PreviewMode Q3FileDialog::previewMode()
    // void Q3FileDialog::setPreviewMode(PreviewMode)
    obj1.setPreviewMode(Q3FileDialog::PreviewMode(Q3FileDialog::NoPreview));
    QCOMPARE(Q3FileDialog::PreviewMode(Q3FileDialog::NoPreview), obj1.previewMode());
    obj1.setPreviewMode(Q3FileDialog::PreviewMode(Q3FileDialog::Contents));
    QCOMPARE(Q3FileDialog::PreviewMode(Q3FileDialog::Contents), obj1.previewMode());
    obj1.setPreviewMode(Q3FileDialog::PreviewMode(Q3FileDialog::Info));
    QCOMPARE(Q3FileDialog::PreviewMode(Q3FileDialog::Info), obj1.previewMode());
}

QTEST_MAIN(tst_Q3FileDialog)
#include "tst_q3filedialog.moc"
