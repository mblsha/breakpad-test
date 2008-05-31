exists($$BREAKPAD_PATH) {
	DEFINES += HAVE_BREAKPAD
	INCLUDEPATH += $$BREAKPAD_PATH
	DEPENDPATH  += $$BREAKPAD_PATH

	mac {
		# hack to make minidump_generator.cc compile as it uses
		# esp instead of __esp
		DEFINES += __DARWIN_UNIX03=0
		SOURCES += \
			$$BREAKPAD_PATH/client/mac/handler/exception_handler.cc \
			$$BREAKPAD_PATH/client/mac/handler/dynamic_images.cc \
			$$BREAKPAD_PATH/common/convert_UTF.c \
			$$BREAKPAD_PATH/common/mac/file_id.cc \
			$$BREAKPAD_PATH/common/mac/macho_id.cc \
			$$BREAKPAD_PATH/common/mac/macho_utilities.cc \
			$$BREAKPAD_PATH/common/mac/macho_walker.cc \
			$$BREAKPAD_PATH/client/minidump_file_writer.cc \
			$$BREAKPAD_PATH/client/mac/handler/minidump_generator.cc \
			$$BREAKPAD_PATH/common/string_conversion.cc \
			$$BREAKPAD_PATH/common/mac/string_utilities.cc
		LIBS += -lcrypto
	}

	win32 {
		SOURCES += \
			$$BREAKPAD_PATH/client/windows/handler/exception_handler.cc \
			$$BREAKPAD_PATH/client/windows/crash_generation/crash_generation_client.cc \
			$$BREAKPAD_PATH/common/windows/guid_string.cc
	}

	INCLUDEPATH += $$PWD
	DEPENDPATH  += $$PWD

	HEADERS += $$PWD/breakpad.h
	SOURCES += $$PWD/breakpad.cpp
}
