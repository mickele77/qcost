# CONFIG += qt
DESTDIR = ../bin
MOC_DIR = ../moc
OBJECTS_DIR = ../obj
UI_DIR = ../ui
QMAKE_LFLAGS += " -Wl,--no-undefined"


# uncomment
DEFINES += BUILD_RELEASE

win32 {
    win32-msvc* {
        # We are using MSVC
        DEFINES += BUILD_MSVC
        # If we compile with MSVC, we use static libraries
        DEFINES += BUILD_STATIC
        # These flags, if present, have no sens with MSVC compiler
        QMAKE_LFLAGS -= " -Wl,--no-undefined"
        # path to static zlib library (not included in Qt if using MSVC)
        ZLIB_STATIC_LIB = c:/zlib/x64/zlibstat.lib
        # path to static zlib library (not included in Qt if using MSVC)
        ZLIB_INCLUDE = c:/zlib
    } else {
        # flag used only with mingw (to check wrong linking options)
        QMAKE_LFLAGS = " -Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc"
        # With mingw, if not building final release version, we use dynamic libraries
        !contains(DEFINES, BUILD_RELEASE){
            DEFINES += BUILD_SHARED_WIN
        }
    }
}

contains(DEFINES, BUILD_RELEASE) {
    # if building final release version, we use static libraries
    !contains(DEFINES, BUILD_STATIC){
        DEFINES += BUILD_STATIC
    }

    target.path = /usr/bin
    INSTALLS += target
}
