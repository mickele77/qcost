DESTDIR = ../bin
MOC_DIR = ../moc
OBJECTS_DIR = ../obj
UI_DIR = ../ui
QMAKE_LFLAGS += " -Wl,--no-undefined"

DEFINES += BUILD_RELEASE

win32 {
    DEFINES += BUILD_SHARED_WIN
    QMAKE_LFLAGS = " -Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc"
}

contains(DEFINES, BUILD_RELEASE) {
    DEFINES += BUILD_STATIC
}
