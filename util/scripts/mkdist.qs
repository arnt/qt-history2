/************************************************************
 * Some global variables needed throughout the packaging script.
 */
const qtdir = System.getenv("QTDIR");
const qmakeCommand = qtdir + "/bin/qmake";

const qdocDir = qtdir + "/util/qdoc";
const qdocCommand = qdocDir + "/qdoc";

const qpkgDir = qtdir + "/util/install/package"
const qpkgCommand = qtdir + "/bin/package"

const outputDir = System.getenv("PWD");

const validPlatforms = ["win", "x11", "mac", "embedded"];
const validEditions = ["free", "commercial"];
const validSwitches = ["branch", "version"];

const user = System.getenv("USER");

var options = [];	// list of all package options
var tmpDir;		// directory for all intermediate files
var distDir;		// directory for P4 checkout
var p4Port;
var p4Command;

/*******************************************************************************
 * Here we go
 */

System.println("Initializing...");
parseArgc();
initialize();
System.println("Checking tools...");
checkTools();
System.println("Building qdoc...");
buildQdoc();
// System.println("Building qpkg...");
// buildQpkg();
checkout();

/************************************************************
 * Parses and checks the commandline options and puts them into options[key] = value
 */
function parseArgc()
{
    validOptions = validPlatforms.toString() + validEditions.toString() + validSwitches.toString();
    for (var i=0; i<argc.length; ++i) {
	if (argc[i].startsWith("-do")) {
	    optionKey = argc[i].split("-")[2];
	    optionValue = true;
	} else if (argc[i].startsWith("-no")) {
	    optionKey = argc[i].split("-")[2];
	    optionValue = false;
	} else if (argc[i].startsWith("-")) {
	    optionKey = argc[i].split("-")[1];
	    optionValue = argc[++i];
	} else {
	    throw "Invalid option format: %1".arg(argc[i]);
	}

	// check that the optionKey is valid
	if (validOptions.find(optionKey) == -1)
	    throw "Unknown option: %1".arg(optionKey);
	else
	    options[optionKey] = optionValue;
    }
}

/************************************************************
 * Setup directories and query environment variables
 */
function initialize()
{
    // checks that branch and version has been specified
    if (options["branch"] == undefined)
	throw "branch not specified.";
    if (options["version"] == undefined)
	throw "version not specified.";


    // by default turn on all valid platforms were not defined
    for (var i in validPlatforms)
	if (!(validPlatforms[i] in options))
	    options[validPlatforms[i]] = true;

    // by default turn on all valid editions that were not defined
    for (var i in validEditions)
	if (!(validEditions[i] in options))
	    options[validEditions[i]] = true;

    // finds a tmpDir
    if (tmpDir == undefined || !File.exists(tmpDir)) {
	if (File.exists(System.getenv("HOME") + "/tmp"))
	    tmpDir = System.getenv("HOME") + "/tmp";
	else if (File.exists("/tmp"))
	    tmpDir = "/tmp";
	else
	    throw "Unable to find tmp directory";
    }
    // creates distDir
    distDir = tmpDir + "/qt-" + options["branch"] + "-" + user + "-";// + Date().getTime();
    var dir = new Dir(distDir);
    if (dir.exists)
	dir.rmdirs();
    dir.mkdir();

    // setting up p4
    if (p4Port == undefined)
	p4Port = "p4.troll.no:866";
    if (p4Command == undefined || !File.exists(p4Command))
	p4Command = System.getenv("which p4");
    if (!File.exists(p4Command))
	p4Command = "/usr/local/bin/p4";

    for (var i in options)
	System.println("options[%1] = %2".arg(i).arg(options[i]));
}

/************************************************************
 * Verify that the necesary tools are available.
 */
function checkTools()
{
    try {
	Process.execute([qmakeCommand, "-help"]);
	Process.execute("zip -help");
	Process.execute("tar --help");
	Process.execute("gzip -h");
	Process.execute(p4Command);
    } catch (e) {
	throw "Tool failed: %1".arg(e);
    }
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
 * checkouts from P4 and puts everything in distDir
 */
function checkout()
{
    //check that the branch exist
    var branchPath = "//depot/qt/" + options["branch"];
    Process.execute([p4Command, "fstat", branchPath + "/configure"]);
    if (Process.stdout.find("depotFile") == -1)
	throw "Branch: " + branchPath + " does not exist.";
    
    //check that the label exists
    var label = "qt/" + options["version"];
    Process.execute([p4Command, "labels", branchPath + "/configure"]);
    if (Process.stdout.find("Label " + label + " ") == -1)
	throw "Label: " + label + " does not exist, or not in this branch.";
}

