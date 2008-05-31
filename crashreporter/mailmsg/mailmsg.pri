INCLUDEPATH  += $$PWD
DEPENDPATH  += $$PWD

win32 {
	SOURCES += $$PWD/mailmsg_windows.cpp
	HEADERS += $$PWD/mailmsg_windows.h
}
