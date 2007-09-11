(: 
    This query opens a Qt Designer UI file file, and outputs an SVG document
    which is an approximated rendering of the widget.

    Run it, by invoking:

    patternist reportGlobal.xq fileToOpen=widget.ui > widget.svg

    "fileToOpen=globals.gccxml" binds the string "globals.gccxml" to the variable
    "fileToOpen." It identifies the UI file file to open.

:)
declare variable $fileToOpen as xs:string external;
declare variable $doc as document-node() := doc($fileToOpen);

if (not(ends-with($fileToOpen, ".ui")))
then error((), "A Qt Designer file, ending with "".ui"", must be supplied")
else ()
,

<svg:svg xmlns:svg="http://www.w3.org/2000/svg"
         width="{$doc/ui/widget[1]/property[@name = 'geometry']/rect/height}"
         height="{$doc/ui/widget[1]/property[@name = 'geometry']/rect/width}"
>
{
    () (: TODO :)
}
</svg:svg>
