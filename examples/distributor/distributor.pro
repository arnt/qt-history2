# Project ID used by some IDEs
GUID 		= {f51133eb-4168-40a9-a16a-6aa9409f39ce}
TEMPLATE	= app
LANGUAGE	= C++
TARGET		= distributor

CONFIG		+= qt warn_on

QTDIR_build:REQUIRES	= full-config

SOURCES		+= main.cpp
FORMS		= distributor.ui
