DESTDIR = ../bin
MOC_DIR = ../moc
OBJECTS_DIR = ../obj
UI_DIR = ../ui

# uncomment path to static zlib library
# DEFINES += BUILD_RELEASE

win32 {
    win32-msvc* {
        DEFINES += BUILD_MSVC
        DEFINES += BUILD_STATIC
        QMAKE_LFLAGS -= " -Wl,--no-undefined"
        # path to static zlib library
        ZLIB_STATIC_LIB = c:/zlib/x64/zlibstat.lib
        ZLIB_INCLUDE = c:/zlib
    } else {
        QMAKE_LFLAGS = " -Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc"
        !contains(DEFINES, BUILD_RELEASE){
            DEFINES += BUILD_SHARED_WIN
        }
    }
}

contains(DEFINES, BUILD_RELEASE) {
    DEFINES += BUILD_STATIC
}
