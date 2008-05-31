TEMPLATE = app
CONFIG += qt
CONFIG -= app_bundle
QT += gui
SOURCES += crashreporter.cpp
INTERFACES += crashreporter.ui
include($$PWD/mailmsg/mailmsg.pri)

mac {
	QMAKE_POST_LINK = rm -f ../test/test.app/Contents/Resources/crashreporter; mkdir -p ../test/test.app/Contents/Resources; cp crashreporter ../test/test.app/Contents/Resources
}

win32 {
	QMAKE_POST_LINK = del ..\test\release\crashreporter.exe && copy release\crashreporter.exe ..\test\release\crashreporter.exe
}

windows {
	# otherwise we would get 'unresolved external _WinMainCRTStartup'
	# when compiling with MSVC
	MOC_DIR     = _moc
	OBJECTS_DIR = _obj
	UI_DIR      = _ui
	RCC_DIR     = _rcc
}
!windows {
	MOC_DIR     = .moc
	OBJECTS_DIR = .obj
	UI_DIR      = .ui
	RCC_DIR     = .rcc
}

