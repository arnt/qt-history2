/************************************************************
 * Some global variables needed throughout the packaging script.
 */
const qtDir = System.getenv("QTDIR");
const qmakeCommand = qtDir + "/bin/qmake";

const qdocDir = qtDir + "/util/qdoc3";
const qdocCommand = qdocDir + "/qdoc3";

const outputDir = System.getenv("PWD");

const validPlatforms = ["win", "x11", "mac", "embedded"];
const validEditions = ["opensource", "commercial", "preview", "beta"];
const validSwitches = ["gzip", "bzip", "zip", "binaries", "snapshots"]; // these are either true or false, set by -do-foo/-no-foo
const validVars = ["branch", "version"];       // variables with arbitrary values, set by -foo value

const binaryExtensions = ["msi", "dll", "gif", "png", "mng",
			  "jpg", "bmp", "any", "pic", "ppm",
			  "exe", "zip", "qm", "ico", "wav",
			  "icns", "qpf", "bdf", "pfb", "pfa",
			  "ttf"];

var binaryHosts = new Array();
binaryHosts["win"] = "innsikt";
binaryHosts["mac"] = "zoidberg";
const binaryUser = "period";
		     
const user = System.getenv("USER");

var startDate = new Date(); // the start date of the script
var options = [];	    // list of all package options
var tmpDir;		    // directory for system temporary files
var distDir;		    // parent directory for all temp packages and the checkout dir
var checkoutDir;	    // directory for P4 checkout
var licenseHeaders = [];    // license text to put in .cpp and .h headers
var moduleMap = [];         // maps between directories and module/class/application names
var p4Port;
var p4Command;
var p4BranchPath;            // typically //depot/qt/[thebranch]
var p4Label;                 // the P4 label or date

var indentation = 0;
const tabSize = 4;

/************************************************************
 * Purging filters
 */

var checkoutRemove = [ new RegExp("^tests"),
		       new RegExp("^tmake"),
		       new RegExp("^util"),
		       new RegExp("^translations"),
		       new RegExp("^pics"),
		       new RegExp("^bin/syncqt.bat"),
		       new RegExp("^extensions/nsplugin"),
		       new RegExp("^extensions/xt"),
		       new RegExp("^tools/makeqpf"),
		       new RegExp("^tools/mergetr"),
		       new RegExp("^tools/msg2qm"),
		       new RegExp("^tools/qconfig"),
		       new RegExp("^tools/qembed"),
		       new RegExp("^tools/qev"),
		       new RegExp("^tools/designer/data"),
		       new RegExp("^tools/designer/tests"),
		       new RegExp("^src/gui/itemviews/qheaderwidget"),
		       new RegExp("^src/gui/painting/makepsheader.pl"),
		       new RegExp("^src/gui/painting/qpsprinter"),
		       new RegExp("^LICENSE.TROLL") ];
var checkoutKeep = [ /./ ];

var platformRemove = new Array();
var platformKeep = new Array();

var editionRemove = new Array();
var editionKeep = new Array();

platformRemove["win"] = [ new RegExp("^gif"),
			  new RegExp("^doc/src"),
			  new RegExp("^config.tests"),
			  new RegExp("^src/plugins/gfxdrivers"),
			  new RegExp("^src/plugins/decorations"),
			  new RegExp("^tools/qtconfig"),
			  new RegExp("^tools/qvfb"),
			  new RegExp("^extensions/motif"),
			  new RegExp("_x11"),
			  new RegExp("_unix"),
			  new RegExp("_qws"),
			  new RegExp("_wce"),
			  new RegExp("_mac"),
			  new RegExp("^src/plugins/styles/mac"),
			  new RegExp("_qnx4"),
			  new RegExp("_qnx6"),
			  new RegExp("^configure"),
			  new RegExp("^LICENSE.PREVIEW"),
			  new RegExp("^README.qws") ];
platformKeep["win"] = [ new RegExp(".") ];

platformRemove["x11"] = [ new RegExp("^gif"),
			  new RegExp("^doc/src"),
			  new RegExp("^src/plugins/gfxdrivers"),
			  new RegExp("^src/plugins/decorations"),
			  new RegExp("^extensions/activeqt"),
			  new RegExp("_win"),
			  new RegExp("_qws"),
			  new RegExp("_wce"),
			  new RegExp("_mac"),
			  new RegExp("^src/plugins/styles/mac"),
			  new RegExp("_qnx4"),
			  new RegExp("_qnx6"),
			  new RegExp("^bin/configure.exe"),
			  new RegExp("^README.qws") ];
platformKeep["x11"] = [ new RegExp(".") ];

platformRemove["mac"] = [ new RegExp("^gif"),
			  new RegExp("^doc/src"),
			  new RegExp("^src/plugins/gfxdrivers"),
			  new RegExp("^src/plugins/decorations"),
			  new RegExp("^extensions/activeqt"),
			  new RegExp("_win"),
			  new RegExp("_qws"),
			  new RegExp("_wce"),
			  new RegExp("_x11"),
			  new RegExp("_qnx4"),
			  new RegExp("_qnx6"),
			  new RegExp("^bin/configure.exe"),
			  new RegExp("^README.qws") ];
platformKeep["mac"] = [ new RegExp(".") ];

platformRemove["embedded"] = [ new RegExp("^gif"),
			       new RegExp("^doc/src"),
			       new RegExp("_win"),
			       new RegExp("_wce"),
			       new RegExp("_mac"),
			       new RegExp("^src/plugins/styles/mac"),
			       new RegExp("^extensions/activeqt"),
			       new RegExp("_qnx4"),
			       new RegExp("_qnx6"),
			       new RegExp("^bin/configure.exe") ];
platformKeep["embedded"] = [ new RegExp(".") ];

editionRemove["commercial"] = [ new RegExp("GPL") ];
editionKeep["commercial"] = [ new RegExp(".") ];

editionRemove["opensource"] = [ new RegExp("^extensions"),
				new RegExp("^src/plugins/sqldrivers/db2"),
				new RegExp("^src/plugins/sqldrivers/oci"),
				new RegExp("^src/plugins/sqldrivers/tds"),
				new RegExp("^src/sql/drivers/db2"),
				new RegExp("^src/sql/drivers/oci"),
				new RegExp("^src/sql/drivers/tds"),
				new RegExp("^qmake/generators/win32/borland"),
				new RegExp("^qmake/generators/win32/msvc"),
				new RegExp("^mkspecs/win32-borland"),
				new RegExp("^mkspecs/win32-icc"),
				new RegExp("^mkspecs/win32-msvc"),
				new RegExp("^mkspecs/win32-msvc.net"),
				new RegExp("^qmake/generators/mac/metrowerks"),
				new RegExp("^qmake/generators/mac/pbuilder"),
				new RegExp("^mkspecs/macx-mwerks"),
				new RegExp("^mkspecs/macx-pbuilder"),
				new RegExp("^mkspecs/macx-xcode"),
				new RegExp("^README-QT.TXT") ];
editionKeep["opensource"] = [ new RegExp(".") ];

editionRemove["preview"] = [ new RegExp("GPL") ];
editionKeep["preview"] = [ new RegExp(".") ];

editionRemove["beta"] = [ new RegExp("GPL") ];
editionKeep["beta"] = [ new RegExp(".") ];

var finalRemove = [ new RegExp("^dist") ];
var finalKeep = [ /./ ];

/************************************************************
 * Mapping from directories to module names
 */

moduleMap["demonstration applications"]  = new RegExp("^demos");
moduleMap["documentation"]               = new RegExp("^doc");
moduleMap["example classes"]             = new RegExp("^examples");
moduleMap["qmake application"]           = new RegExp("^qmake");
moduleMap["activeqt module"]             = new RegExp("^extensions/activeqt");
moduleMap["motif module"]                = new RegExp("^extensions/motif");
moduleMap["Qt 3 compatibility classes"]  = new RegExp("^src/qt3support");
moduleMap["core module"]                 = new RegExp("^src/core");
moduleMap["accessibility module"]        = new RegExp("(^src/gui/accessible|^src/plugins/accessible)");
moduleMap["dialog module"]               = new RegExp("^src/gui/dialogs");
moduleMap["embedded classes"]            = new RegExp("(^src/gui/embedded|^src/plugins/gfxdrivers|^src/plugins/decorations)");
moduleMap["item views module"]           = new RegExp("^src/gui/itemviews");
moduleMap["gui module"]                  = new RegExp("^src/gui/kernel");
moduleMap["painting module"]             = new RegExp("(^src/gui/painting|^src/gui/image|^src/plugins/imageformats)");
moduleMap["style module"]                = new RegExp("(^src/gui/styles|^src/plugins/styles)");
moduleMap["text module"]                 = new RegExp("^src/gui/text");
moduleMap["widgets module"]              = new RegExp("^src/gui/widgets");
moduleMap["input methods"]               = new RegExp("^src/gui/inputmethod");
moduleMap["moc application"]             = new RegExp("^src/moc");
moduleMap["network module"]              = new RegExp("^src/network");
moduleMap["opengl module"]               = new RegExp("^src/opengl");
moduleMap["internationalization module"] = new RegExp("^src/plugins/codecs");
moduleMap["sql module"]                  = new RegExp("(^src/sql|^src/plugins/sqldrivers)");
moduleMap["tools applications"]          = new RegExp("^src/tools");
moduleMap["window classes"]              = new RegExp("^src/winmain");
moduleMap["xml module"]                  = new RegExp("^src/xml");
moduleMap["designer application"]        = new RegExp("^tools/designer");
moduleMap["assistant application"]       = new RegExp("^tools/assistant");
moduleMap["linguist application"]        = new RegExp("^tools/linguist");
moduleMap["qtconfig application"]        = new RegExp("^tools/qtconfig");
moduleMap["virtual framebuffer"]         = new RegExp("^tools/qvfb");
moduleMap["porting application"]         = new RegExp("^tools/porting");
moduleMap["resource dump application"]   = new RegExp("^tools/rccdump");

/*******************************************************************************
 * Here we go
 */
print("Initializing...");
parseArgc();
initialize();
print("Checking tools and hosts...");
checkTools();
print("Building qdoc...");
buildQdoc();
print("Checkout from P4...");
checkout();
print("Purging checkout...");
purgeFiles(checkoutDir, getFileList(checkoutDir), checkoutRemove, checkoutKeep);
indentation+=tabSize;
for (var p in validPlatforms) {
    for (var e in validEditions) {
  	var platform = validPlatforms[p];
  	var edition = validEditions[e];
  	if (options[platform] && options[edition]) {
	    if (edition == "opensource" && platform == "win")
		continue;
  	    print("Packaging %1-%2...".arg(platform).arg(edition));
  	    indentation+=tabSize;

  	    // copy checkoutDir to platDir and set permissions
  	    print("Copying checkout...");
  	    var platName = "qt-%1-%2-%3".arg(platform).arg(edition).arg(options["version"]);
  	    var platDir = distDir + "/" + platName;
  	    execute(["cp", "-r", checkoutDir, platDir]);
	    execute(["chmod", "-R", "ug+w", platDir]);

	    //copying dist files
	    print("Copying dist files...");
	    copyDist(platDir, platform, edition);

	    // run qdoc
  	    print("Running qdoc...");
  	    qdoc(platDir, edition);

  	    // purge platform and edition files
  	    print("Purging platform and edition specific files...");
  	    purgeFiles(platDir,
		       getFileList(platDir),
  		       [].concat(platformRemove[platform]).concat(editionRemove[edition]),
  		       [].concat(platformKeep[platform]).concat(editionKeep[edition]));

	    // run syncqt
  	    print("Running syncqt...");
  	    syncqt(platDir, platform);

  	    // final package purge
  	    print("Final package purge...");
  	    purgeFiles(platDir, getFileList(platDir), finalRemove, finalKeep);

	    // replace tags (like THISYEAR etc.)
	    print("Traversing all txt files and replacing tags...");
	    replaceTags(platDir, getFileList(platDir), platform, edition, platName);

  	    // package directory
	    print("Compressing and packaging file(s)...")
	    compress(platform, edition, platDir);

	    // create binaries
	    compile(platform, edition, platName);
	    
  	    indentation-=tabSize;
  	}
    }
}
indentation-=tabSize;
print("Cleaning all temp files...");
cleanup();

/************************************************************
 * Parses and checks the commandline options and puts them into options[key] = value
 */
function parseArgc()
{
    var validOptions = []
	.concat(validPlatforms)
	.concat(validEditions)
	.concat(validSwitches)
	.concat(validVars);
    for (var i=0; i<argc.length; ++i) {
	var optionKey;
	var optionValue;
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

	var optionOk = false;
	for (var o in validOptions) {
	    if (optionKey == validOptions[o]) {
		optionOk = true;
		break;
	    }
	}

	if (optionOk)
	    options[optionKey] = optionValue;
	else
	    throw "Unknown option: %1".arg(optionKey);
    }
}

/************************************************************
 * Setup directories and query environment variables
 */
function initialize()
{
    // checks that all valid vars are specified
    for (var i in validVars)
	if (!(validVars[i] in options))
	    throw "%1 was not specified.".arg(validVars[i]);

    // by default turn off all valid switches that were not defined
    for (var i in validSwitches)
	if (!(validSwitches[i] in options))
	    options[validSwitches[i]] = false;

    // by default turn off all valid platforms that were not defined
    for (var i in validPlatforms)
	if (!(validPlatforms[i] in options))
	    options[validPlatforms[i]] = false;

    // by default turn off all valid editions that were not defined
    for (var i in validEditions)
	if (!(validEditions[i] in options))
	    options[validEditions[i]] = false;

    // make sure platform and edition filters are defined
    for (var i in validPlatforms) {
	if (!(validPlatforms[i] in platformRemove))
	    platformRemove[validPlatforms[i]] = new Array();
	if (!(validPlatforms[i] in platformKeep))
	    platformKeep[validPlatforms[i]] = new Array();
    }
    for (var i in validEditions) {
	if (!(validEditions[i] in editionRemove))
	    editionRemove[validEditions[i]] = new Array();
	if (!(validEditions[i] in editionKeep))
	    editionKeep[validEditions[i]] = new Array();
    }

    // finds a tmpDir
    if (tmpDir == undefined || !File.exists(tmpDir)) {
	if (File.exists(System.getenv("HOME") + "/tmp"))
	    tmpDir = System.getenv("HOME") + "/tmp";
	else if (File.exists("/tmp"))
	    tmpDir = "/tmp";
	else
	    throw "Unable to find tmp directory";
    }
    // creates distDir and sets checkoutDir
    distDir = tmpDir + "/qt-" + options["branch"] + "-" + user + "-" + startDate.getTime();
    var dir = new Dir(distDir);
    if (dir.exists)
	dir.rmdirs();
    dir.mkdir();
    checkoutDir = distDir + "/qt";

    // setting up p4
    if (p4Port == undefined)
	p4Port = "p4.troll.no:866";
    if (p4Command == undefined || !File.exists(p4Command))
	p4Command = System.getenv("which p4");
    if (!File.exists(p4Command))
	p4Command = "/usr/local/bin/p4";
    if (!File.exists(p4Command))
	p4Command = "/usr/bin/p4";

    // add "-snapshot-yyyymmdd" to version
    if (options["snapshots"])
	options["version"] = options["version"] + "-snapshot-%1%2%3"
	    .arg(startDate.getYear())
	    .arg(startDate.getMonth() < 10 ? "0" + startDate.getMonth() : startDate.getMonth())
	    .arg(startDate.getDate() < 10 ? "0" + startDate.getDate() : startDate.getDate());

//     for (var i in options)
// 	print("options[%1] = %2".arg(i).arg(options[i]));
}

/************************************************************
 * Verify that the necessary tools and hosts are available.
 */
function checkTools()
{
    try {
	execute(["which", qmakeCommand]);
	execute("zip -help");
	execute("tar --help");
	execute("gzip -h");
 	execute("bzip2 -h");
	execute("cp --help");
	execute("which scp");
	execute("ssh -V");
	for (var p in binaryHosts) {
	    if (options["binaries"] && options[p]) {
		var host = binaryHosts[p];
		execute(["ssh", "period@" + host, "true"]);
	    }
	}
	execute(p4Command);
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
    execute("%1 qdoc3.pro".arg(qmakeCommand));
    execute("make");
    // test qdoc
    execute( [qdocCommand, "-version"] );
}


/************************************************************
 * checkouts from P4 and puts everything in checkoutDir
 */
function checkout()
{
    // check that the branch exist
    p4BranchPath = "//depot/qt/" + options["branch"];
    execute([p4Command, "fstat", p4BranchPath + "/configure"]);
    if (Process.stdout.find("depotFile") == -1)
	throw "Branch: " + p4BranchPath + " does not exist.";
    
    // check that the label exists
    if (options["snapshots"]) {
	p4Label = startDate.toString().replace(/-/g, "/").replace(/T/g, ":");
    } else {
	p4Label = "qt/" + options["version"];
	execute([p4Command, "labels", p4BranchPath + "/configure"]);
	if (Process.stdout.find("Label " + p4Label + " ") == -1)
	    throw "Label: " + p4Label + " does not exist, or not in this branch.";
    }

    // generate clientSpec
    var tmpClient="qt-release-tmp-" + user;
    execute([p4Command, "client", "-t", "qt-release-3x", "-o", tmpClient]);
    var clientSpec = Process.stdout.split("\n");
    for (var i in clientSpec) {
	clientSpec[i] = clientSpec[i].replace(/^Root:.*/, "Root: " + distDir);
	clientSpec[i] = clientSpec[i].replace(/X.Y/, options["branch"]);
	clientSpec[i] = clientSpec[i].replace(/\bnomodtime\b/, "modtime");
    }
    // save clientSpec
    clientSpec = clientSpec.join("\n");
    execute([p4Command, "client", "-i"], clientSpec);

    // checkout
    execute([p4Command, "-c", tmpClient, "-d", distDir, "sync", "-f", "...@" + p4Label]);

    // test for checkoutDir
    if (!File.exists(checkoutDir))
	throw "Checkout failed, checkout dir %1 does not exist.".arg(checkoutDir);
}

/************************************************************
 * iterates over the fileList and removes any files found in the
 * remove patterns and keeps any files found in the keep pattern, any
 * file not found in any of the patterns throws an exception
 */
function purgeFiles(rootDir, fileList, remove, keep)
{
    var doRemove = false;
    var doKeep = false;
    var fileName = new String();
    var absFileName = new String();

    for (var i in fileList) {
	doRemove = false;
	doKeep = false;
	fileName = fileList[i];
	absFileName = rootDir + "/" + fileName;
	// check if the file should be removed
	for (var r in remove) {
	    if (fileName.find(remove[r]) != -1) {
		doRemove = true;
		break;
	    }
	}

	// remove file
	if (doRemove) {
	    if (File.exists(absFileName)) {
		if (File.isFile(absFileName)) {
		    File.remove(absFileName);
		} else if (File.isDir(absFileName)) {
		    var dir = new Dir(absFileName);
		    dir.rmdirs();
		}
	    }
	    continue;
	}

	// check if the file should be kept
	for (var k in keep) {
	    if (fileName.find(keep[k]) != -1) {
		doKeep = true;
		break;
	    }
	}

	// bail out
	if (!doKeep)
	    throw "File: %1 not found in remove nor keep filter, bailing out.".arg(absFileName);
    }
}

/************************************************************
 * compresses platDir into files (.zip .gz etc.)
 */
function compress(platform, edition, packageDir)
{
    // set directory to parent of packageDir
    var dir = new Dir(packageDir);
    var packageName = dir.name;
    dir.cdUp();
    dir.setCurrent();

    if (platform == "win") {
	if (options["zip"]) {
	    var files = getFileList(packageDir);
	    var binaryFiles = new Array();
	    var textFiles = new Array();
	    var fileName = new String();
	    var absFileName = new String();
	    var zipFile = outputDir + "/" + packageName + ".zip";
	    // delete any old zipFile
	    if (File.exists(zipFile))
		File.remove(zipFile);
	    // generate list of binary and text files
	    for (var i in files) {
		fileName = files[i];
		absFileName = packageDir + "/" + fileName;
		if (File.exists(absFileName) && File.isFile(absFileName)) {
		    if (binaryFile(absFileName))
			binaryFiles.push(packageName + "/" + fileName);
		    else
			textFiles.push(packageName + "/" + fileName);
		}
	    }
	    // add the binary and text files to the zip file in in two big goes
	    dir.setCurrent(); //  current dir is parent of packageDir
	    if (binaryFiles.length > 0)
		execute(["zip", "-9q", zipFile, "-@"], binaryFiles.join("\n"));
	    if (textFiles.length > 0)
		execute(["zip", "-l9q", zipFile, "-@"], textFiles.join("\n"));
	}
    } else {
	var tarFile = outputDir + "/" + packageName + ".tar";
	execute(["tar", "-cf", tarFile, packageName]);
	if (!File.exists(tarFile))
	    throw "Failed to produce %1.".arg(tarFile);
	
 	if (options["bzip"]) {
 	    execute(["bzip2", "-zkf", tarFile]);
 	}
 	if (options["gzip"]) {
 	    execute(["gzip", "-f", tarFile]);
	}
	// remove .tar if we have bzipped or gzipped
	if ((options["gzip"] || options["bzip"]) && File.exists(tarFile))
	    File.remove(tarFile);
    }
}


/************************************************************
 * copies a qt-package to binary host, compiles qt, and collects the
 * resulting dlls etc.
 */
function compile(platform, edition, platformName)
{
    if (!options["binaries"] || !(platform in binaryHosts))
	return;

    print("Compiling binaries...")

    var login = binaryUser + "@" + binaryHosts[platform];

    // remove any previous packages/dirs/scripts for this platform
    execute(["ssh", login, "rm -rf", platformName + "*"]);
    execute(["ssh", login, "rm -rf", "buildbinary" + platform + "*"]);
    execute(["ssh", login, "rm -rf", "installscript" + platform + "*"]);
    execute(["ssh", login, "rm -rf", "write*.nsh"]);
    execute(["ssh", login, "rm -rf", "checkqtlicense.ini"]);
    execute(["ssh", login, "rm -rf", "setenvpage.ini"]);

    if (platform == "win" && options["zip"]) {
	// copy zip package to host
	var packageName = platformName + ".zip";
	execute(["scp", outputDir + "/" + packageName, login + ":."]);
	
	// unzip package (overwrite)
	execute(["ssh", login, "unzip", "-q", "-o", packageName]);
	
	// duplicate directory where we copy the compiled results over
	execute(["ssh", login, "cp", "-r", platformName, platformName+"clean"]);
	// regenerate include/ with syncqt -copy
	execute(["ssh", login, "cygpath", "-w", "`pwd`/" + platformName + "clean"]);
	var windowsPath = Process.stdout.split("\n")[0];
	execute(["ssh", login, "rm -rf", platformName+"clean/include"]);
	execute(["ssh", login, "QTDIR='" + windowsPath + "'", "cmd", "/c",
		 "perl", platformName+"clean/bin/syncqt", "-copy"]);
	// remove src/
	execute(["ssh", login, "rm -rf", platformName+"clean/src"]);

	// copy build script
	var buildScript = p4Copy(p4BranchPath + "/util/scripts" ,"buildbinarywin.bat", p4Label);
	execute(["scp", buildScript, login + ":."]);

	// run it
	execute(["ssh", login, "cmd", "/c", "buildbinarywin.bat win32-msvc.net " + platformName]);

	// copy files from bin
	execute(["ssh", login, "cp", platformName + "/bin/*.exe", platformName+"clean/bin/."]);
	execute(["ssh", login, "cp", platformName + "/bin/*.dll", platformName+"clean/bin/."]);
	// copy files from lib
	execute(["ssh", login, "cp", platformName + "/lib/*.dll", platformName+"clean/lib/."]);
	execute(["ssh", login, "cp", platformName + "/lib/*.lib", platformName+"clean/lib/."]);
	execute(["ssh", login, "cp", platformName + "/lib/*.pdb", platformName+"clean/lib/."]);
	// copy the plugin directory
	execute(["ssh", login, "cp", "-r", platformName + "/plugins", platformName+"clean/."]);
	// copy generated qconfig.h/.cpp
	execute(["ssh", login, "cp",
		 platformName + "/src/core/global/qconfig.h",
		 platformName + "clean/include/Qt/qconfig.h"]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/global/qconfig.h",
		 platformName + "clean/include/QtCore/qconfig.h"]);
	// copy arch/qatomic.h
	execute(["ssh", login, "mkdir", "-p", platformName + "clean/include/Qt/arch"]);
	execute(["ssh", login, "mkdir", "-p", platformName + "clean/include/QtCore/arch"]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/arch/windows/arch/qatomic.h",
		 platformName + "clean/include/Qt/arch/."]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/arch/windows/arch/qatomic.h",
		 platformName + "clean/include/QtCore/arch/."]);
	// copy qatomic.h
	execute(["ssh", login, "cp",
		 platformName + "/src/core/thread/qatomic.h",
		 platformName + "clean/include/Qt/."]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/thread/qatomic.h",
		 platformName + "clean/include/QtCore/."]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/thread/qatomic.h",
		 platformName + "clean/include/QtCore/QAtomic"]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/thread/qatomic.h",
		 platformName + "clean/include/QtCore/QBasicAtomic"]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/thread/qatomic.h",
		 platformName + "clean/include/QtCore/QAtomicPointer"]);
	execute(["ssh", login, "cp",
		 platformName + "/src/core/thread/qatomic.h",
		 platformName + "clean/include/QtCore/QBasicAtomicPointer"]);
	// copy generated .qt.config
	execute(["ssh", login, "cp",
		 platformName + "/mkspecs/.qt.config",
		 platformName + "clean/mkspecs/.qt.config"]);

	// replace tags in installscript.nsi
	var installScript = p4Copy(p4BranchPath + "/util/scripts", "installscriptwin.nsi",
				   p4Label);
	var extraTags = new Array();
	extraTags[windowsPath] = /\%PACKAGEDIR\%/g;
	var scriptFile = new File(installScript);
	replaceTags(scriptFile.path, ["installscriptwin.nsi"], platform, edition, platformName,
		    extraTags);

	// copy over the install scipt files
	var installWriteEnv = p4Copy(p4BranchPath + "/util/scripts", "writeEnvStr.nsh", p4Label);
	var installWritePath = p4Copy(p4BranchPath + "/util/scripts", "writePathStr.nsh", p4Label);
	var installLicensePage = p4Copy(p4BranchPath + "/util/scripts", "checkqtlicense.ini",
					p4Label);
	var installEnvPage = p4Copy(p4BranchPath + "/util/scripts", "setenvpage.ini", p4Label);
	execute(["scp", installScript, login + ":."]);
	execute(["scp", installWriteEnv, login + ":."]);
	execute(["scp", installWritePath, login + ":."]);
	execute(["scp", installLicensePage, login + ":."]);
	execute(["scp", installEnvPage, login + ":."]);

	// copy over the latest version of the install dll to the nsis plugins dir
	var installDll = p4Copy(p4BranchPath + "/util/scripts/qtnsisext", "qtnsisext.dll",
				p4Label);
	execute(["ssh", login, "which", "makensis.exe"]);
	var nsisPluginsPath = Process.stdout.split("\n")[0];
	nsisPluginsPath = nsisPluginsPath.left(nsisPluginsPath.lastIndexOf("/")) + "/plugins";
	execute(["scp", installDll, login + ":'" + nsisPluginsPath + "'"]);

	// run the install script and create compiler
	execute(["ssh", login, "cmd", "/c", "makensis.exe", "installscriptwin.nsi"]);

	//copy the result back
	execute(["scp", login + ":" + platformName + ".exe", outputDir + "/."]);


    } else if (platform == "mac") {

    }

    // clean up on host after building binaries
//     execute(["ssh", login, "rm -rf", platformName + "*"]);
//     execute(["ssh", login, "rm -rf", "buildbinary" + platform + "*"]);
//     execute(["ssh", login, "rm -rf", "installscript" + platform + "*"]);
//     execute(["ssh", login, "rm -rf", "write*.nsh"]);
//     execute(["ssh", login, "rm -rf", "checkqtlicense.ini"]);
//     execute(["ssh", login, "rm -rf", "setenvpage.ini"]);
}

/************************************************************
 * gets a list of all files and subdirectories relative to the specified directory (not absolutePath)
 */
function getFileList(rootDir)
{
    var dir = new Dir(rootDir);
    dir.setCurrent();
    var rootLength = dir.absPath.length + 1; // +1 because "/" is not included in absPath
    var result = new Array();

    // add files to result
    var files = dir.entryList("*", Dir.Files | Dir.Hidden | Dir.System, Dir.Name);
    for (var f in files)
	result.push(files[f]);

    // expand dirs to absolute path
    var dirs = new Array();
    var tempDirs = dir.entryList("*", Dir.Dirs | Dir.Hidden | Dir.System, Dir.Name);
    for (var t in tempDirs) {
	if (tempDirs[t] != "." && tempDirs[t] != "..")
	    dirs.push(dir.absFilePath(tempDirs[t]));
    }
    
    for (var i=0; i<dirs.length; ++i) {
 	// cd to directory and add directory to result
 	dir.cd(dirs[i]);
 	result.push(dirs[i].right(dirs[i].length - rootLength));

	// add files
	var files = dir.entryList("*", Dir.Files | Dir.Hidden | Dir.System, Dir.Name);
	for (var f in files)
	    result.push(dir.absFilePath(files[f]).right(dir.absFilePath(files[f]).length - rootLength));

	// adds subDirs to dirs
	tempDirs = dir.entryList("*", Dir.Dirs | Dir.Hidden | Dir.System, Dir.Name);
	for (var t in tempDirs) {
	    if (tempDirs[t] != "." && tempDirs[t] != "..")
		dirs.push(dir.absFilePath(tempDirs[t]));
	}
    }
    return result;
}

/************************************************************
 * cleans up temp files
 */
function cleanup()
{
    // deletes distDir
    var dir = new Dir(tmpDir);
    dir.setCurrent();
    dir = new Dir(distDir);
    if (dir.exists)
	dir.rmdirs();
}


/************************************************************
 * copies the special dist files according to platform and edition
 * and populates the licenseHeaders array
 */
function copyDist(packageDir, platform, edition)
{
    var platformFiles = getFileList(packageDir + "/dist/" + platform);
    var editionFiles = getFileList(packageDir + "/dist/" + edition);

    //copies changes file to root
    var changesFile = packageDir + "/dist/changes-" + options["version"];
    if (File.exists(changesFile))
	execute(["cp", changesFile, packageDir]);
    
    //copies default README to root
    var readmeFile = packageDir + "/dist/README";
    if (File.exists(readmeFile))
	execute(["cp", readmeFile, packageDir]);

    // copies any platform specific files
    for (var i in platformFiles) {
	var fileName = platformFiles[i];
	var absFileName = packageDir + "/dist/" + platform + "/" + fileName;
	if (File.exists(absFileName) && File.isFile(absFileName)) {
	    var dir = new Dir(new File(packageDir + "/" + fileName).path);
	    if (!dir.exists)
		dir.mkdirs();
	    execute(["cp", absFileName, packageDir + "/" + fileName]);
	}
    }

    // copies any edition specific files
    for (var i in editionFiles) {
	var fileName = editionFiles[i];
	var absFileName = packageDir + "/dist/" + edition + "/" + fileName;
	if (File.exists(absFileName) && File.isFile(absFileName))
	    execute(["cp", absFileName, packageDir + "/" + fileName]);
    }

    // rename any LICENSE and LICENSE-US to hidden . files
    var dir = new Dir(packageDir);
    if (dir.fileExists("LICENSE"))
	dir.rename("LICENSE", ".LICENSE");
    if (dir.fileExists("LICENSE-US"))
	dir.rename("LICENSE-US", ".LICENSE-US");
    if (dir.fileExists("LICENSE-COMBINED"))
	dir.rename("LICENSE-COMBINED", "LICENSE.PREVIEW");

    // populate licenseHeaders with all files found in dist/licenses
    var licenseFiles = getFileList(packageDir + "/dist/licenses");
    for (var i in licenseFiles) {
	var fileName = licenseFiles[i];
	var absFileName = packageDir + "/dist/licenses/" + fileName;
	if (File.exists(absFileName) && File.isFile(absFileName))
	    licenseHeaders[fileName] = File.read(absFileName);
    }

    //check that key files are present
    var keyFiles = ["README",
		    "INSTALL",
		    "PLATFORMS"];
    if (!options["snapshots"] && (edition != "preview" || edition != "beta"))
	keyFiles.push("changes-" + options["version"]);
    if (edition == "opensource") {
	keyFiles.push("LICENSE.GPL");
	keyFiles.push("LICENSE.QPL");
    } else {
	keyFiles.push(".LICENSE");
	keyFiles.push(".LICENSE-US");
    }
    for (var i in keyFiles) {
	if (!File.exists(packageDir + "/" + keyFiles[i]))
	    warning("Missing %1 in package.".arg(packageDir + "/" + keyFiles[i]));
    }
}

/************************************************************
 * runs syncqt in packageDir with the specified platform
 */
function syncqt(packageDir, platform)
{
    var dir = new Dir(packageDir);
    dir.setCurrent();
    System.setenv("QTDIR", packageDir);
    var syncqtCommand = packageDir + "/bin/syncqt";
    if (platform == "win")
	execute([syncqtCommand, "-windows"]);
    else
	execute([syncqtCommand]);
}

/************************************************************
 * runs qdoc on packageDir
 */
function qdoc(packageDir, edition)
{
    var dir = new Dir(packageDir);
    dir.setCurrent();
    System.setenv("QTDIR", packageDir);
    var qdocConfigFile = qdocDir + "/test/qt-" + edition + ".qdocconf";
    if (!File.exists(qdocConfigFile))
	throw "Missing qdoc configuratio file: %1".arg(qdocConfigFile);
    execute([qdocCommand, qdocConfigFile]);
}

/************************************************************
 * goes through all txt files and replaces tags like %VERSION%, %THISYEAR% etc.
 */
function replaceTags(packageDir, fileList, platform, edition, platName, additionalTags)
{
    var replace = new Array();
    replace[startDate.getYear().toString()] = /\$THISYEAR\$/g;
    replace[options["version"]] = /\%VERSION\%/g;
    replace["#define QT_VERSION_STR   \"" + options["version"] + "\""] =
	/#\s*define\s+QT_VERSION_STR\s+\"([^\"]+)\"*/g;
    replace[platName] = /\%DISTNAME\%/g;

    if (platform + "-" + edition in licenseHeaders)
	replace[licenseHeaders[platform+"-"+edition]] = /\*\* \$LICENSE\$\n/;
    else
	replace[licenseHeaders[edition]] = /\*\* \$LICENSE\$\n/;
    for (var i in additionalTags)
	replace[i] = additionalTags[i];


    var fileName = new String();
    var absFileName = new String();
    var content = new String();
    for (var i in fileList) {
	fileName = fileList[i];
	absFileName = packageDir + "/" + fileName;
	//only replace for non binary files
	if (File.isFile(absFileName) &&  !binaryFile(absFileName)) {
	    content = File.read(absFileName);
	    for (var i in replace)
		content = content.replace(replace[i], i);
	    // special case for $MODULE$
	    if (content.find(/\$MODULE\$/) != -1) {
		var match = false;
		for (var i in moduleMap) {
		    if (fileName.find(moduleMap[i]) != -1) {
			content = content.replace(/\$MODULE\$/, i);
			match = true;
			break;
		    }
		}
		if (!match)
		    warning("No module map for: " + fileName);
	    }
	    File.write(absFileName, content);
	}
    }
}

/************************************************************
 * returns a local writable copy of fileName specified in the
 * depotPath
 * 
 * typically used when files are needed from the depot that can have
 * been purged from the package (like qdoc configurations and scripts
 * etc.)
 */
function p4Copy(depotPath, fileName, label)
{
    var depotFileName = depotPath + "/" + fileName + "@" + label;
    var newFile = distDir + "/" + fileName;
    execute([p4Command, "print", "-o", newFile, "-q", depotFileName]);
    if (!File.exists(newFile))
	throw "Failed copying file: %1 to: %2".arg(depotFileName).arg(newFile);
    execute(["chmod", "ug+w", newFile]);
    return newFile;
}

/************************************************************
 * prints out text with indentation
 */
function print(text)
{
    var i = indentation;
    var spaces = new String();
    while (i--)
	spaces += ' ';
    System.println(spaces + text);
}

/************************************************************
 * prints out warning text with indentation
 */
function warning(text)
{
    print("** Warning! ** " + text);
}

/************************************************************
 * returns true if the file exists, is a file, is executable or has a binary extension
 */
function binaryFile(fileName)
{
    if (File.exists(fileName) && File.isFile(fileName)) {
	var file = new File(fileName);
	if (file.executable) {
	    file.open(File.ReadOnly);
	    var isScript = file.readLine().lower().startsWith("#!");
	    file.close();
	    if (isScript)
		return false;
	    return true;
	} else {
	    for (var i in binaryExtensions)
		if (file.extension.lower() == binaryExtensions[i])
		    return true;
	}
    }
    return false;
}


/************************************************************
 * runs the command and prints out stderror if not empty
 */
function execute(command, stdin) {
    var start = Date().getTime();
    var error = Process.execute(command, stdin);
    var runTime = Math.floor((Date().getTime() - start)/1000);
    if (runTime > 0)
	print("%1\n   ->took %1 second(s)".arg(command).arg(runTime));
    if (error != 0) {
	throw "Error runnning: %1 stderr: %2".arg(command).arg(Process.stderr.left(200));
    } else if (Process.stderr.length > 0
	       && Process.stderr.left(200).lower().find(/warning|error/) != -1) {
	warning("Running %1 stderr: %2".arg(command).arg(Process.stderr.left(200)));
    }
}
