#include <qsettings.h>
#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>

static void readtest()
{
    printf("Read test.\n");

    QSettings settings;
    QString val;

    settings.insertSearchPath(QSettings::Unix, QDir::current().absPath());
    val = settings.readEntry("/settings test/multiline");
    if (val.isNull() || val.isEmpty())
	printf("read empty value\n");
    else
	printf("read: --%s--\n", val.latin1());
}

static void writetest()
{
    printf("Write test.\n");

    QSettings settings;
    QString val;

    settings.insertSearchPath(QSettings::Unix, QDir::current().absPath());

    val = "this is multiline\ntext for testing the\nescaping.";
    settings.writeEntry("/settings_test/multiline", val);
}

int main(int argc, char **argv)
{
    bool doread = true;
    bool dowrite = false;
    int i;

    for(i = 1; i < argc; i++) {
	QString arg(argv[i]);
	if (arg == "-write")
	    dowrite = true;
	else if (arg == "-read")
	    doread = true;
	else if (arg == "-nowrite")
	    dowrite = false;
	else if (arg == "-noread")
	    doread = false;
	else {
	    printf("%s: unknown option %s\n\nknown options:\n\n"
		   "-read\t\tpreform read test (default)\n"
		   "-noread\t\tdo not perform read test\n"
		   "-write\t\tperform write test\n"
		   "-nowrite\tdo not perform write test (default)\n",
		   argv[0], arg.latin1());
	    exit(1);
	}
    }

    if (doread)
	readtest();
    if (dowrite)
	writetest();

    return 0;
}
