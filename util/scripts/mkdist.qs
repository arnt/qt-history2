/************************************************************
 * Some global variables needed throughout the packaging script.
 */
const qtdir = System.getenv("QTDIR");
const qmakeCommand = qtdir + "/bin/qmake";

const qdocDir = qtdir + "/util/qdoc";
const qdocCommand = qdocDir + "/qdoc";

const qpkgDir = qtdir + "/util/install/package"
const qpkgCommand = qtdir + "/bin/package"

/*******************************************************************************
 * Here we go
 */

try {
    System.println("Checking tools...");
    checkTools();
    System.println("Building qdoc...");
    buildQdoc();
    System.println("Building qpkg...");
    buildQpkg();
} catch (e) {
    System.println("mkdist failed: " + e);
}
    
/************************************************************
 * Builds and checks qdoc
 */  
function buildQdoc()
{
    var dir = new Dir(qdocDir);
    dir.setCurrent();
    Process.execute("%1 qdoc.pro".arg(qmakeCommand));
    if (Process.execute("make") != 0)
	throw "Failed to build qdoc:\n %1".arg(Process.stderr);
    // test qdoc
    Process.execute( [qdocCommand, "-help"] );
}

/************************************************************
 * Builds and checks qpkg
 */  
function buildQpkg()
{
    var dir = new Dir(qpkgDir);
    dir.setCurrent();
    Process.execute("%1 package.pro".arg(qmakeCommand));
    if (Process.execute("make") != 0)
	throw "Failed to build qpkg:\n %1".arg(Process.stderr);
    // test qpkg
    Process.execute( [qpkgCommand, "-help"] );
}

/************************************************************
 * Verify that the necesary tools are available.
 */  
function checkTools()
{
    try {
	Process.execute( [qmakeCommand, "-help"] );
	Process.execute("zip -help");
	Process.execute("tar --help");
	Process.execute("gzip -h");    
	Process.execute("p4");
    } catch (e) {
	throw "Tool failed: %1".arg(e);
    }
}
