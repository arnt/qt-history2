/************************************************************
 * Some global variables needed throughout the packaging script.
 */
const qtDir = System.getenv("QTDIR");
const qmakeCommand = qtDir + "/bin/qmake";

const qdocDir = qtDir + "/util/qdoc";
const qdocCommand = qdocDir + "/qdoc";

const outputDir = System.getenv("PWD");

const validPlatforms = ["win", "x11", "mac", "embedded"];
const validEditions = ["free", "commercial", "preview"];
const validSwitches = ["gzip", "bzip", "zip"]; // these are either true or false, set by -do-foo/-no-foo
const validVars = ["branch", "version"];       // variables with arbitrary values, set by -foo value

const binaryExtensions = ["msi", "dll", "gif", "png", "mng",
			  "jpg", "bmp", "any", "pic", "ppm",
			  "exe", "zip", "qm", "ico", "wav",
			  "icns"];

const user = System.getenv("USER");

var options = [];	 // list of all package options
var tmpDir;		 // directory for system temporary files
var distDir;		 // parent directory for all temp packages and the checkout dir
var checkoutDir;	 // directory for P4 checkout
var licenseHeaders = []; // license text to put in .cpp and .h headers
var moduleMap = [];      // maps between directories and module/class/application names
var p4Port;
var p4Command;

var indentation = 0;
const tabSize = 4;

/************************************************************
 * Purging filters that will be moved into files later
 */

var checkoutRemove = [ new RegExp("^tests"),
		       new RegExp("^tmake"),
		       new RegExp("^util"),
		       new RegExp("^examples"),
		       new RegExp("^tutorial"),
		       new RegExp("^translations"),
		       new RegExp("^pics"),
		       new RegExp("^extensions/xt"),
		       new RegExp("^tools/designer"),
		       new RegExp("^tools/linguist"),
		       new RegExp("^tools/makeqpf"),
		       new RegExp("^tools/mergetr"),
		       new RegExp("^tools/msg2qm"),
		       new RegExp("^tools/qconfig"),
		       new RegExp("^tools/qembed"),
		       new RegExp("^tools/qvfb"),

		       new RegExp("^src/gui/painting/makepsheader.pl"),
		       new RegExp("^src/gui/painting/qpsprinter"),
		       new RegExp("^src/gui/itemviews/qlistview"),
		       new RegExp("^src/gui/itemviews/qtreeview"),
		       new RegExp("^src/gui/itemviews/qtableview"),
		       new RegExp("^LICENSE.TROLL") ];
var checkoutKeep = [ /./ ];

var platformRemove = new Array();
var platformKeep = new Array();

var editionRemove = new Array();
var editionKeep = new Array();

platformRemove["win"] = [ new RegExp("^gif"),
			  new RegExp("^dist"),
			  new RegExp("^config.tests"),
			  new RegExp("^extensions/motif"),
			  new RegExp("^include/QtMotif"),
			  new RegExp("^extensions/nsplugin"),
			  new RegExp("^include/QtNsPlugin"),
			  new RegExp("^src/plugins/gfxdrivers"),
			  new RegExp("_x11"),
			  new RegExp("_unix"),
			  new RegExp("_qws"),
			  new RegExp("_wce"),
			  new RegExp("_mac"),
			  new RegExp("_qnx4"),
			  new RegExp("_qnx6"),
			  new RegExp("^configure"),
			  new RegExp("^bin/syncqt"),
			  new RegExp("^Makefile.win32-g++"),
			  new RegExp("^install.exe") ];
platformKeep["win"] = [ new RegExp(".") ];

platformRemove["x11"] = [ new RegExp("^gif"),
			  new RegExp("^dist"),
			  new RegExp("^extensions"),
			  new RegExp("^include/ActiveQt"),
			  new RegExp("^include/QtNsPlugin"),
			  new RegExp("^include/QtMotif"),
			  new RegExp("^src/plugins/gfxdrivers"),
			  new RegExp("_win"),
			  new RegExp("_qws"),
			  new RegExp("_wce"),
			  new RegExp("_mac"),
			  new RegExp("_qnx4"),
			  new RegExp("_qnx6"),
			  new RegExp("^bin/syncqt"),
			  new RegExp("^bin/configure.exe") ];
platformKeep["x11"] = [ new RegExp(".") ];

platformRemove["mac"] = [ new RegExp("^gif"),
			  new RegExp("^dist"),
			  new RegExp("^extensions"),
			  new RegExp("^include/ActiveQt"),
			  new RegExp("^include/QtNsPlugin"),
			  new RegExp("^include/QtMotif"),
			  new RegExp("^src/plugins/gfxdrivers"),
			  new RegExp("_win"),
			  new RegExp("_qws"),
			  new RegExp("_wce"),
			  new RegExp("_x11"),
			  new RegExp("_qnx4"),
			  new RegExp("_qnx6"),
			  new RegExp("^bin/syncqt"),
			  new RegExp("^bin/configure.exe") ];
platformKeep["mac"] = [ new RegExp(".") ];

editionRemove["commercial"] = [ new RegExp("GPL") ];
editionKeep["commercial"] = [ new RegExp(".") ];
editionRemove["preview"] = [ new RegExp("GPL") ];
editionKeep["preview"] = [ new RegExp(".") ];

/************************************************************
 * Mapping from directories to module names
 */

moduleMap["demonstration applications"]  = new RegExp("^demos");
moduleMap["example classes"]             = new RegExp("^examples");
moduleMap["qmake application"]           = new RegExp("^qmake");
moduleMap["activeqt module"]             = new RegExp("^extensions/activeqt");
moduleMap["motif module"]                = new RegExp("^extensions/motif");
moduleMap["compatibility classes"]       = new RegExp("^src/compat");
moduleMap["core module"]                 = new RegExp("^src/core");
moduleMap["accessibility module"]        = new RegExp("(^src/gui/accessible|^src/plugins/accessible)");
moduleMap["dialog module"]               = new RegExp("^src/gui/dialogs");
moduleMap["embedded classes"]            = new RegExp("(^src/gui/embedded|^src/plugins/gfxdrivers)");
moduleMap["item views module"]           = new RegExp("^src/gui/itemviews");
moduleMap["gui module"]                  = new RegExp("^src/gui/kernel");
moduleMap["painting module"]             = new RegExp("(^src/gui/painting|^src/gui/image|^src/plugins/imageformats)");
moduleMap["style module"]                = new RegExp("(^src/gui/styles|^src/plugins/styles)");
moduleMap["text module"]                 = new RegExp("^src/gui/text");
moduleMap["widgets module"]              = new RegExp("^src/gui/widgets");
moduleMap["moc application"]             = new RegExp("^src/moc");
moduleMap["network module"]              = new RegExp("^src/network");
moduleMap["opengl module"]               = new RegExp("^src/opengl");
moduleMap["internationalization module"] = new RegExp("^src/plugins/codecs");
moduleMap["sql module"]                  = new RegExp("(^src/sql|^src/plugins/sqldrivers)");
moduleMap["tools applications"]          = new RegExp("^src/tools");
moduleMap["window classes"]              = new RegExp("^src/winmain");
moduleMap["xml module"]                  = new RegExp("^src/xml");
moduleMap["assistant application"]       = new RegExp("^tools/assistant");
moduleMap["qtconfig application"]        = new RegExp("^tools/qtconfig");

/*******************************************************************************
 * Here we go
 */
print("Initializing...");
parseArgc();
initialize();
print("Checking tools...");
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

	    // run syncqt
  	    print("Running syncqt...");
  	    syncqt(platDir, platform);

	    // run qdoc
  	    print("Running qdoc...");
  	    qdoc(platDir);

  	    // purge platform and edition files
  	    print("Purging platform and edition specific files...");
  	    purgeFiles(platDir,
		       getFileList(platDir),
  		       [].concat(platformRemove[platform]).concat(editionRemove[edition]),
  		       [].concat(platformKeep[platform]).concat(editionKeep[edition]));

	    // replace tags (like THISYEAR etc.)
	    print("Traversing all txt files and replacing tags...");
	    replaceTags(platDir, getFileList(platDir), platform, edition, platName);

  	    // package directory
	    print("Compressing and packaging file(s)...")
	    compress(platDir, platform, edition);
	    
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
    validOptions =
	validPlatforms.toString() +
	validEditions.toString() +
	validSwitches.toString() +
	validVars.toString();
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
    // checks that all valid vars are specified
    for (var i in validVars)
	if (!(validVars[i] in options))
	    throw "%1 was not specified.".arg(validVars[i]);

    // by default turn on all valid switches that were not defined
    for (var i in validSwitches)
	if (!(validSwitches[i] in options))
	    options[validSwitches[i]] = true;

    // by default turn on all valid platforms that were not defined
    for (var i in validPlatforms)
	if (!(validPlatforms[i] in options))
	    options[validPlatforms[i]] = true;

    // by default turn on all valid editions that were not defined
    for (var i in validEditions)
	if (!(validEditions[i] in options))
	    options[validEditions[i]] = true;

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
    distDir = tmpDir + "/qt-" + options["branch"] + "-" + user + "-" + Date().getTime();
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

//     for (var i in options)
// 	print("options[%1] = %2".arg(i).arg(options[i]));
}

/************************************************************
 * Verify that the necesary tools are available.
 */
function checkTools()
{
    try {
	execute([qmakeCommand, "-help"]);
	execute("zip -help");
	execute("tar --help");
	execute("gzip -h");
 	execute("bzip2 -h");
	execute("cp --help");
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
    execute("%1 qdoc.pro".arg(qmakeCommand));
    if (execute("make") != 0)
	throw "Failed to build qdoc:\n %1".arg(Process.stderr);
    // test qdoc
    execute( [qdocCommand, "--help"] );
}


/************************************************************
 * checkouts from P4 and puts everything in checkoutDir
 */
function checkout()
{
    // check that the branch exist
    var branchPath = "//depot/qt/" + options["branch"];
    execute([p4Command, "fstat", branchPath + "/configure"]);
    if (Process.stdout.find("depotFile") == -1)
	throw "Branch: " + branchPath + " does not exist.";
    
    // check that the label exists
    var label = "qt/" + options["version"];
    execute([p4Command, "labels", branchPath + "/configure"]);
    if (Process.stdout.find("Label " + label + " ") == -1)
	throw "Label: " + label + " does not exist, or not in this branch.";

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
    execute([p4Command, "-c", tmpClient, "-d", distDir, "sync", "-f", "...@" + label]);

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
function compress(packageDir, platform, edition)
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
	// remove .tar
	if (File.exists(tarFile))
	    File.remove(tarFile);
    }
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
	if (File.exists(absFileName) && File.isFile(absFileName))
	    execute(["cp", absFileName, packageDir + "/" + fileName]);
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
    dir.rename("LICENSE", ".LICENSE");
    dir.rename("LICENSE-US", ".LICENSE-US");

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
		    ".LICENSE",
		    ".LICENSE-US",
		    "INSTALL",
		    "PLATFORMS",
		    "MANIFEST",
		    "changes-" + options["version"]];
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
function qdoc(packageDir)
{
    var dir = new Dir(packageDir);
    dir.setCurrent();
    System.setenv("QTDIR", packageDir);
    execute([qdocCommand, qtDir + "/util/qdoc/qdoc.conf"]);
}

/************************************************************
 * goes through all txt files and replaces tags like %VERSION%, %THISYEAR% etc.
 */
function replaceTags(packageDir, fileList, platform, edition, platName)
{
    var replace = new Array();
    replace[Date().getYear().toString()] = /\$THISYEAR\$/g;
    replace[options["version"]] = /\%VERSION\%/g;
    replace[platName] = /\%DISTNAME\%/g;
    if (platform + "-" + edition in licenseHeaders)
	replace[licenseHeaders[platform+"-"+edition]] = /\*\* \$LICENSE\$\n/;
    else
	replace[licenseHeaders[edition]] = /\*\* \$LICENSE\$\n/;
    
    var fileName = new String();
    var absFileName = new String();
    var content = new String();
    for (var i in fileList) {
	fileName = fileList[i];
	absFileName = packageDir + "/" + fileName;
	//only replace in non binaries but not for .html files
	if (File.isFile(absFileName) &&
	    !binaryFile(absFileName) &&
	    !absFileName.endsWith(".html")) {
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
 * runs the command and throws an exception if stderror is not empty 
 */
function execute(command, stdin) {
    var error = Process.execute(command, stdin);
    if (Process.stderr.length > 0)
	warning("Running %1 stderr: %2".arg(command).arg(Process.stderr.left(40)))
    return error;
}
