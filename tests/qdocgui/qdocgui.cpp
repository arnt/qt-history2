#include <qaccel.h>
#include <qapplication.h>
#include <qdict.h>
#include <qfiledialog.h>
#include <qheader.h>
#include <qinputdialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qtimer.h>

#include "qdocgui.h"


QDocListItem::QDocListItem(QListViewItem* after, QString text,
			   QString lineNumber)
    : QListViewItem(after, text)
{
    line = lineNumber;
}


QDocListItem::~QDocListItem()
{
}


QString QDocListItem::key(int, bool) const
{
    QString key = line;
    return key.rightJustify(7, '0');
}



QDocMainWindow::QDocMainWindow(const QString &qtdir, QStringList defines,
			       QWidget* parent, const char* name)
    : QDialog(parent, name),
      prevCurrentIndex(0), warnings(0), prevWarnings(0), _defines(defines)
{
    if (qtdir.isEmpty())
	qtdirenv = getenv("QTDIR");
    else {
	qtdirenv = qtdir;
	if (qtdirenv.endsWith("/"))
	    qtdirenv.truncate(qtdirenv.length() - 1);
    }

    vb = new QVBoxLayout(this);
    classList = new QListView(this);
    classList->addColumn("Text");
    classList->header()->hide();
    classList->setRootIsDecorated(TRUE);
    vb->addWidget(classList);
    QHBoxLayout* hb = new QHBoxLayout();
    statusBar = new QLabel("Ready. Click Repopulate to run qdoc.", this);
    hb->addWidget(statusBar);
    hb->setStretchFactor(statusBar, 2);
    findButton = new QPushButton("&Find...", this);
    findButton->setFocusPolicy(NoFocus);
    findButton->setAutoDefault(FALSE);
    hb->addWidget(findButton);
    version = new QPushButton("&QTDIR...", this);
    version->setFocusPolicy(NoFocus);
    version->setAutoDefault(FALSE);
    hb->addWidget(version);
    commercial = new QPushButton("&Commercial", this);
    commercial->setFocusPolicy(NoFocus);
    commercial->setAutoDefault(FALSE);
    commercial->setToggleButton(TRUE);
    commercial->setOn(FALSE);
    hb->addWidget(commercial);
    redo = new QPushButton("&Repopulate", this);
    redo->setFocusPolicy(NoFocus);
    redo->setAutoDefault(FALSE);
    hb->addWidget(redo);
    stop = new QPushButton("&Stop", this);
    stop->setFocusPolicy(NoFocus);
    stop->setAutoDefault(FALSE);
    stop->setEnabled(FALSE);
    hb->addWidget(stop);
    QPushButton *quit = new QPushButton("E&xit", this);
    quit->setFocusPolicy(NoFocus);
    quit->setAutoDefault(FALSE);
    hb->addWidget(quit);
    vb->addLayout(hb);

    {
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, "/Trolltech");
	int x = settings.readNumEntry("/qDocGUI/geometry/x", 0);
	int y = settings.readNumEntry("/qDocGUI/geometry/y", 0);
	int width = settings.readNumEntry("/qDocGUI/geometry/width", 200);
	int height = settings.readNumEntry("/qDocGUI/geometry/height", 200);
	setGeometry(x, y, width, height);
	findText = settings.readEntry("/qDocGUI/find");
    }

    updateTitle();
    setEditor();
    classList->setFocus();
    proc = new QProcess(this);

    QAccel *f3 = new QAccel(this);
    f3->connectItem(f3->insertItem(Key_F3), this, SLOT(findNext()));
    connect(classList, SIGNAL(returnPressed(QListViewItem*)),
	     this, SLOT(activateEditor(QListViewItem*)));
    connect(classList, SIGNAL(doubleClicked(QListViewItem*)),
	     this, SLOT(activateEditor(QListViewItem*)));
    connect(findButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(version, SIGNAL(clicked()), this, SLOT(changeVersion()));
    connect(redo, SIGNAL(clicked()), this, SLOT(populateListView()));
    connect(stop, SIGNAL(clicked()), proc, SLOT(tryTerminate()));
    connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(proc, SIGNAL(readyReadStderr()), this, SLOT(readOutput()));
    connect(proc, SIGNAL(processExited()), this, SLOT(finished()));
    QTimer::singleShot(20 * 1000, this, SLOT(timeout()));
}


void QDocMainWindow::find()
{
    bool ok;
    QString text = QInputDialog::getText(
			"qdocgui -- Find What?",
			"&Find what (press F3 to find again):",
			QLineEdit::Normal, findText, &ok, this);
    text = text.simplifyWhiteSpace();
    if (ok && !text.isEmpty()) {
	findText = text;
	findNext();
    }
}

void QDocMainWindow::findNext()
{
    if (findText.isEmpty())
	find();
    else {
	QString oldStatus = statusBar->text();
	statusBar->setText(QString("Searching for '%1'...").arg(findText));
	QListViewItem *current = classList->currentItem();
	if (!current)
	    current = classList->firstChild();
	if (current) {
	    QString text = findText.lower();
	    QListViewItemIterator it(current);
	    ++it;
	    while (it.current()) {
		QListViewItem *item = it.current();
		if (item->text(0).lower().contains(text)) {
		    current = item;
		    break;
		}
		++it;
	    }
	    classList->setCurrentItem(current);
	    classList->ensureItemVisible(current);
	}
	statusBar->setText(oldStatus);
    }
}

void QDocMainWindow::changeVersion()
{
    QString path = QFileDialog::getExistingDirectory(
			qtdirenv, this, "get version path",
			"Choose the QTDIR path", true);
    if (!path.isEmpty()) {
	qtdirenv = path;
	if (qtdirenv.endsWith("/"))
	    qtdirenv.truncate(qtdirenv.length() - 1);
	statusBar->setText(QString("QTDIR is now %1").arg(qtdirenv));
    }
}


void QDocMainWindow::updateTitle()
{
    QString edition("(free)");
    if (commercial->isOn())
	edition = "(commercial)";
    setCaption(QString("qdocgui -- %1 %2 %3")
		    .arg(qtdirenv).arg(edition).arg(_defines.join(" ")));
}


void QDocMainWindow::timeout()
{
    if (!stop->isEnabled())
	populateListView();
}


void QDocMainWindow::populateListView()
{
    msgCount = 0;

    updateTitle();
    findButton->setEnabled(FALSE);
    redo->setEnabled(FALSE);
    commercial->setEnabled(FALSE);
    version->setEnabled(FALSE);
    stop->setEnabled(TRUE);
    // Remember where we were on the previous run.
    QListViewItem *current = classList->currentItem();
    if (current && (QListView*)current->parent() == classList)
	current = 0; // Ignore top-level
    if (current && current->text(0).startsWith("Line"))
	current = current->parent(); // Switch to file name item
    if (current) {
	prevCurrentFile = current->text(0);
	prevCurrentIndex = 0;
	QListViewItemIterator it(classList);
	while (it.current()) {
	    QListViewItem *item = it.current();
	    if (item == current)
		break;
	    ++prevCurrentIndex;
	    ++it;
	}
    }
    classList->clear();
    QDir dir(qtdirenv + "/util/qdoc");
    if (! dir.exists("qdoc"))
	statusBar->setText(QString("No qdoc to execute in %1").
				arg(dir.path()));
    else {
	if (proc->isRunning())
	    proc->kill();
	proc->setWorkingDirectory(dir);
	qDebug("QTDIR=%s", qtdirenv.latin1());
	qDebug("cwd=%s", dir.path().latin1());
	QString command;
	proc->clearArguments();
	proc->addArgument(dir.path() + "/qdoc");
	command += dir.path() + "/qdoc ";
	proc->addArgument(dir.path() + "/qdoc.conf");
	command += dir.path() + "/qdoc.conf ";
	proc->addArgument("--friendly");
	proc->addArgument("-Wall");
	proc->addArgument("-W4");
	command += "--friendly -Wall -W4";
	for (QStringList::const_iterator it = _defines.constBegin();
	     it != _defines.constEnd(); ++it) {
	    proc->addArgument(*it);
	    command += QString(" %1").arg(*it);
	}
	if (commercial->isOn()) {
	    proc->addArgument("-Dcommercial");
	    command += " -Dcommercial";
	}

	statusBar->setText(QString("Running qdoc..."));
	// qdoc relies on $QTDIR _as well as_ qdoc.conf
	QStringList *env = new QStringList(QString("QTDIR=%1").
						arg(qtdirenv));
	if (!proc->start(env)) {
	    QString msg = QString("Failed to execute %1").
			    arg(dir.path() + "/qdoc");
	    statusBar->setText(msg);
	    qDebug(msg);
	}
	else
	    qDebug(command);
	delete env;
    }
}


void QDocMainWindow::readOutput()
{
    outputText.append(QString(proc->readStderr()));
    statusBar->setText(QString("%1 messages...").arg(++msgCount));
}


void QDocMainWindow::setEditor()
{
    bool ok = FALSE;
    QSettings settings;
    settings.insertSearchPath(QSettings::Windows, "/Trolltech");
    editor = settings.readEntry("/qDocGUI/editor");
    while (editor.isEmpty()) {
	editor = QInputDialog::getText(
			"Please enter your editor",
			"qdocgui -- choose editor",
			QLineEdit::Normal, QString::null, &ok, this);
	if (!editor.isEmpty())
	    settings.writeEntry("/qDocGUI/editor", editor);
	else
	    QMessageBox::information(
		    this,
		    "qdocgui - no editor entered",
		    "You didn't choose an editor", QMessageBox::Ok);
    }
}


void QDocMainWindow::activateEditor(QListViewItem * item)
{
    if (!item)
	return;
    statusBar->setText("");
    QString subdir;
    QString filename;
    QString cppfilename;
    classList->update();
    qApp->processEvents();
    bool hasLino = item->text(0).startsWith("Line");
    if (! hasLino &&
	 ! (item->text(0).endsWith(".h") ||
	     item->text(0).endsWith(".cpp"))) {
	statusBar->setText("Can only open files or error Lines");
	return;
    }
    QListViewItem *grandparent = item->parent();
    if (grandparent && hasLino) grandparent = grandparent->parent();
    if (!grandparent) {
	statusBar->setText(QString("Failed to find grandparent of ") +
			    item->text(0));
	return;
    }
    QString prefix = grandparent->text(0);
    if (prefix.startsWith("doc"))
	filename = qtdirenv + '/' + prefix + '/' + item->parent()->text(0);
    else if (prefix.startsWith("include")) {
	QFile f;
	QString fileText = item->parent()->text(0).
				replace(QRegExp("\\.h$"), ".doc");
	f.setName(qtdirenv + "/doc/" + fileText);
	if (f.exists())
	    filename = qtdirenv + "/doc/" + fileText;
	else {
	    fileText = item->parent()->text(0).
			    replace(QRegExp("\\.h$"), ".cpp");
	    QDir d;
	    d.setPath(qtdirenv + "/src/");
	    QStringList lst = d.entryList("*", QDir::Dirs);
	    QStringList::Iterator i = lst.begin();
	    while (i != lst.end()) {
		f.setName(qtdirenv + "/src/" + (*i) + '/' + fileText);
		if (f.exists()) {
		    filename = qtdirenv + "/include/" +
				item->parent()->text(0); // Include file first
		    cppfilename = qtdirenv + "/src/" + (*i) +
				    '/' + fileText; // source or doc file second
		    break;
		}
		++i;
	    }
	}
    }
    else if (prefix.startsWith("designer"))
	filename = qtdirenv + "/tools/designer/" + item->parent()->text(0);
    else if (prefix.startsWith("extensions"))
	filename = qtdirenv + "/extensions/" + item->parent()->text(0);
    else
	filename = qtdirenv + "/src/" + prefix + '/' + item->parent()->text(0);

    if (! hasLino) {
	int i = filename.findRev('/');
	if (i != -1)
	    filename = filename.mid(0, i + 1) + item->text(0);
    }
    QString itemtext = item->text(0);
    QRegExp rxp("(\\d+)");
    int foundpos = rxp.search(itemtext, 5);
    if (foundpos != -1 || ! hasLino) {
	// yes!
	if (QDir::home().dirName() == QString("jasmin")) {
	    QProcess *p4 = new QProcess(this);
	    p4->addArgument(QString("p4"));
	    p4->addArgument(QString("edit"));
	    p4->addArgument(filename);
	    p4->start();
	}

	procedit = new QProcess(this);
	procedit->addArgument(editor);
	if (hasLino)
	    procedit->addArgument(QString("+") + rxp.cap(0));
	procedit->addArgument(filename);
	if (! cppfilename.isEmpty())
	    procedit->addArgument(cppfilename);
	connect(procedit, SIGNAL(processExited()), this, SLOT(editorFinished()));
	if (!procedit->start()) {
	    QString msg = QString("Failed to execute %1").arg(editor);
	    statusBar->setText(msg);
	    qDebug(msg);
	    // Fix for crappy editors
	}
    }
}


void QDocMainWindow::editorFinished()
{
    QString msg = QString("%1 (%2) warnings")
		    .arg(warnings).arg(prevWarnings);
}


void QDocMainWindow::finished()
{
    statusBar->setText("Sorting...");
    QString dirText;
    QString classText;
    QString warningText;
    QString linenumber;
    int newLine;
    QString text;
    int count = 0;
    QDict<QListViewItem> category(31);
    QDict<QListViewItem> filename(4001);
    QListViewItem *dirItem   = 0;
    QListViewItem *classItem = 0;
    while (! outputText.isEmpty()) {
	newLine = outputText.find('\n');
	if (newLine == -1)
	    outputText = "";
	text = outputText.left(newLine);
	if (text.startsWith(qtdirenv))
	    text = text.mid(qtdirenv.length() + 1);

	if (!text.isEmpty() && !text.startsWith("qdoc")) {
	    if (text.startsWith("src"))
		text = text.right(text.length() - 4);
	    else if (text.startsWith("tools/designer"))
		text = text.right(text.length() - 6);
	    if (! text.startsWith("attic")) {
		int slashpos = text.find('/');
		int classfind = text.find(':');
		int secondcolonpos = text.find(':', classfind + 1);
		dirText = text.left(slashpos);
		classText = text.mid (slashpos + 1, (classfind - slashpos - 1));
		linenumber = text.mid(classfind + 1, (secondcolonpos - classfind - 1));
		warningText = text.right(text.length() - secondcolonpos - 1);
		if (warningText.right(1) == '\r')
		    warningText.truncate(warningText.length() - 1);

		if (!category[dirText]) {
		    dirItem = new QListViewItem(classList, dirText);
		    category.insert(dirText, dirItem);
		}
		else {
		    dirItem = category[dirText];
		}

		if (!filename[classText]) {
		    classItem = new QListViewItem(dirItem, classText);
		    filename.insert(classText, classItem);
		}
		else {
		    classItem = filename[classText];
		}

		new QDocListItem(classItem,
				    ("Line " + linenumber + " - " + warningText),
				    linenumber);
		count++;
	    }
	}
	outputText = outputText.right(outputText.length() - (newLine + 1));
	classList->sort();
    }
    classList->setCurrentItem(classList->firstChild());
    if (!prevCurrentFile.isEmpty()) {
	// Try to return to the item that was shown in the last run.
	QListViewItem *item = 0;
	QListViewItemIterator it(classList);
	while (it.current()) {
	    if ((it.current())->text(0) == prevCurrentFile) {
		item = it.current();
		break;
	    }
	    ++it;
	}
	if (!item) {
	    int i = 0;
	    QListViewItemIterator it(classList);
	    while (it.current()) {
		if (i == prevCurrentIndex) {
		    item = it.current();
		    break;
		}
		++i;
		++it;
	    }
	}
	if (item) {
	    classList->setCurrentItem(item);
	    classList->setOpen(item, true);
	    classList->ensureItemVisible(item);
	}
    }
    findButton->setEnabled(TRUE);
    redo->setEnabled(TRUE);
    commercial->setEnabled(TRUE);
    version->setEnabled(TRUE);
    stop->setEnabled(FALSE);
    warnings = count;
    QString msg = QString("%1 (%2) warnings").arg(warnings).arg(prevWarnings);
    statusBar->setText(msg);
    qDebug(msg);
    prevWarnings = warnings;
}


QDocMainWindow::~QDocMainWindow()
{
    QSettings settings;
    settings.insertSearchPath(QSettings::Windows, "/Trolltech");
    settings.writeEntry("/qDocGUI/geometry/x", x());
    settings.writeEntry("/qDocGUI/geometry/y", y());
    settings.writeEntry("/qDocGUI/geometry/width", width());
    settings.writeEntry("/qDocGUI/geometry/height", height());
    settings.writeEntry("/qDocGUI/find", findText);
    if (proc)
	proc->kill();
}


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    /* By default qdocgui uses $QTDIR, but you can override this by
       specifying a path on the command line.
       You can also specify defines, e.g.
       qdocgui /home/mark/qt-3.2 -Dcommercial
    */
    QString qtdir;
    QStringList defines;
    for (int i = 1; i < app.argc(); ++i) {
	QString arg = app.argv()[i];
	if (arg.startsWith("-D"))
	    defines += arg;
	else
	    qtdir = arg;
    }

    QDocMainWindow qdmw(qtdir, defines);
    app.setMainWidget(&qdmw);
    qdmw.show();
    return app.exec();

}
