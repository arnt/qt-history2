/****************************************************************************
**
** Implementation of VCProject class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "msvc_objectmodel.h"
#include "msvc_vcproj.h"
#include <qstringlist.h>
#include <qfileinfo.h>

// XML Tags ---------------------------------------------------------
const char* _Configuration                      = "Configuration";
const char* _Configurations                     = "Configurations";
const char* _File                               = "File";
const char* _FileConfiguration                  = "FileConfiguration";
const char* _Files                              = "Files";
const char* _Filter                             = "Filter";
const char* _Globals                            = "Globals";
const char* _Platform                           = "Platform";
const char* _Platforms                          = "Platforms";
const char* _Tool                               = "Tool";
const char* _VisualStudioProject                = "VisualStudioProject";

// XML Properties ---------------------------------------------------
const char* _AddModuleNamesToAssembly           = "AddModuleNamesToAssembly";
const char* _AdditionalDependencies             = "AdditionalDependencies";
const char* _AdditionalIncludeDirectories       = "AdditionalIncludeDirectories";
const char* _AdditionalLibraryDirectories       = "AdditionalLibraryDirectories";
const char* _AdditionalOptions                  = "AdditionalOptions";
const char* _AdditionalUsingDirectories         = "AdditionalUsingDirectories";
const char* _AssemblerListingLocation           = "AssemblerListingLocation";
const char* _AssemblerOutput                    = "AssemblerOutput";
const char* _ATLMinimizesCRunTimeLibraryUsage   = "ATLMinimizesCRunTimeLibraryUsage";
const char* _BaseAddress                        = "BaseAddress";
const char* _BasicRuntimeChecks                 = "BasicRuntimeChecks";
const char* _BrowseInformation                  = "BrowseInformation";
const char* _BrowseInformationFile              = "BrowseInformationFile";
const char* _BufferSecurityCheck                = "BufferSecurityCheck";
const char* _BuildBrowserInformation            = "BuildBrowserInformation";
const char* _CPreprocessOptions                 = "CPreprocessOptions";
const char* _CallingConvention                  = "CallingConvention";
const char* _CharacterSet                       = "CharacterSet";
const char* _CommandLine                        = "CommandLine";
const char* _CompileAs                          = "CompileAs";
const char* _CompileAsManaged                   = "CompileAsManaged";
const char* _CompileOnly                        = "CompileOnly";
const char* _ConfigurationType                  = "ConfigurationType";
const char* _Culture                            = "Culture";
const char* _DLLDataFileName                    = "DLLDataFileName";
const char* _DebugInformationFormat             = "DebugInformationFormat";
const char* _DefaultCharIsUnsigned              = "DefaultCharIsUnsigned";
const char* _DefaultCharType                    = "DefaultCharType";
const char* _DelayLoadDLLs                      = "DelayLoadDLLs";
const char* _DeleteExtensionsOnClean            = "DeleteExtensionsOnClean";
const char* _Description                        = "Description";
const char* _Detect64BitPortabilityProblems     = "Detect64BitPortabilityProblems";
const char* _DisableLanguageExtensions          = "DisableLanguageExtensions";
const char* _DisableSpecificWarnings            = "DisableSpecificWarnings";
const char* _EnableCOMDATFolding                = "EnableCOMDATFolding";
const char* _EnableErrorChecks                  = "EnableErrorChecks";
const char* _EnableFiberSafeOptimizations       = "EnableFiberSafeOptimizations";
const char* _EnableFunctionLevelLinking         = "EnableFunctionLevelLinking";
const char* _EnableIntrinsicFunctions           = "EnableIntrinsicFunctions";
const char* _EntryPointSymbol                   = "EntryPointSymbol";
const char* _ErrorCheckAllocations              = "ErrorCheckAllocations";
const char* _ErrorCheckBounds                   = "ErrorCheckBounds";
const char* _ErrorCheckEnumRange                = "ErrorCheckEnumRange";
const char* _ErrorCheckRefPointers              = "ErrorCheckRefPointers";
const char* _ErrorCheckStubData                 = "ErrorCheckStubData";
const char* _ExceptionHandling                  = "ExceptionHandling";
const char* _ExcludedFromBuild                  = "ExcludedFromBuild";
const char* _ExpandAttributedSource             = "ExpandAttributedSource";
const char* _ExportNamedFunctions               = "ExportNamedFunctions";
const char* _FavorSizeOrSpeed                   = "FavorSizeOrSpeed";
const char* _ForceConformanceInForLoopScope     = "ForceConformanceInForLoopScope";
const char* _ForceSymbolReferences              = "ForceSymbolReferences";
const char* _ForcedIncludeFiles                 = "ForcedIncludeFiles";
const char* _ForcedUsingFiles                   = "ForcedUsingFiles";
const char* _FullIncludePath                    = "FullIncludePath";
const char* _FunctionOrder                      = "FunctionOrder";
const char* _GenerateDebugInformation           = "GenerateDebugInformation";
const char* _GenerateMapFile                    = "GenerateMapFile";
const char* _GeneratePreprocessedFile           = "GeneratePreprocessedFile";
const char* _GenerateStublessProxies            = "GenerateStublessProxies";
const char* _GenerateTypeLibrary                = "GenerateTypeLibrary";
const char* _GlobalOptimizations                = "GlobalOptimizations";
const char* _HeaderFileName                     = "HeaderFileName";
const char* _HeapCommitSize                     = "HeapCommitSize";
const char* _HeapReserveSize                    = "HeapReserveSize";
const char* _IgnoreAllDefaultLibraries          = "IgnoreAllDefaultLibraries";
const char* _IgnoreDefaultLibraryNames          = "IgnoreDefaultLibraryNames";
const char* _IgnoreEmbeddedIDL                  = "IgnoreEmbeddedIDL";
const char* _IgnoreImportLibrary                = "IgnoreImportLibrary";
const char* _IgnoreStandardIncludePath          = "IgnoreStandardIncludePath";
const char* _ImportLibrary                      = "ImportLibrary";
const char* _ImproveFloatingPointConsistency    = "ImproveFloatingPointConsistency";
const char* _InlineFunctionExpansion            = "InlineFunctionExpansion";
const char* _InterfaceIdentifierFileName        = "InterfaceIdentifierFileName";
const char* _IntermediateDirectory              = "IntermediateDirectory";
const char* _KeepComments                       = "KeepComments";
const char* _LargeAddressAware                  = "LargeAddressAware";
const char* _LinkDLL                            = "LinkDLL";
const char* _LinkIncremental                    = "LinkIncremental";
const char* _LinkTimeCodeGeneration             = "LinkTimeCodeGeneration";
const char* _LinkToManagedResourceFile          = "LinkToManagedResourceFile";
const char* _MapExports                         = "MapExports";
const char* _MapFileName                        = "MapFileName";
const char* _MapLines                           = "MapLines ";
const char* _MergeSections                      = "MergeSections";
const char* _MergedIDLBaseFileName              = "MergedIDLBaseFileName";
const char* _MidlCommandFile                    = "MidlCommandFile";
const char* _MinimalRebuild                     = "MinimalRebuild";
const char* _MkTypLibCompatible                 = "MkTypLibCompatible";
const char* _ModuleDefinitionFile               = "ModuleDefinitionFile";
const char* _Name                               = "Name";
const char* _ObjectFile                         = "ObjectFile";
const char* _OmitFramePointers                  = "OmitFramePointers";
const char* _Optimization                       = "Optimization ";
const char* _OptimizeForProcessor               = "OptimizeForProcessor";
const char* _OptimizeForWindows98               = "OptimizeForWindows98";
const char* _OptimizeForWindowsApplication      = "OptimizeForWindowsApplication";
const char* _OptimizeReferences                 = "OptimizeReferences";
const char* _OutputDirectory                    = "OutputDirectory";
const char* _OutputFile                         = "OutputFile";
const char* _Outputs                            = "Outputs";
const char* _ParseFiles                         = "ParseFiles";
const char* _PrecompiledHeaderFile              = "PrecompiledHeaderFile";
const char* _PrecompiledHeaderThrough           = "PrecompiledHeaderThrough";
const char* _PreprocessorDefinitions            = "PreprocessorDefinitions";
const char* _PrimaryOutput                      = "PrimaryOutput";
const char* _ProjectGUID                        = "ProjectGUID";
const char* _ProjectType                        = "ProjectType";
const char* _ProgramDatabase                    = "ProgramDatabase";
const char* _ProgramDataBaseFileName            = "ProgramDataBaseFileName";
const char* _ProgramDatabaseFile                = "ProgramDatabaseFile";
const char* _ProxyFileName                      = "ProxyFileName";
const char* _RedirectOutputAndErrors            = "RedirectOutputAndErrors";
const char* _RegisterOutput                     = "RegisterOutput";
const char* _RelativePath                       = "RelativePath";
const char* _ResourceOnlyDLL                    = "ResourceOnlyDLL";
const char* _ResourceOutputFileName             = "ResourceOutputFileName";
const char* _RuntimeLibrary                     = "RuntimeLibrary";
const char* _RuntimeTypeInfo                    = "RuntimeTypeInfo";
const char* _SccProjectName                     = "SccProjectName";
const char* _SccLocalPath                       = "SccLocalPath";
const char* _SetChecksum                        = "SetChecksum";
const char* _ShowIncludes                       = "ShowIncludes";
const char* _ShowProgress                       = "ShowProgress";
const char* _SmallerTypeCheck                   = "SmallerTypeCheck";
const char* _StackCommitSize                    = "StackCommitSize";
const char* _StackReserveSize                   = "StackReserveSize";
const char* _StringPooling                      = "StringPooling";
const char* _StripPrivateSymbols                = "StripPrivateSymbols";
const char* _StructMemberAlignment              = "StructMemberAlignment";
const char* _SubSystem                          = "SubSystem";
const char* _SupportUnloadOfDelayLoadedDLL      = "SupportUnloadOfDelayLoadedDLL";
const char* _SuppressStartupBanner              = "SuppressStartupBanner";
const char* _SwapRunFromCD                      = "SwapRunFromCD";
const char* _SwapRunFromNet                     = "SwapRunFromNet";
const char* _TargetEnvironment                  = "TargetEnvironment";
const char* _TargetMachine                      = "TargetMachine";
const char* _TerminalServerAware                = "TerminalServerAware";
const char* _Path                               = "Path";
const char* _TreatWChar_tAsBuiltInType          = "TreatWChar_tAsBuiltInType";
const char* _TurnOffAssemblyGeneration          = "TurnOffAssemblyGeneration";
const char* _TypeLibraryFile                    = "TypeLibraryFile";
const char* _TypeLibraryName                    = "TypeLibraryName";
const char* _TypeLibraryResourceID              = "TypeLibraryResourceID";
const char* _UndefineAllPreprocessorDefinitions = "UndefineAllPreprocessorDefinitions";
const char* _UndefinePreprocessorDefinitions    = "UndefinePreprocessorDefinitions";
const char* _UseOfATL                           = "UseOfATL";
const char* _UseOfMfc                           = "UseOfMfc";
const char* _UsePrecompiledHeader               = "UsePrecompiledHeader";
const char* _ValidateParameters                 = "ValidateParameters";
const char* _VCCLCompilerTool                   = "VCCLCompilerTool";
const char* _VCCustomBuildTool                  = "VCCustomBuildTool";
const char* _VCLibrarianTool                    = "VCLibrarianTool";
const char* _VCLinkerTool                       = "VCLinkerTool";
const char* _VCResourceCompilerTool             = "VCResourceCompilerTool";
const char* _VCMIDLTool                         = "VCMIDLTool";
const char* _Version                            = "Version";
const char* _WarnAsError                        = "WarnAsError";
const char* _WarnLevel                          = "WarnLevel";
const char* _WarningLevel                       = "WarningLevel";
const char* _WholeProgramOptimization           = "WholeProgramOptimization";

// XmlOutput stream functions ------------------------------
inline XmlOutput::xml_output attrT(const char *name, const triState v)
{
    if(v == unset)
        return noxml();
    return attr(name, (v == _True ? "true" : "false"));
}

inline XmlOutput::xml_output attrE(const char *name, int v)
{
    return attr(name, QString::number(v));
}

/*ifNot version*/ 
inline XmlOutput::xml_output attrE(const char *name, int v, int ifn)
{
    if (v == ifn)
        return noxml();
    return attr(name, QString::number(v));
}

inline XmlOutput::xml_output attrL(const char *name, long v)
{
    return attr(name, QString::number(v));
}

/*ifNot version*/ 
inline XmlOutput::xml_output attrL(const char *name, long v, long ifn)
{
    if (v == ifn)
        return noxml();
    return attr(name, QString::number(v));
}

inline XmlOutput::xml_output attrS(const char *name, const QString &v)
{
    if(v.isEmpty())
        return noxml();
    return attr(name, v);
}

inline XmlOutput::xml_output attrX(const char *name, const QStringList &v, const char *s = ",")
{
    if(v.isEmpty())
        return noxml();
    return attr(name, v.join(s));
}

// VCCLCompilerTool -------------------------------------------------
VCCLCompilerTool::VCCLCompilerTool()
    :        AssemblerOutput(asmListingNone),
        BasicRuntimeChecks(runtimeBasicCheckNone),
        BrowseInformation(brInfoNone),
        BufferSecurityCheck(_False),
        CallingConvention(callConventionDefault),
        CompileAs(compileAsDefault),
        CompileAsManaged(managedDefault),
        CompileOnly(unset),
        DebugInformationFormat(debugDisabled),
        DefaultCharIsUnsigned(unset),
        Detect64BitPortabilityProblems(unset),
        DisableLanguageExtensions(unset),
        EnableFiberSafeOptimizations(unset),
        EnableFunctionLevelLinking(unset),
        EnableIntrinsicFunctions(unset),
        ExceptionHandling(_False),
        ExpandAttributedSource(unset),
        FavorSizeOrSpeed(favorNone),
        ForceConformanceInForLoopScope(unset),
        GeneratePreprocessedFile(preprocessNo),
        GlobalOptimizations(unset),
        IgnoreStandardIncludePath(unset),
        ImproveFloatingPointConsistency(unset),
        InlineFunctionExpansion(expandDefault),
        KeepComments(unset),
        MinimalRebuild(unset),
        OmitFramePointers(unset),
        Optimization(optimizeCustom),
        OptimizeForProcessor(procOptimizeBlended),
        OptimizeForWindowsApplication(unset),
        ProgramDataBaseFileName(""),
        RuntimeLibrary(rtMultiThreaded),
        RuntimeTypeInfo(unset),
        ShowIncludes(unset),
        SmallerTypeCheck(unset),
        StringPooling(unset),
        StructMemberAlignment(alignNotSet),
        SuppressStartupBanner(unset),
        TreatWChar_tAsBuiltInType(unset),
        TurnOffAssemblyGeneration(unset),
        UndefineAllPreprocessorDefinitions(unset),
        UsePrecompiledHeader(pchNone),
        WarnAsError(unset),
        WarningLevel(warningLevel_0),
        WholeProgramOptimization(unset)
{
}

XmlOutput &operator<<(XmlOutput &xml, const VCCLCompilerTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, _VCCLCompilerTool)
            << attrX(_AdditionalIncludeDirectories, tool.AdditionalIncludeDirectories)
            << attrX(_AdditionalOptions, tool.AdditionalOptions, " ")
            << attrX(_AdditionalUsingDirectories, tool.AdditionalUsingDirectories)
            << attrS(_AssemblerListingLocation, tool.AssemblerListingLocation)
            << attrE(_AssemblerOutput, tool.AssemblerOutput, /*ifNot*/ asmListingNone)
            << attrE(_BasicRuntimeChecks, tool.BasicRuntimeChecks, /*ifNot*/ runtimeBasicCheckNone)
            << attrE(_BrowseInformation, tool.BrowseInformation, /*ifNot*/ brInfoNone)
            << attrS(_BrowseInformationFile, tool.BrowseInformationFile)
            << attrT(_BufferSecurityCheck, tool.BufferSecurityCheck)
            << attrE(_CallingConvention, tool.CallingConvention, /*ifNot*/ callConventionDefault)
            << attrE(_CompileAs, tool.CompileAs, compileAsDefault)
            << attrE(_CompileAsManaged, tool.CompileAsManaged, /*ifNot*/ managedDefault)
            << attrT(_CompileOnly, tool.CompileOnly)
            << attrE(_DebugInformationFormat, tool.DebugInformationFormat, /*ifNot*/ debugUnknown)
            << attrT(_DefaultCharIsUnsigned, tool.DefaultCharIsUnsigned)
            << attrT(_Detect64BitPortabilityProblems, tool.Detect64BitPortabilityProblems)
            << attrT(_DisableLanguageExtensions, tool.DisableLanguageExtensions)
            << attrX(_DisableSpecificWarnings, tool.DisableSpecificWarnings)
            << attrT(_EnableFiberSafeOptimizations, tool.EnableFiberSafeOptimizations)
            << attrT(_EnableFunctionLevelLinking, tool.EnableFunctionLevelLinking)
            << attrT(_EnableIntrinsicFunctions, tool.EnableIntrinsicFunctions)
            << attrT(_ExceptionHandling, tool.ExceptionHandling)
            << attrT(_ExpandAttributedSource, tool.ExpandAttributedSource)
            << attrE(_FavorSizeOrSpeed, tool.FavorSizeOrSpeed, /*ifNot*/favorNone)
            << attrT(_ForceConformanceInForLoopScope, tool.ForceConformanceInForLoopScope)
            << attrX(_ForcedIncludeFiles, tool.ForcedIncludeFiles)
            << attrX(_ForcedUsingFiles, tool.ForcedUsingFiles)
            << attrE(_GeneratePreprocessedFile, tool.GeneratePreprocessedFile, /*ifNot*/ preprocessUnknown)
            << attrT(_GlobalOptimizations, tool.GlobalOptimizations)
            << attrT(_IgnoreStandardIncludePath, tool.IgnoreStandardIncludePath)
            << attrT(_ImproveFloatingPointConsistency, tool.ImproveFloatingPointConsistency)
            << attrE(_InlineFunctionExpansion, tool.InlineFunctionExpansion, /*ifNot*/ expandDefault)
            << attrT(_KeepComments, tool.KeepComments)
            << attrT(_MinimalRebuild, tool.MinimalRebuild)
            << attrS(_ObjectFile, tool.ObjectFile)
            << attrT(_OmitFramePointers, tool.OmitFramePointers)
            << attrE(_Optimization, tool.Optimization, /*ifNot*/ optimizeDefault)
            << attrE(_OptimizeForProcessor, tool.OptimizeForProcessor, /*ifNot*/ procOptimizeBlended)
            << attrT(_OptimizeForWindowsApplication, tool.OptimizeForWindowsApplication)
            << attrS(_OutputFile, tool.OutputFile)
            << attrS(_PrecompiledHeaderFile, tool.PrecompiledHeaderFile)
            << attrS(_PrecompiledHeaderThrough, tool.PrecompiledHeaderThrough)
            << attrX(_PreprocessorDefinitions, tool.PreprocessorDefinitions)
            << (tool.ProgramDataBaseFileName.isNull() ? noxml() : attr(_ProgramDataBaseFileName, tool.ProgramDataBaseFileName))
            << attrE(_RuntimeLibrary, tool.RuntimeLibrary, /*ifNot*/ rtUnknown)
            << attrT(_RuntimeTypeInfo, tool.RuntimeTypeInfo)
            << attrT(_ShowIncludes, tool.ShowIncludes)
            << attrT(_SmallerTypeCheck, tool.SmallerTypeCheck)
            << attrT(_StringPooling, tool.StringPooling)
            << attrE(_StructMemberAlignment, tool.StructMemberAlignment, /*ifNot*/ alignNotSet)
            << attrT(_SuppressStartupBanner, tool.SuppressStartupBanner)
            << attrT(_TreatWChar_tAsBuiltInType, tool.TreatWChar_tAsBuiltInType)
            << attrT(_TurnOffAssemblyGeneration, tool.TurnOffAssemblyGeneration)
            << attrT(_UndefineAllPreprocessorDefinitions, tool.UndefineAllPreprocessorDefinitions)
            << attrX(_UndefinePreprocessorDefinitions, tool.UndefinePreprocessorDefinitions)
            << (!tool.PrecompiledHeaderFile.isEmpty() || !tool.PrecompiledHeaderThrough.isEmpty() ? attrE(_UsePrecompiledHeader, tool.UsePrecompiledHeader) : noxml())
            << attrT(_WarnAsError, tool.WarnAsError)
            << attrE(_WarningLevel, tool.WarningLevel, /*ifNot*/ warningLevelUnknown)
            << attrT(_WholeProgramOptimization, tool.WholeProgramOptimization)
        << closetag(_Tool);
}

bool VCCLCompilerTool::parseOption(const char* option)
{
    // skip index 0 ('/' or '-')
    char first  = option[1];
    char second = option[2];
    char third  = option[3];
    char fourth = option[4];
    bool found = true;

    switch (first) {
    case '?':
    case 'h':
        qWarning("Generator: Option '/?', '/help': MSVC.NET projects do not support outputting help info");
        found = false;
        break;
    case '@':
        qWarning("Generator: Option '/@': MSVC.NET projects do not support the use of a response file");
        found = false;
        break;
    case 'l':
        qWarning("Generator: Option '/link': qmake generator does not support passing link options through the compiler tool");
        found = false;
        break;
    case 'A':
        if(second != 'I') {
            found = false; break;
        }
        AdditionalUsingDirectories += option+3;
        break;
    case 'C':
        KeepComments = _True;
        break;
    case 'D':
        PreprocessorDefinitions += option+2;
        break;
    case 'E':
        if(second == 'H') {
            if(third == 'a'
                || (third == 'c' && fourth != 's')
                || (third == 's' && fourth != 'c')) {
                // ExceptionHandling must be false, or it will override
                // with an /EHsc option
                ExceptionHandling = _False;
                AdditionalOptions += option;
                break;
            } else if((third == 'c' && fourth == 's')
                     || (third == 's' && fourth == 'c')) {
                ExceptionHandling = _True;
                AdditionalOptions += option;
                break;
            }
            found = false; break;
        }
        GeneratePreprocessedFile = preprocessYes;
        break;
    case 'F':
        if(second <= '9' && second >= '0') {
            AdditionalOptions += option;
            break;
        } else {
            switch (second) {
            case 'A':
                if(third == 'c') {
                    AssemblerOutput = asmListingAsmMachine;
                    if(fourth == 's')
                        AssemblerOutput = asmListingAsmMachineSrc;
                } else if(third == 's') {
                    AssemblerOutput = asmListingAsmSrc;
                } else {
                    AssemblerOutput = asmListingAssemblyOnly;
                }
                break;
            case 'a':
                AssemblerListingLocation = option+3;
                break;
            case 'I':
                ForcedIncludeFiles += option+3;
                break;
            case 'R':
                BrowseInformation = brAllInfo;
                BrowseInformationFile = option+3;
                break;
            case 'r':
                BrowseInformation = brNoLocalSymbols;
                BrowseInformationFile = option+3;
                break;
            case 'U':
                ForcedUsingFiles += option+3;
                break;
            case 'd':
                ProgramDataBaseFileName = option+3;
                break;
            case 'e':
                OutputFile = option+3;
                break;
            case 'm':
                AdditionalOptions += option;
                break;
            case 'o':
                ObjectFile = option+3;
                break;
            case 'p':
                PrecompiledHeaderFile = option+3;
                break;
            case 'x':
                ExpandAttributedSource = _True;
                break;
            default:
                found = false; break;
            }
        }
        break;
    case 'G':
        switch (second) {
        case '3':
        case '4':
            qWarning("Option '/G3' and '/G4' were phased out in Visual C++ 5.0");
            found = false; break;
        case '5':
            OptimizeForProcessor = procOptimizePentium;
            break;
        case '6':
        case 'B':
            OptimizeForProcessor = procOptimizePentiumProAndAbove;
            break;
        case 'A':
            OptimizeForWindowsApplication = _True;
            break;
        case 'F':
            StringPooling = _True;
            break;
        case 'H':
            AdditionalOptions += option;
            break;
        case 'L':
            WholeProgramOptimization = _True;
            if(third == '-')
                WholeProgramOptimization = _False;
            break;
        case 'R':
            RuntimeTypeInfo = _True;
            if(third == '-')
                RuntimeTypeInfo = _False;
            break;
        case 'S':
            BufferSecurityCheck = _True;
            break;
        case 'T':
            EnableFiberSafeOptimizations = _True;
            break;
        case 'X':
            // ExceptionHandling == true will override with
            // an /EHsc option, which is correct with /GX
            ExceptionHandling = _True; // Fall-through
        case 'Z':
        case 'e':
        case 'h':
            AdditionalOptions += option;
            break;
        case 'd':
            CallingConvention = callConventionCDecl;
            break;
        case 'f':
            StringPooling = _True;
            AdditionalOptions += option;
            break;
        case 'm':
            MinimalRebuild = _True;
            if(third == '-')
                MinimalRebuild = _False;
            break;
        case 'r':
            CallingConvention = callConventionFastCall;
            break;
        case 's':
            // Warning: following [num] is not used,
            // were should we put it?
            BufferSecurityCheck = _True;
            break;
        case 'y':
            EnableFunctionLevelLinking = _True;
            break;
        case 'z':
            CallingConvention = callConventionStdCall;
            break;
        default:
            found = false; break;
        }
        break;
    case 'H':
        AdditionalOptions += option;
        break;
    case 'I':
        AdditionalIncludeDirectories += option+2;
        break;
    case 'L':
        if(second == 'D') {
            AdditionalOptions += option;
            break;
        }
        found = false; break;
    case 'M':
        if(second == 'D') {
            RuntimeLibrary = rtMultiThreadedDLL;
            if(third == 'd')
                RuntimeLibrary = rtMultiThreadedDebugDLL;
            break;
        } else if(second == 'L') {
            RuntimeLibrary = rtSingleThreaded;
            if(third == 'd')
                RuntimeLibrary = rtSingleThreadedDebug;
            break;
        } else if(second == 'T') {
            RuntimeLibrary = rtMultiThreaded;
            if(third == 'd')
                RuntimeLibrary = rtMultiThreadedDebug;
            break;
        }
        found = false; break;
    case 'O':
        switch (second) {
        case '1':
            Optimization = optimizeMinSpace;
            break;
        case '2':
            Optimization = optimizeMaxSpeed;
            break;
        case 'a':
            AdditionalOptions += option;
            break;
        case 'b':
            if(third == '0')
                InlineFunctionExpansion = expandDisable;
            else if(third == '1')
                InlineFunctionExpansion = expandOnlyInline;
            else if(third == '2')
                InlineFunctionExpansion = expandAnySuitable;
            else
                found = false;
            break;
        case 'd':
            Optimization = optimizeDisabled;
            break;
        case 'g':
            GlobalOptimizations = _True;
            break;
        case 'i':
            EnableIntrinsicFunctions = _True;
            break;
        case 'p':
            ImproveFloatingPointConsistency = _True;
            if(third == '-')
                ImproveFloatingPointConsistency = _False;
            break;
        case 's':
            FavorSizeOrSpeed = favorSize;
            break;
        case 't':
            FavorSizeOrSpeed = favorSpeed;
            break;
        case 'w':
            AdditionalOptions += option;
            break;
        case 'x':
            Optimization = optimizeFull;
            break;
        case 'y':
            OmitFramePointers = _True;
            if(third == '-')
                OmitFramePointers = _False;
            break;
        default:
            found = false; break;
        }
        break;
    case 'P':
        GeneratePreprocessedFile = preprocessYes;
        break;
    case 'Q':
        if(second == 'I') {
            AdditionalOptions += option;
            break;
        }
        found = false; break;
    case 'R':
        if(second == 'T' && third == 'C') {
            if(fourth == '1')
                BasicRuntimeChecks = runtimeBasicCheckAll;
            else if(fourth == 'c')
                SmallerTypeCheck = _True;
            else if(fourth == 's')
                BasicRuntimeChecks = runtimeCheckStackFrame;
            else if(fourth == 'u')
                BasicRuntimeChecks = runtimeCheckUninitVariables;
            else
                found = false; break;
        }
        break;
    case 'T':
        if(second == 'C') {
            CompileAs = compileAsC;
        } else if(second == 'P') {
            CompileAs = compileAsCPlusPlus;
        } else {
            qWarning("Generator: Options '/Tp<filename>' and '/Tc<filename>' are not supported by qmake");
            found = false; break;
        }
        break;
    case 'U':
        UndefinePreprocessorDefinitions += option+2;
        break;
    case 'V':
        AdditionalOptions += option;
        break;
    case 'W':
        switch (second) {
        case 'a':
        case '4':
            WarningLevel = warningLevel_4;
            break;
        case '3':
            WarningLevel = warningLevel_3;
            break;
        case '2':
            WarningLevel = warningLevel_2;
            break;
        case '1':
            WarningLevel = warningLevel_1;
            break;
        case '0':
            WarningLevel = warningLevel_0;
            break;
        case 'L':
            AdditionalOptions += option;
            break;
        case 'X':
            WarnAsError = _True;
            break;
        case 'p':
            if(third == '6' && fourth == '4') {
                Detect64BitPortabilityProblems = _True;
                break;
            }
            // Fallthrough
        default:
            found = false; break;
        }
        break;
    case 'X':
        IgnoreStandardIncludePath = _True;
        break;
    case 'Y':
        switch (second) {
        case '\0':
        case '-':
            AdditionalOptions += option;
            break;
        case 'X':
            UsePrecompiledHeader = pchGenerateAuto;
            PrecompiledHeaderFile = option+3;
            break;
        case 'c':
            UsePrecompiledHeader = pchCreateUsingSpecific;
            PrecompiledHeaderFile = option+3;
            break;
        case 'd':
        case 'l':
            AdditionalOptions =+ option;
            break;
        case 'u':
            UsePrecompiledHeader = pchUseUsingSpecific;
            PrecompiledHeaderFile = option+3;
            break;
        default:
            found = false; break;
        }
        break;
    case 'Z':
        switch (second) {
        case '7':
            DebugInformationFormat = debugOldStyleInfo;
            break;
        case 'I':
            DebugInformationFormat = debugEditAndContinue;
            break;
        case 'd':
            DebugInformationFormat = debugLineInfoOnly;
            break;
        case 'i':
            DebugInformationFormat = debugEnabled;
            break;
        case 'l':
            DebugInformationFormat = debugEditAndContinue;
            break;
        case 'a':
            DisableLanguageExtensions = _True;
            break;
        case 'e':
            DisableLanguageExtensions = _False;
            break;
        case 'c':
            if(third == ':') {
                if(fourth == 'f')
                    ForceConformanceInForLoopScope = _True;
                else if(fourth == 'w')
                    TreatWChar_tAsBuiltInType = _True;
                else
                    found = false;
            } else {
                found = false; break;
            }
            break;
        case 'g':
        case 'm':
        case 's':
            AdditionalOptions += option;
            break;
        case 'p':
            switch (third)
            {
            case '\0':
            case '1':
                StructMemberAlignment = alignSingleByte;
                if(fourth == '6')
                    StructMemberAlignment = alignSixteenBytes;
                break;
            case '2':
                StructMemberAlignment = alignTwoBytes;
                break;
            case '4':
                StructMemberAlignment = alignFourBytes;
                break;
            case '8':
                StructMemberAlignment = alignEightBytes;
                break;
            default:
                found = false; break;
            }
            break;
        default:
            found = false; break;
        }
        break;
    case 'c':
        if(second == '\0') {
            CompileOnly = _True;
        } else if(second == 'l') {
            if(*(option+5) == 'n') {
                CompileAsManaged = managedAssembly;
                TurnOffAssemblyGeneration = _True;
            } else {
                CompileAsManaged = managedAssembly;
            }
        } else {
            found = false; break;
        }
        break;
    case 'd':
        if(second != 'r') {
            found = false; break;
        }
        CompileAsManaged = managedAssembly;
        break;
    case 'n':
        if(second == 'o' && third == 'B' && fourth == 'o') {
            AdditionalOptions += "/noBool";
            break;
        }
        if(second == 'o' && third == 'l' && fourth == 'o') {
            SuppressStartupBanner = _True;
            break;
        }
        found = false; break;
    case 's':
        if(second == 'h' && third == 'o' && fourth == 'w') {
            ShowIncludes = _True;
            break;
        }
        found = false; break;
    case 'u':
        UndefineAllPreprocessorDefinitions = _True;
        break;
    case 'v':
        if(second == 'd' || second == 'm') {
            AdditionalOptions += option;
            break;
        }
        found = false; break;
    case 'w':
        switch (second) {
        case '\0':
            WarningLevel = warningLevel_0;
            break;
        case 'd':
            DisableSpecificWarnings += option+3;
            break;
        default:
            AdditionalOptions += option;
        }
        break;
    default:
        found = false; break;
    }
    if(!found)
        warn_msg(WarnLogic, "Could not parse Compiler option: %s", option);
    return true;
}

// VCLinkerTool -----------------------------------------------------
VCLinkerTool::VCLinkerTool()
    :        EnableCOMDATFolding(optFoldingDefault),
        GenerateDebugInformation(unset),
        GenerateMapFile(unset),
        HeapCommitSize(-1),
        HeapReserveSize(-1),
        IgnoreAllDefaultLibraries(unset),
        IgnoreEmbeddedIDL(unset),
        IgnoreImportLibrary(_True),
        LargeAddressAware(addrAwareDefault),
        LinkDLL(unset),
        LinkIncremental(linkIncrementalDefault),
        LinkTimeCodeGeneration(unset),
        MapExports(unset),
        MapLines(unset),
        OptimizeForWindows98(optWin98Default),
        OptimizeReferences(optReferencesDefault),
        RegisterOutput(unset),
        ResourceOnlyDLL(unset),
        SetChecksum(unset),
        ShowProgress(linkProgressNotSet),
        StackCommitSize(-1),
        StackReserveSize(-1),
        SubSystem(subSystemNotSet),
        SupportUnloadOfDelayLoadedDLL(unset),
        SuppressStartupBanner(unset),
        SwapRunFromCD(unset),
        SwapRunFromNet(unset),
        TargetMachine(machineNotSet),
        TerminalServerAware(termSvrAwareDefault),
        TurnOffAssemblyGeneration(unset),
        TypeLibraryResourceID(0)
{
}

XmlOutput &operator<<(XmlOutput &xml, const VCLinkerTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, _VCLinkerTool)
            << attrX(_AdditionalDependencies, tool.AdditionalDependencies, " ")
            << attrX(_AdditionalLibraryDirectories, tool.AdditionalLibraryDirectories)
            << attrX(_AdditionalOptions, tool.AdditionalOptions, " ")
            << attrX(_AddModuleNamesToAssembly, tool.AddModuleNamesToAssembly)
            << attrS(_BaseAddress, tool.BaseAddress)
            << attrX(_DelayLoadDLLs, tool.DelayLoadDLLs)
            << attrE(_EnableCOMDATFolding, tool.EnableCOMDATFolding, /*ifNot*/ optFoldingDefault)
            << attrS(_EntryPointSymbol, tool.EntryPointSymbol)
            << attrX(_ForceSymbolReferences, tool.ForceSymbolReferences)
            << attrS(_FunctionOrder, tool.FunctionOrder)
            << attrT(_GenerateDebugInformation, tool.GenerateDebugInformation)
            << attrT(_GenerateMapFile, tool.GenerateMapFile)
            << attrL(_HeapCommitSize, tool.HeapCommitSize, /*ifNot*/ -1)
            << attrL(_HeapReserveSize, tool.HeapReserveSize, /*ifNot*/ -1)
            << attrT(_IgnoreAllDefaultLibraries, tool.IgnoreAllDefaultLibraries)
            << attrX(_IgnoreDefaultLibraryNames, tool.IgnoreDefaultLibraryNames)
            << attrT(_IgnoreEmbeddedIDL, tool.IgnoreEmbeddedIDL)
            << attrT(_IgnoreImportLibrary, tool.IgnoreImportLibrary)
            << attrS(_ImportLibrary, tool.ImportLibrary)
            << attrE(_LargeAddressAware, tool.LargeAddressAware, /*ifNot*/ addrAwareDefault)
            << attrT(_LinkDLL, tool.LinkDLL)
            << attrE(_LinkIncremental, tool.LinkIncremental, /*ifNot*/ linkIncrementalDefault)
            << attrT(_LinkTimeCodeGeneration, tool.LinkTimeCodeGeneration)
            << attrS(_LinkToManagedResourceFile, tool.LinkToManagedResourceFile)
            << attrT(_MapExports, tool.MapExports)
            << attrS(_MapFileName, tool.MapFileName)
            << attrT(_MapLines, tool.MapLines)
            << attrS(_MergedIDLBaseFileName, tool.MergedIDLBaseFileName)
            << attrS(_MergeSections, tool.MergeSections)
            << attrS(_MidlCommandFile, tool.MidlCommandFile)
            << attrS(_ModuleDefinitionFile, tool.ModuleDefinitionFile)
            << attrE(_OptimizeForWindows98, tool.OptimizeForWindows98, /*ifNot*/ optWin98Default)
            << attrE(_OptimizeReferences, tool.OptimizeReferences, /*ifNot*/ optReferencesDefault)
            << attrS(_OutputFile, tool.OutputFile)
            << attr(_ProgramDatabaseFile, tool.ProgramDatabaseFile)
            << attrT(_RegisterOutput, tool.RegisterOutput)
            << attrT(_ResourceOnlyDLL, tool.ResourceOnlyDLL)
            << attrT(_SetChecksum, tool.SetChecksum)
            << attrE(_ShowProgress, tool.ShowProgress, /*ifNot*/ linkProgressNotSet)
            << attrL(_StackCommitSize, tool.StackCommitSize, /*ifNot*/ -1)
            << attrL(_StackReserveSize, tool.StackReserveSize, /*ifNot*/ -1)
            << attrS(_StripPrivateSymbols, tool.StripPrivateSymbols)
            << attrE(_SubSystem, tool.SubSystem)
            << attrT(_SupportUnloadOfDelayLoadedDLL, tool.SupportUnloadOfDelayLoadedDLL)
            << attrT(_SuppressStartupBanner, tool.SuppressStartupBanner)
            << attrT(_SwapRunFromCD, tool.SwapRunFromCD)
            << attrT(_SwapRunFromNet, tool.SwapRunFromNet)
            << attrE(_TargetMachine, tool.TargetMachine, /*ifNot*/ machineNotSet)
            << attrE(_TerminalServerAware, tool.TerminalServerAware, /*ifNot*/ termSvrAwareDefault)
            << attrT(_TurnOffAssemblyGeneration, tool.TurnOffAssemblyGeneration)
            << attrS(_TypeLibraryFile, tool.TypeLibraryFile)
            << attrL(_TypeLibraryResourceID, tool.TypeLibraryResourceID, /*ifNot*/ rcUseDefault)
            << attrS(_Version, tool.Version)
        << closetag(_Tool);
}

// Hashing routine to do fast option lookups ----
// Slightly rewritten to stop on ':' ',' and '\0'
// Original routine in qtranslator.cpp ----------
static uint elfHash(const char* name)
{
    const uchar *k;
    uint h = 0;
    uint g;

    if(name) {
        k = (const uchar *) name;
        while((*k) &&
                (*k)!= ':' &&
                (*k)!=',' &&
                (*k)!=' ') {
            h = (h << 4) + *k++;
            if((g = (h & 0xf0000000)) != 0)
                h ^= g >> 24;
            h &= ~g;
        }
    }
    if(!h)
        h = 1;
    return h;
}

//#define USE_DISPLAY_HASH
#ifdef USE_DISPLAY_HASH
static void displayHash(const char* str)
{
    printf("case 0x%07x: // %s\n    break;\n", elfHash(str), str);
}
#endif

bool VCLinkerTool::parseOption(const char* option)
{
#ifdef USE_DISPLAY_HASH
    // Main options
    displayHash("/ALIGN"); displayHash("/ALLOWBIND"); displayHash("/ASSEMBLYMODULE");
    displayHash("/ASSEMBLYRESOURCE"); displayHash("/BASE"); displayHash("/DEBUG");
    displayHash("/DEF"); displayHash("/DEFAULTLIB"); displayHash("/DELAY");
    displayHash("/DELAYLOAD"); displayHash("/DLL"); displayHash("/DRIVER");
    displayHash("/ENTRY"); displayHash("/EXETYPE"); displayHash("/EXPORT");
    displayHash("/FIXED"); displayHash("/FORCE"); displayHash("/HEAP");
    displayHash("/IDLOUT"); displayHash("/IGNOREIDL"); displayHash("/IMPLIB");
    displayHash("/INCLUDE"); displayHash("/INCREMENTAL"); displayHash("/LARGEADDRESSAWARE");
    displayHash("/LIBPATH"); displayHash("/LTCG"); displayHash("/MACHINE");
    displayHash("/MAP"); displayHash("/MAPINFO"); displayHash("/MERGE");
    displayHash("/MIDL"); displayHash("/NOASSEMBLY"); displayHash("/NODEFAULTLIB");
    displayHash("/NOENTRY"); displayHash("/NOLOGO"); displayHash("/OPT");
    displayHash("/ORDER"); displayHash("/OUT"); displayHash("/PDB");
    displayHash("/PDBSTRIPPED"); displayHash("/RELEASE"); displayHash("/SECTION");
    displayHash("/STACK"); displayHash("/STUB"); displayHash("/SUBSYSTEM");
    displayHash("/SWAPRUN"); displayHash("/TLBID"); displayHash("/TLBOUT");
    displayHash("/TSAWARE"); displayHash("/VERBOSE"); displayHash("/VERSION");
    displayHash("/VXD"); displayHash("/WS ");
#endif
#ifdef USE_DISPLAY_HASH
    // Sub options
    displayHash("UNLOAD"); displayHash("NOBIND"); displayHash("no"); displayHash("NOSTATUS"); displayHash("STATUS");
    displayHash("AM33"); displayHash("ARM"); displayHash("CEE"); displayHash("IA64"); displayHash("X86"); displayHash("M32R");
    displayHash("MIPS"); displayHash("MIPS16"); displayHash("MIPSFPU"); displayHash("MIPSFPU16"); displayHash("MIPSR41XX"); displayHash("PPC");
    displayHash("SH3"); displayHash("SH4"); displayHash("SH5"); displayHash("THUMB"); displayHash("TRICORE"); displayHash("EXPORTS");
    displayHash("LINES"); displayHash("REF"); displayHash("NOREF"); displayHash("ICF"); displayHash("WIN98"); displayHash("NOWIN98");
    displayHash("CONSOLE"); displayHash("EFI_APPLICATION"); displayHash("EFI_BOOT_SERVICE_DRIVER"); displayHash("EFI_ROM"); displayHash("EFI_RUNTIME_DRIVER"); displayHash("NATIVE");
    displayHash("POSIX"); displayHash("WINDOWS"); displayHash("WINDOWSCE"); displayHash("NET"); displayHash("CD"); displayHash("NO");
#endif
    bool found = true;
    switch (elfHash(option)) {
    case 0x3360dbe: // /ALIGN[:number]
    case 0x1485c34: // /ALLOWBIND[:NO]
    case 0x6b21972: // /DEFAULTLIB:library
    case 0x396ea92: // /DRIVER[:UPONLY | :WDM]
    case 0xaca9d75: // /EXETYPE[:DYNAMIC | :DEV386]
    case 0x3ad5444: // /EXPORT:entryname[,@ordinal[,NONAME]][,DATA]
    case 0x33aec94: // /FIXED[:NO]
    case 0x33b4675: // /FORCE:[MULTIPLE|UNRESOLVED]
    case 0x7988f7e: // /SECTION:name,[E][R][W][S][D][K][L][P][X][,ALIGN=#]
    case 0x0348992: // /STUB:filename
    case 0x0034bc4: // /VXD
    case 0x0034c50: // /WS
        AdditionalOptions += option;
        break;
    case 0x679c075: // /ASSEMBLYMODULE:filename
        AddModuleNamesToAssembly += option+15;
        break;
    case 0x062d065: // /ASSEMBLYRESOURCE:filename
        LinkToManagedResourceFile = option+18;
        break;
    case 0x0336675: // /BASE:{address | @filename,key}
        // Do we need to do a manual lookup when '@filename,key'?
        // Seems BaseAddress only can contain the location...
        // We don't use it in Qt, so keep it simple for now
        BaseAddress = option+6;
        break;
    case 0x3389797: // /DEBUG
        GenerateDebugInformation = _True;
        break;
    case 0x0033896: // /DEF:filename
        ModuleDefinitionFile = option+5;
        break;
    case 0x338a069: // /DELAY:{UNLOAD | NOBIND}
        // MS documentation does not specify what to do with
        // this option, so we'll put it in AdditionalOptions
        AdditionalOptions += option;
        break;
    case 0x06f4bf4: // /DELAYLOAD:dllname
        DelayLoadDLLs += option+11;
        break;
    case 0x003390c: // /DLL
        // This option is not used for vcproj files
        break;
    case 0x33a3979: // /ENTRY:function
        EntryPointSymbol = option+7;
        break;
    case 0x033c960: // /HEAP:reserve[,commit]
        {
            QStringList both = QString(option+6).split(",");
            HeapReserveSize = both[0].toLong();
            if(both.count() == 2)
                HeapCommitSize = both[1].toLong();
        }
        break;
    case 0x3d91494: // /IDLOUT:[path\]filename
        MergedIDLBaseFileName = option+8;
        break;
    case 0x345a04c: // /IGNOREIDL
        IgnoreEmbeddedIDL = _True;
        break;
    case 0x3e250e2: // /IMPLIB:filename
        ImportLibrary = option+8;
        break;
    case 0xe281ab5: // /INCLUDE:symbol
        ForceSymbolReferences += option+9;
        break;
    case 0xb28103c: // /INCREMENTAL[:no]
        if(*(option+12) == ':' &&
             *(option+13) == 'n')
            LinkIncremental = linkIncrementalNo;
        else
            LinkIncremental = linkIncrementalYes;
        break;
    case 0x26e4675: // /LARGEADDRESSAWARE[:no]
        if(*(option+18) == ':' &&
             *(option+19) == 'n')
            LargeAddressAware = addrAwareNoLarge;
        else
            LargeAddressAware = addrAwareLarge;
        break;
    case 0x0d745c8: // /LIBPATH:dir
        AdditionalLibraryDirectories += option+9;
        break;
    case 0x0341877: // /LTCG[:NOSTATUS|:STATUS]
        config->WholeProgramOptimization = _True;
        LinkTimeCodeGeneration = _True;
        if(*(option+5) == ':' &&
             *(option+6) == 'S')
             ShowProgress = linkProgressAll;
        break;
    case 0x157cf65: // /MACHINE:{AM33|ARM|CEE|IA64|X86|M32R|MIPS|MIPS16|MIPSFPU|MIPSFPU16|MIPSR41XX|PPC|SH3|SH4|SH5|THUMB|TRICORE}
        switch (elfHash(option+9)) {
            // Very limited documentation on all options but X86,
            // so we put the others in AdditionalOptions...
        case 0x0046063: // AM33
        case 0x000466d: // ARM
        case 0x0004795: // CEE
        case 0x004d494: // IA64
        case 0x0050672: // M32R
        case 0x0051e53: // MIPS
        case 0x51e5646: // MIPS16
        case 0x1e57b05: // MIPSFPU
        case 0x57b09a6: // MIPSFPU16
        case 0x5852738: // MIPSR41XX
        case 0x0005543: // PPC
        case 0x00057b3: // SH3
        case 0x00057b4: // SH4
        case 0x00057b5: // SH5
        case 0x058da12: // THUMB
        case 0x96d8435: // TRICORE
            AdditionalOptions += option;
            break;
        case 0x0005bb6: // X86
            TargetMachine = machineX86;
            break;
        default:
            found = false;
        }
        break;
    case 0x0034160: // /MAP[:filename]
        GenerateMapFile = _True;
        MapFileName = option+5;
        break;
    case 0x164e1ef: // /MAPINFO:{EXPORTS|LINES}
        if(*(option+9) == 'E')
            MapExports = _True;
        else if(*(option+9) == 'L')
            MapLines = _True;
        break;
    case 0x341a6b5: // /MERGE:from=to
        MergeSections = option+7;
        break;
    case 0x0341d8c: // /MIDL:@file
        MidlCommandFile = option+7;
        break;
    case 0x84e2679: // /NOASSEMBLY
        TurnOffAssemblyGeneration = _True;
        break;
    case 0x2b21942: // /NODEFAULTLIB[:library]
        if(*(option+13) == '\0')
            IgnoreAllDefaultLibraries = _True;
        else
            IgnoreDefaultLibraryNames += option+14;
        break;
    case 0x33a3a39: // /NOENTRY
        ResourceOnlyDLL = _True;
        break;
    case 0x434138f: // /NOLOGO
        SuppressStartupBanner = _True;
        break;
    case 0x0034454: // /OPT:{REF | NOREF | ICF[=iterations] | NOICF | WIN98 | NOWIN98}
        {
            char third = *(option+7);
            switch (third) {
            case 'F': // REF
                if(*(option+5) == 'R') {
                    OptimizeReferences = optReferences;
                } else { // ICF[=iterations]
                    EnableCOMDATFolding = optFolding;
                    // [=iterations] case is not documented
                }
                break;
            case 'R': // NOREF
                OptimizeReferences = optNoReferences;
                break;
            case 'I': // NOICF
                EnableCOMDATFolding = optNoFolding;
                break;
            case 'N': // WIN98
                OptimizeForWindows98 = optWin98Yes;
                break;
            case 'W': // NOWIN98
                OptimizeForWindows98 = optWin98No;
                break;
            default:
                found = false;
            }
        }
        break;
    case 0x34468a2: // /ORDER:@filename
        FunctionOrder = option+8;
        break;
    case 0x00344a4: // /OUT:filename
        OutputFile = option+5;
        break;
    case 0x0034482: // /PDB:filename
        ProgramDatabaseFile = option+5;
        break;
    case 0xa2ad314: // /PDBSTRIPPED:pdb_file_name
        StripPrivateSymbols = option+13;
        break;
    case 0x6a09535: // /RELEASE
        SetChecksum = _True;
        break;
    case 0x348857b: // /STACK:reserve[,commit]
        {
            QStringList both = QString(option+7).split(",");
            StackReserveSize = both[0].toLong();
            if(both.count() == 2)
                StackCommitSize = both[1].toLong();
        }
        break;
    case 0x78dc00d: // /SUBSYSTEM:{CONSOLE|EFI_APPLICATION|EFI_BOOT_SERVICE_DRIVER|EFI_ROM|EFI_RUNTIME_DRIVER|NATIVE|POSIX|WINDOWS|WINDOWSCE}[,major[.minor]]
        {
            // Split up in subsystem, and version number
            QStringList both = QString(option+11).split(",");
            switch (elfHash(both[0].latin1())) {
            case 0x8438445: // CONSOLE
                SubSystem = subSystemConsole;
                break;
            case 0xbe29493: // WINDOWS
                SubSystem = subSystemWindows;
                break;
            // The following are undocumented, so add them to AdditionalOptions
            case 0x240949e: // EFI_APPLICATION
            case 0xe617652: // EFI_BOOT_SERVICE_DRIVER
            case 0x9af477d: // EFI_ROM
            case 0xd34df42: // EFI_RUNTIME_DRIVER
            case 0x5268ea5: // NATIVE
            case 0x05547e8: // POSIX
            case 0x2949c95: // WINDOWSCE
                AdditionalOptions += option;
                break;
            default:
                found = false;
            }
        }
        break;
    case 0x8b654de: // /SWAPRUN:{NET | CD}
        if(*(option+9) == 'N')
            SwapRunFromNet = _True;
        else if(*(option+9) == 'C')
            SwapRunFromCD = _True;
        else
            found = false;
        break;
    case 0x34906d4: // /TLBID:id
        TypeLibraryResourceID = QString(option+7).toLong();
        break;
    case 0x4907494: // /TLBOUT:[path\]filename
        TypeLibraryFile = option+8;
        break;
    case 0x976b525: // /TSAWARE[:NO]
        if(*(option+8) == ':')
            TerminalServerAware = termSvrAwareNo;
        else
            TerminalServerAware = termSvrAwareYes;
        break;
    case 0xaa67735: // /VERBOSE[:lib]
        if(*(option+9) == ':') {
            ShowProgress = linkProgressLibs;
            AdditionalOptions += option;
        } else {
            ShowProgress = linkProgressAll;
        }
        break;
    case 0xaa77f7e: // /VERSION:major[.minor]
        Version = option+9;
        break;
    default:
        found = false;
    }
    if(!found)
        warn_msg(WarnLogic, "Could not parse Linker options: %s", option);
    return found;
}

// VCMIDLTool -------------------------------------------------------
VCMIDLTool::VCMIDLTool()
    :        DefaultCharType(midlCharUnsigned),
        EnableErrorChecks(midlDisableAll),
        ErrorCheckAllocations(unset),
        ErrorCheckBounds(unset),
        ErrorCheckEnumRange(unset),
        ErrorCheckRefPointers(unset),
        ErrorCheckStubData(unset),
        GenerateStublessProxies(unset),
        GenerateTypeLibrary(unset),
        IgnoreStandardIncludePath(unset),
        MkTypLibCompatible(unset),
        StructMemberAlignment(midlAlignNotSet),
        SuppressStartupBanner(unset),
        TargetEnvironment(midlTargetNotSet),
        ValidateParameters(unset),
        WarnAsError(unset),
        WarningLevel(midlWarningLevel_0)
{
}

XmlOutput &operator<<(XmlOutput &xml, const VCMIDLTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, _VCMIDLTool)
            << attrX(_AdditionalIncludeDirectories, tool.AdditionalIncludeDirectories)
            << attrX(_AdditionalOptions, tool.AdditionalOptions, " ")
            << attrX(_CPreprocessOptions, tool.CPreprocessOptions)
            << attrE(_DefaultCharType, tool.DefaultCharType)
            << attrS(_DLLDataFileName, tool.DLLDataFileName)
            << attrE(_EnableErrorChecks, tool.EnableErrorChecks)
            << attrT(_ErrorCheckAllocations, tool.ErrorCheckAllocations)
            << attrT(_ErrorCheckBounds, tool.ErrorCheckBounds)
            << attrT(_ErrorCheckEnumRange, tool.ErrorCheckEnumRange)
            << attrT(_ErrorCheckRefPointers, tool.ErrorCheckRefPointers)
            << attrT(_ErrorCheckStubData, tool.ErrorCheckStubData)
            << attrX(_FullIncludePath, tool.FullIncludePath)
            << attrT(_GenerateStublessProxies, tool.GenerateStublessProxies)
            << attrT(_GenerateTypeLibrary, tool.GenerateTypeLibrary)
            << attrS(_HeaderFileName, tool.HeaderFileName)
            << attrT(_IgnoreStandardIncludePath, tool.IgnoreStandardIncludePath)
            << attrS(_InterfaceIdentifierFileName, tool.InterfaceIdentifierFileName)
            << attrT(_MkTypLibCompatible, tool.MkTypLibCompatible)
            << attrS(_OutputDirectory, tool.OutputDirectory)
            << attrX(_PreprocessorDefinitions, tool.PreprocessorDefinitions)
            << attrS(_ProxyFileName, tool.ProxyFileName)
            << attrS(_RedirectOutputAndErrors, tool.RedirectOutputAndErrors)
            << attrE(_StructMemberAlignment, tool.StructMemberAlignment, /*ifNot*/ midlAlignNotSet)
            << attrT(_SuppressStartupBanner, tool.SuppressStartupBanner)
            << attrE(_TargetEnvironment, tool.TargetEnvironment, /*ifNot*/ midlTargetNotSet)
            << attrS(_TypeLibraryName, tool.TypeLibraryName)
            << attrX(_UndefinePreprocessorDefinitions, tool.UndefinePreprocessorDefinitions)
            << attrT(_ValidateParameters, tool.ValidateParameters)
            << attrT(_WarnAsError, tool.WarnAsError)
            << attrE(_WarningLevel, tool.WarningLevel)
        << closetag(_Tool);
}

bool VCMIDLTool::parseOption(const char* option)
{
#ifdef USE_DISPLAY_HASH
    displayHash("/D name[=def]"); displayHash("/I directory-list"); displayHash("/Oi");
    displayHash("/Oic"); displayHash("/Oicf"); displayHash("/Oif"); displayHash("/Os");
    displayHash("/U name"); displayHash("/WX"); displayHash("/W{0|1|2|3|4}");
    displayHash("/Zp {N}"); displayHash("/Zs"); displayHash("/acf filename");
    displayHash("/align {N}"); displayHash("/app_config"); displayHash("/c_ext");
    displayHash("/char ascii7"); displayHash("/char signed"); displayHash("/char unsigned");
    displayHash("/client none"); displayHash("/client stub"); displayHash("/confirm");
    displayHash("/cpp_cmd cmd_line"); displayHash("/cpp_opt options");
    displayHash("/cstub filename"); displayHash("/dlldata filename"); displayHash("/env win32");
    displayHash("/env win64"); displayHash("/error all"); displayHash("/error allocation");
    displayHash("/error bounds_check"); displayHash("/error enum"); displayHash("/error none");
    displayHash("/error ref"); displayHash("/error stub_data"); displayHash("/h filename");
    displayHash("/header filename"); displayHash("/iid filename"); displayHash("/lcid");
    displayHash("/mktyplib203"); displayHash("/ms_ext"); displayHash("/ms_union");
    displayHash("/msc_ver <nnnn>"); displayHash("/newtlb"); displayHash("/no_cpp");
    displayHash("/no_def_idir"); displayHash("/no_default_epv"); displayHash("/no_format_opt");
    displayHash("/no_warn"); displayHash("/nocpp"); displayHash("/nologo"); displayHash("/notlb");
    displayHash("/o filename"); displayHash("/oldnames"); displayHash("/oldtlb");
    displayHash("/osf"); displayHash("/out directory"); displayHash("/pack {N}");
    displayHash("/prefix all"); displayHash("/prefix client"); displayHash("/prefix server");
    displayHash("/prefix switch"); displayHash("/protocol all"); displayHash("/protocol dce");
    displayHash("/protocol ndr64"); displayHash("/proxy filename"); displayHash("/robust");
    displayHash("/rpcss"); displayHash("/savePP"); displayHash("/server none");
    displayHash("/server stub"); displayHash("/sstub filename"); displayHash("/syntax_check");
    displayHash("/target {system}"); displayHash("/tlb filename"); displayHash("/use_epv");
    displayHash("/win32"); displayHash("/win64");
#endif
    bool found = true;
    int offset = 0;
    switch(elfHash(option)) {
    case 0x0000334: // /D name[=def]
        PreprocessorDefinitions += option+3;
        break;
    case 0x0000339: // /I directory-list
        AdditionalIncludeDirectories += option+3;
        break;
    case 0x0345f96: // /Oicf
    case 0x00345f6: // /Oif
        GenerateStublessProxies = _True;
        break;
    case 0x0000345: // /U name
        UndefinePreprocessorDefinitions += option+3;
        break;
    case 0x00034c8: // /WX
        WarnAsError = _True;
        break;
    case 0x3582fde: // /align {N}
        offset = 3;  // Fallthrough
    case 0x0003510: // /Zp {N}
        switch (*(option+offset+4)) {
        case '1':
            StructMemberAlignment = (*(option+offset+5) == '\0') ? midlAlignSingleByte : midlAlignSixteenBytes;
            break;
        case '2':
            StructMemberAlignment = midlAlignTwoBytes;
            break;
        case '4':
            StructMemberAlignment = midlAlignFourBytes;
            break;
        case '8':
            StructMemberAlignment = midlAlignEightBytes;
            break;
        default:
            found = false;
        }
        break;
    case 0x0359e82: // /char {ascii7|signed|unsigned}
        switch(*(option+6)) {
        case 'a':
            DefaultCharType = midlCharAscii7;
            break;
        case 's':
            DefaultCharType = midlCharSigned;
            break;
        case 'u':
            DefaultCharType = midlCharUnsigned;
            break;
        default:
            found = false;
        }
        break;
    case 0xa766524: // /cpp_opt options
        CPreprocessOptions += option+9;
        break;
    case 0xb32abf1: // /dlldata filename
        DLLDataFileName = option + 9;
        break;
    case 0x0035c56: // /env {win32|win64}
        TargetEnvironment = (*(option+8) == '6') ? midlTargetWin64 : midlTargetWin32;
        break;
    case 0x35c9962: // /error {all|allocation|bounds_check|enum|none|ref|stub_data}
        EnableErrorChecks = midlEnableCustom;
        switch (*(option+7)) {
        case 'a':
            if(*(option+10) == '\0')
                EnableErrorChecks = midlEnableAll;
            else
                ErrorCheckAllocations = _True;
            break;
        case 'b':
            ErrorCheckBounds = _True;
            break;
        case 'e':
            ErrorCheckEnumRange = _True;
            break;
        case 'n':
            EnableErrorChecks = midlDisableAll;
            break;
        case 'r':
            ErrorCheckRefPointers = _True;
            break;
        case 's':
            ErrorCheckStubData = _True;
            break;
        default:
            found = false;
        }
        break;
    case 0x5eb7af2: // /header filename
        offset = 5;
    case 0x0000358: // /h filename
        HeaderFileName = option + offset + 3;
        break;
    case 0x0035ff4: // /iid filename
        InterfaceIdentifierFileName = option+5;
        break;
    case 0x64b7933: // /mktyplib203
        MkTypLibCompatible = _True;
        break;
    case 0x8e0b0a2: // /no_def_idir
        IgnoreStandardIncludePath = _True;
        break;
    case 0x65635ef: // /nologo
        SuppressStartupBanner = _True;
        break;
    case 0x3656b22: // /notlb
        GenerateTypeLibrary = _True;
        break;
    case 0x000035f: // /o filename
        RedirectOutputAndErrors = option+3;
        break;
    case 0x00366c4: // /out directory
        OutputDirectory = option+5;
        break;
    case 0x36796f9: // /proxy filename
        ProxyFileName = option+7;
        break;
    case 0x6959c94: // /robust
        ValidateParameters = _True;
        break;
    case 0x6a88df4: // /target {system}
        if(*(option+11) == '6')
            TargetEnvironment = midlTargetWin64;
        else
            TargetEnvironment = midlTargetWin32;
        break;
    case 0x0036b22: // /tlb filename
        TypeLibraryName = option+5;
        break;
    case 0x36e0162: // /win32
        TargetEnvironment = midlTargetWin32;
        break;
    case 0x36e0194: // /win64
        TargetEnvironment = midlTargetWin64;
        break;
    case 0x0003459: // /Oi
    case 0x00345f3: // /Oic
    case 0x0003463: // /Os
    case 0x0003513: // /Zs
    case 0x0035796: // /acf filename
    case 0x5b1cb97: // /app_config
    case 0x3595cf4: // /c_ext
    case 0x5a2fc64: // /client {none|stub}
    case 0xa64d3dd: // /confirm
    case 0xa765b64: // /cpp_cmd cmd_line
    case 0x35aabb2: // /cstub filename
    case 0x03629f4: // /lcid
    case 0x6495cc4: // /ms_ext
    case 0x96c7a1e: // /ms_union
    case 0x4996fa2: // /msc_ver <nnnn>
    case 0x64ceb12: // /newtlb
    case 0x6555a40: // /no_cpp
    case 0xf64d6a6: // /no_default_epv
    case 0x6dd9384: // /no_format_opt
    case 0x556dbee: // /no_warn
    case 0x3655a70: // /nocpp
    case 0x2b455a3: // /oldnames
    case 0x662bb12: // /oldtlb
    case 0x0036696: // /osf
    case 0x036679b: // /pack {N}
    case 0x678bd38: // /prefix {all|client|server|switch}
    case 0x96b702c: // /protocol {all|dce|ndr64}
    case 0x3696aa3: // /rpcss
    case 0x698ca60: // /savePP
    case 0x69c9cf2: // /server {none|stub}
    case 0x36aabb2: // /sstub filename
    case 0xce9b12b: // /syntax_check
    case 0xc9b5f16: // /use_epv
        AdditionalOptions += option;
        break;
    default:
        // /W{0|1|2|3|4} case
        if(*(option+1) == 'W') {
            switch (*(option+2)) {
            case '0':
                WarningLevel = midlWarningLevel_0;
                break;
            case '1':
                WarningLevel = midlWarningLevel_1;
                break;
            case '2':
                WarningLevel = midlWarningLevel_2;
                break;
            case '3':
                WarningLevel = midlWarningLevel_3;
                break;
            case '4':
                WarningLevel = midlWarningLevel_4;
                break;
            default:
                found = false;
            }
        }
        break;
    }
    if(!found)
        warn_msg(WarnLogic, "Could not parse MIDL option: %s", option);
    return true;
}

// VCLibrarianTool --------------------------------------------------
VCLibrarianTool::VCLibrarianTool()
    :        IgnoreAllDefaultLibraries(unset),
        SuppressStartupBanner(_True)
{
}

XmlOutput &operator<<(XmlOutput &xml, const VCLibrarianTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, _VCLibrarianTool)
            << attrX(_AdditionalDependencies, tool.AdditionalDependencies)
            << attrX(_AdditionalLibraryDirectories, tool.AdditionalLibraryDirectories)
            << attrX(_AdditionalOptions, tool.AdditionalOptions, " ")
            << attrX(_ExportNamedFunctions, tool.ExportNamedFunctions)
            << attrX(_ForceSymbolReferences, tool.ForceSymbolReferences)
            << attrT(_IgnoreAllDefaultLibraries, tool.IgnoreAllDefaultLibraries)
            << attrX(_IgnoreDefaultLibraryNames, tool.IgnoreDefaultLibraryNames)
            << attrS(_ModuleDefinitionFile, tool.ModuleDefinitionFile)
            << attrS(_OutputFile, tool.OutputFile)
            << attrT(_SuppressStartupBanner, tool.SuppressStartupBanner)
        << closetag(_Tool);
}

// VCCustomBuildTool ------------------------------------------------
VCCustomBuildTool::VCCustomBuildTool()
{
    ToolName = "VCCustomBuildTool";
}

XmlOutput &operator<<(XmlOutput &xml, const VCCustomBuildTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, tool.ToolName)
            << attrX(_AdditionalDependencies, tool.AdditionalDependencies, ";")
            << attrX(_CommandLine, tool.CommandLine, "\n")
            << attrS(_Description, tool.Description)
            << attrX(_Outputs, tool.Outputs, ";")
            << attrS(_Path, tool.ToolPath)
        << closetag(_Tool);
}

// VCResourceCompilerTool -------------------------------------------
VCResourceCompilerTool::VCResourceCompilerTool()
    :   Culture(rcUseDefault),
        IgnoreStandardIncludePath(unset),
        ShowProgress(linkProgressNotSet)
{
    PreprocessorDefinitions = "NDEBUG";
}

XmlOutput &operator<<(XmlOutput &xml, const VCResourceCompilerTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, _VCResourceCompilerTool)
            << attrS(_Path, tool.ToolPath)
            << attrX(_AdditionalIncludeDirectories, tool.AdditionalIncludeDirectories)
            << attrX(_AdditionalOptions, tool.AdditionalOptions, " ")
            << attrE(_Culture, tool.Culture, /*ifNot*/ rcUseDefault)
            << attrX(_FullIncludePath, tool.FullIncludePath)
            << attrT(_IgnoreStandardIncludePath, tool.IgnoreStandardIncludePath)
            << attrX(_PreprocessorDefinitions, tool.PreprocessorDefinitions)
            << attrS(_ResourceOutputFileName, tool.ResourceOutputFileName)
            << attrE(_ShowProgress, tool.ShowProgress, /*ifNot*/ linkProgressNotSet)
        << closetag(_Tool);
}

// VCEventTool -------------------------------------------------
XmlOutput &operator<<(XmlOutput &xml, const VCEventTool &tool)
{
    return xml
        << tag(_Tool)
            << attrS(_Name, tool.ToolName)
            << attrS(_Path, tool.ToolPath)
            << attrS(_CommandLine, tool.CommandLine)
            << attrS(_Description, tool.Description)
            << attrT(_ExcludedFromBuild, tool.ExcludedFromBuild)
        << closetag(_Tool);
}

// VCPostBuildEventTool ---------------------------------------------
VCPostBuildEventTool::VCPostBuildEventTool()
{
    ToolName = "VCPostBuildEventTool";
}

// VCPreBuildEventTool ----------------------------------------------
VCPreBuildEventTool::VCPreBuildEventTool()
{
    ToolName = "VCPreBuildEventTool";
}

// VCPreLinkEventTool -----------------------------------------------
VCPreLinkEventTool::VCPreLinkEventTool()
{
    ToolName = "VCPreLinkEventTool";
}

// VCConfiguration --------------------------------------------------

VCConfiguration::VCConfiguration()
    :        ATLMinimizesCRunTimeLibraryUsage(unset),
        BuildBrowserInformation(unset),
        CharacterSet(charSetNotSet),
        ConfigurationType(typeApplication),
        RegisterOutput(unset),
        UseOfATL(useATLNotSet),
        UseOfMfc(useMfcStdWin),
        WholeProgramOptimization(unset)
{
    compiler.config = this;
    linker.config = this;
    idl.config = this;
}

XmlOutput &operator<<(XmlOutput &xml, const VCConfiguration &tool)
{
    xml << tag(_Configuration)
            << attrS(_Name, tool.Name)
            << attrS(_OutputDirectory, tool.OutputDirectory)
            << attrT(_ATLMinimizesCRunTimeLibraryUsage, tool.ATLMinimizesCRunTimeLibraryUsage)
            << attrT(_BuildBrowserInformation, tool.BuildBrowserInformation)
            << attrE(_CharacterSet, tool.CharacterSet, /*ifNot*/ charSetNotSet)
            << attrE(_ConfigurationType, tool.ConfigurationType)
            << attrS(_DeleteExtensionsOnClean, tool.DeleteExtensionsOnClean)
            << attrS(_ImportLibrary, tool.ImportLibrary)
            << attrS(_IntermediateDirectory, tool.IntermediateDirectory)
            << attrS(_PrimaryOutput, tool.PrimaryOutput)
            << attrS(_ProgramDatabase, tool.ProgramDatabase)
            << attrT(_RegisterOutput, tool.RegisterOutput)
            << attrE(_UseOfATL, tool.UseOfATL, /*ifNot*/ useATLNotSet)
            << attrE(_UseOfMfc, tool.UseOfMfc)
            << attrT(_WholeProgramOptimization, tool.WholeProgramOptimization)
            << tool.compiler
            << tool.custom;
    if (tool.ConfigurationType == typeStaticLibrary)
        xml << tool.librarian;
    else 
        xml << tool.linker;
    xml     << tool.idl
            << tool.postBuild
            << tool.preBuild
            << tool.preLink
            << tool.resource
        << closetag(_Configuration);
    return xml;
}
// VCFilter ---------------------------------------------------------
VCFilter::VCFilter()
    :   ParseFiles(unset), Files(0), flat_files(true)
{
    useCustomBuildTool = false;
    useCompilerTool = false;
}

VCFilter::~VCFilter()
{
    delete Files;
    Files = 0;
}

void VCFilter::createOutputStructure()
{
    if (Files)
        return;

    if (flat_files)
        Files = new FlatNode;
    else
        Files = new TreeNode;
}

void VCFilter::addFile(const QString& filename)
{
    createOutputStructure();
    Files->addElement(filename, VCFilterFile(filename));
}

void VCFilter::addFile(const VCFilterFile& fileInfo) 
{
    createOutputStructure();
    Files->addElement(fileInfo.file, fileInfo);
}

void VCFilter::addFiles(const QStringList& fileList)
{
    for (int i = 0; i < fileList.count(); ++i)
        addFile(fileList.at(i));
}

void VCFilter::addMOCstage(const VCFilterFile &file, bool hdr)
{
    if (file.additionalFile.isEmpty())
        return;

    const QString &filename  = hdr ? file.file : file.additionalFile;
    const QString &mocOutput = hdr ? file.additionalFile : file.file;
    if(mocOutput.isEmpty())
        return;

    useCustomBuildTool = true;
    CustomBuildTool.Description = QString("Moc'ing %1...").arg(filename);
    QString mocApp = Project->var("QMAKE_MOC");
    CustomBuildTool.CommandLine += (mocApp + " " + customMocArguments + " " 
				+ filename + " -o " + mocOutput);
    CustomBuildTool.AdditionalDependencies = mocApp;
    CustomBuildTool.Outputs += mocOutput;
}

void VCFilter::addUICstage(QString str)
{
    useCustomBuildTool = true;
    QString uicApp = Project->var("QMAKE_UIC");
    QString mocApp = Project->var("QMAKE_MOC");
    QString fname = str.section('\\', -1);
    QString mocDir = Project->var("MOC_DIR");
    QString uiDir = Project->var("UI_DIR");
    QString uiHeaders;
    QString uiSources;

    // Determining the paths for the output files.
    int slash = str.lastIndexOf('\\');
    QString pname = (slash != -1) ? str.left(slash+1) : QString(".\\");
    if(!uiDir.isEmpty()) {
        uiHeaders = uiDir;
        uiSources = uiDir;
    } else {
        uiHeaders = Project->var("UI_HEADERS_DIR");
        uiSources = Project->var("UI_SOURCES_DIR");
        if(uiHeaders.isEmpty())
            uiHeaders = pname;
        if(uiSources.isEmpty())
            uiSources = pname;
    }
    if(!uiHeaders.endsWith("\\"))
        uiHeaders += "\\";
    if(!uiSources.endsWith("\\"))
        uiSources += "\\";

    // Determine the file name.
    int dot = fname.lastIndexOf('.');
    if(dot != -1)
        fname.truncate(dot);

    if(mocDir.isEmpty())
        mocDir = pname;

    CustomBuildTool.Description = ("Uic'ing " + str + "...\"");
    CustomBuildTool.CommandLine += // Create .h from .ui file
        uicApp + " " + str + " -o " + uiHeaders + fname + ".h";
    CustomBuildTool.CommandLine += // Create .cpp from .ui file
        uicApp + " " + str + " -i " + fname + ".h -o " + uiSources + fname + ".cpp";
    CustomBuildTool.CommandLine += // Moc the headerfile
        mocApp + " " + uiHeaders + fname + ".h -o " + mocDir + Option::h_moc_mod + fname + Option::h_moc_ext;

    CustomBuildTool.AdditionalDependencies += mocApp;
    CustomBuildTool.AdditionalDependencies += uicApp;
    CustomBuildTool.Outputs +=
        uiHeaders + fname + ".h;" + uiSources + fname + ".cpp;" + mocDir + Option::h_moc_mod + fname + Option::h_moc_ext;
}

void VCFilter::modifyPCHstage(QString str)
{
    bool isCFile = str.endsWith(".c");
    bool isHFile = (str.endsWith(".h") && str == Project->precompH);

    if(!isCFile && !isHFile)
        return;

    useCompilerTool = true;
    // Setup PCH options
    CompilerTool.UsePrecompiledHeader     = (isCFile ? pchNone : pchCreateUsingSpecific);
    CompilerTool.PrecompiledHeaderThrough = "$(NOINHERIT)";
    CompilerTool.ForcedIncludeFiles       = "$(NOINHERIT)";
}

bool VCFilter::addIMGstage(QString str)
{
    bool isCorH = false;
    if (str.endsWith(".c") || str.endsWith(".rc"))
        isCorH = true;
    QStringList::Iterator it;
    for(it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it)
        if(str.endsWith(*it))
            isCorH = true;
    for(it = Option::h_ext.begin(); it != Option::h_ext.end(); ++it)
        if(str.endsWith(*it))
            isCorH = true;

    QString collectionName = Project->project->first("QMAKE_IMAGE_COLLECTION");
    if (str.isEmpty() || isCorH || collectionName.isEmpty())
        return false;

    CustomBuildTool = VCCustomBuildTool();
    useCustomBuildTool = true;

    // Some projects (like designer core) may have too many images to
    // call uic directly. Therefor we have to create a temporary
    // file, with the image list, and call uic with the -f option.
    QString tmpFileCmd = "echo ";
    QString tmpImageFilename = ".imgcol";
    QStringList& list = Project->project->variables()["IMAGES"];
    bool firstOutput = true;
    it = list.begin();
    while(it!=list.end()) {
        tmpFileCmd += (*it) + " ";
        ++it;
        if (tmpFileCmd.length()>250 || it==list.end()) {
            CustomBuildTool.CommandLine += tmpFileCmd
                                          + (firstOutput?"> ":">> ")
                                          + tmpImageFilename;
            tmpFileCmd = "echo ";
            firstOutput = false;
        }
    }

    QString uicApp = Project->var("QMAKE_UIC");
    CustomBuildTool.Description = ("Generate imagecollection");
    CustomBuildTool.CommandLine +=
        uicApp + " -embed " + Project->project->first("QMAKE_ORIG_TARGET")
        + " -f .imgcol -o " + collectionName;
    CustomBuildTool.AdditionalDependencies += uicApp;
    CustomBuildTool.AdditionalDependencies += list;
    CustomBuildTool.Outputs = collectionName;
    CustomBuildTool.Outputs += tmpImageFilename;
    return true;
}

// Tree file generation [Start] -----------------------------------------------
void TreeNode::generateXML(XmlOutput &xml, const QString &tagName, VCFilter *tool) {
    if (children.size()) {
        // Filter
        ChildrenMap::ConstIterator it, end = children.constEnd();
        if (!tagName.isEmpty()) {
            xml << tag("Filter")
                << attr("Name", tagName)
                << attr("Filter", "");
        }
        // First round, do nested filters
        for(it = children.constBegin(); it != end; it++)
            if ((*it)->children.size())
                (*it)->generateXML(xml, it.key(), tool);
        // Second round, do leafs
        for(it = children.constBegin(); it != end; it++)
            if (!(*it)->children.size())
                (*it)->generateXML(xml, it.key(), tool);

        if (!tagName.isEmpty())
            xml << closetag("Filter");        
    } else {
        // Leaf
        tool->generateXml(xml, info);
    }
}
// Tree file generation [End] -------------------------------------------------

// Flat file generation [Start] -----------------------------------------------
void FlatNode::generateXML(XmlOutput &xml, const QString &/*tagName*/, VCFilter *tool) {
    if (children.size()) {
        ChildrenMapFlat::ConstIterator it = children.constBegin();
        ChildrenMapFlat::ConstIterator end = children.constEnd();
        for( ; it != end; it++) {
            tool->generateXml(xml, (*it));
        }
    }
}
// Flat file generation [End] -------------------------------------------------

void VCFilter::generateXml(XmlOutput &xml, const VCFilterFile &info)
{
    {
        // Clearing each filter tool
        useCustomBuildTool = false;
        useCompilerTool = false;
        CustomBuildTool = VCCustomBuildTool();
        CompilerTool = VCCLCompilerTool();

        // Unset some default options
        CompilerTool.BufferSecurityCheck = unset;
        CompilerTool.DebugInformationFormat = debugUnknown;
        CompilerTool.ExceptionHandling = unset;
        CompilerTool.GeneratePreprocessedFile = preprocessUnknown;
        CompilerTool.Optimization = optimizeDefault;
        CompilerTool.ProgramDataBaseFileName = QString::null;
        CompilerTool.RuntimeLibrary = rtUnknown;
        CompilerTool.WarningLevel = warningLevelUnknown;

        // Excluded files uses an empty compiler stage
        if(info.excludeFromBuild)
            useCompilerTool = true;

        // Add UIC, MOC and PCH stages to file
        if(CustomBuild == mocSrc) {
            if (info.file.endsWith(Option::cpp_moc_ext))
                addMOCstage(info, false);
        } else if(CustomBuild == mocHdr) {
            addMOCstage(info, true);
        } else if(CustomBuild == uic) {
            addUICstage(info.file);
        } else if (CustomBuild == resource) {
            static bool resourceBuild = false;
            if (!resourceBuild)
                resourceBuild = addIMGstage(info.file);
        }
        if(Project->usePCH)
            modifyPCHstage(info.file);
    }

    // Actual XML output ----------------------------------
    xml << tag(_File)
            << attrS(_RelativePath, info.file)
        << data(); // In case no custom builds, to avoid "/>" endings
    
    // Output custom build and compiler options for all configurations
    if(useCustomBuildTool || useCompilerTool) {
        for(int i = 0; i < Config->count(); i++) {
            xml << tag(_FileConfiguration)
                    << attr(_Name, (*Config)[i].Name)
                    << (info.excludeFromBuild ? attrS(_ExcludedFromBuild, "true") : noxml());
            if (useCustomBuildTool)
                xml << CustomBuildTool;
            if (useCompilerTool)
                xml << CompilerTool;
            xml << closetag(_FileConfiguration);
        }
    }
    xml << closetag(_File);
}

XmlOutput &operator<<(XmlOutput &xml, VCFilter &tool)
{
    if(!tool.Files || !tool.Files->hasElements())
        return xml;

    xml << tag(_Filter)
            << attrS(_Name, tool.Name)
            << attrT(_ParseFiles, tool.ParseFiles)
            << attrS(_Filter, tool.Filter);
    tool.Files->generateXML(xml, QString(), &tool);
    xml << closetag(_Filter);
    return xml;
}

// VCProject --------------------------------------------------------
VCProject::VCProject()
{
    VCConfiguration conf;
    Configuration += conf ; // Release
    //Configuration += conf ; // Debug added later, after Release init
}

XmlOutput &operator<<(XmlOutput &xml, const VCProject &tool)
{
    xml << decl("1.0", "Windows-1252")
        << tag(_VisualStudioProject)
            << attrS(_ProjectType, "Visual C++")
            << attrS(_Version, tool.Version)
            << attrS(_Name, tool.Name)
            << attrS(_ProjectGUID, tool.ProjectGUID)
            << attrS(_SccProjectName, tool.SccProjectName)
            << attrS(_SccLocalPath, tool.SccLocalPath)
            << tag(_Platforms)
                << tag(_Platform)
                    << attrS(_Name, tool.PlatformName)
            << closetag(_Platforms)
            << tag(_Configurations);
    for(int i = 0; i < tool.Configuration.count(); i++)
        xml << tool.Configuration[i];
    xml     << closetag(_Configurations)
            << tag(_Files)
                << (VCFilter&)tool.SourceFiles
                << (VCFilter&)tool.HeaderFiles
                << (VCFilter&)tool.MOCFiles
                << (VCFilter&)tool.UICFiles
                << (VCFilter&)tool.FormFiles
                << (VCFilter&)tool.TranslationFiles
                << (VCFilter&)tool.LexYaccFiles
                << (VCFilter&)tool.ResourceFiles
            << closetag(_Files)
            << tag(_Globals)
                << data(); // No "/>" end tag
    return xml;
}

