const AtomicValue::Ptr boolFromLex(Boolean::fromLexical(QLatin1String("1  ")));
// boolfromLex contains true

const AtomicValue::Ptr boolFromVal(Boolean::fromValue(false));
// boolFromVal contains false

const AtomicValue::Ptr lexicalError(Boolean::fromLexical(QLatin1String("3")));
// lexicalError->hasError() returns true
