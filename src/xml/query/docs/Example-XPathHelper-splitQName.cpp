const QLatin1String myQName("xhtml:body");
QString prefix;
QString ncName;

XPathHelper::splitQName(myQName, prefix, ncName);

// prefix now contains "xhtml", and ncName "body".
