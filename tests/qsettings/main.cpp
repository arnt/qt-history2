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
    settings.insertSearchPath(QSettings::Mac, "test/qsettings");
    val = settings.readEntry("/settings test/multiline");
    if (val.isNull() || val.isEmpty())
	printf("read empty value\n");
    else
	printf("read: --%s--\n", val.latin1());

    val = settings.readEntry("/settings test/winfilename");
    if (val.isNull() || val.isEmpty())
	printf("read empty winfilename\n");
    else
	printf("read: --%s--\n", val.latin1());

    printf( "entrylist/subkeylist tests\n" );
    QSettings settings2;
    settings2.insertSearchPath(QSettings::Unix, QDir::current().absPath());
    settings2.insertSearchPath(QSettings::Mac, "test/qsettings");
    QStringList entries = settings2.entryList("/settings test/one/two/");
    QStringList::Iterator it = entries.begin();
    while (it != entries.end())
	printf("entry1: %s\n", (*it++).latin1());
    entries = settings2.entryList("/settings test/one/two");
    it = entries.begin();
    while (it != entries.end())
	printf("entry2: %s\n", (*it++).latin1());


    QStringList subkeys = settings2.subkeyList("/settings test/one/");
    it = subkeys.begin();
    while (it != subkeys.end())
	printf("subkey: %s\n", (*it++).latin1());

}

static void writetest()
{
    printf("Write test.\n");

    QSettings settings;
    QString val;

    settings.insertSearchPath(QSettings::Unix, QDir::current().absPath());
    settings.insertSearchPath(QSettings::Mac, "test/qsettings");

    val = "this is multiline\ntext for testing the\nescaping.";
    settings.writeEntry("/settings test/multiline", val);
    settings.writeEntry("/settings test/winfilename", "c:\\depot\\mq3");
    settings.writeEntry( "/settings test/one/key0", "value" );
    settings.writeEntry( "/settings test/one/key1", "value" );
    settings.writeEntry( "/settings test/one/two/key2", "value" );
    settings.writeEntry( "/settings test/one/two/key3", "value" );
    settings.writeEntry( "/settings test/one/two/three/key4", "value" );
    settings.writeEntry( "/settings test/one/two/three/key5", "value" );
    settings.writeEntry( "/settings test/one/key6", "value" );
    settings.writeEntry( "/settings test/one/key7", "value" );
    settings.writeEntry( "/settings test/one/two/three/key8", "value" );
    settings.writeEntry( "/settings test/one/two/three/key9", "value" );
    settings.writeEntry( "/settings test/one/two/three/four/spank0", "value" );
    settings.writeEntry( "/settings test/one/two/spank1", "value" );
    settings.writeEntry( "/settings test/one/two/three/four/spank2", "value" );
    settings.writeEntry( "/settings test/one/two/three/spank3", "value" );
    settings.writeEntry( "/settings test/one/two/three/four/spank4", "value" );
    settings.writeEntry( "/settings test/one/two/spank5", "value" );
    settings.writeEntry( "/settings test/one/two/three/four/spank6", "value" );
    settings.writeEntry( "/settings test/one/two/three/spank7", "value" );
    settings.writeEntry( "/settings test/one/two/three/four/spank8", "value" );
    settings.writeEntry( "/settings test/one/two/spank9", "value" );
    settings.writeEntry( "/settings test/eno/key0", "value" );
    settings.writeEntry( "/settings test/eno/key1", "value" );
    settings.writeEntry( "/settings test/eno/owt/key2", "value" );
    settings.writeEntry( "/settings test/eno/owt/key3", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/key4", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/key5", "value" );
    settings.writeEntry( "/settings test/eno/key6", "value" );
    settings.writeEntry( "/settings test/eno/key7", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/key8", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/key9", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/four/spank0", "value" );
    settings.writeEntry( "/settings test/eno/owt/spank1", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/four/spank2", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/spank3", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/four/spank4", "value" );
    settings.writeEntry( "/settings test/eno/owt/spank5", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/four/spank6", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/spank7", "value" );
    settings.writeEntry( "/settings test/eno/owt/three/four/spank8", "value" );
    settings.writeEntry( "/settings test/eno/owt/spank9", "value" );
    settings.writeEntry( "/settings test/one/key0", "value" );
    settings.writeEntry( "/settings test/one/key1", "value" );
    settings.writeEntry( "/settings test/one/owt/key2", "value" );
    settings.writeEntry( "/settings test/one/owt/key3", "value" );
    settings.writeEntry( "/settings test/one/owt/three/key4", "value" );
    settings.writeEntry( "/settings test/one/owt/three/key5", "value" );
    settings.writeEntry( "/settings test/one/key6", "value" );
    settings.writeEntry( "/settings test/one/key7", "value" );
    settings.writeEntry( "/settings test/one/owt/three/key8", "value" );
    settings.writeEntry( "/settings test/one/owt/three/key9", "value" );
    settings.writeEntry( "/settings test/one/owt/three/four/spank0", "value" );
    settings.writeEntry( "/settings test/one/owt/spank1", "value" );
    settings.writeEntry( "/settings test/one/owt/three/four/spank2", "value" );
    settings.writeEntry( "/settings test/one/owt/three/spank3", "value" );
    settings.writeEntry( "/settings test/one/owt/three/four/spank4", "value" );
    settings.writeEntry( "/settings test/one/owt/spank5", "value" );
    settings.writeEntry( "/settings test/one/owt/three/four/spank6", "value" );
    settings.writeEntry( "/settings test/one/owt/three/spank7", "value" );
    settings.writeEntry( "/settings test/one/owt/three/four/spank8", "value" );
    settings.writeEntry( "/settings test/one/owt/spank9", "value" );
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
