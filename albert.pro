SOURCES += src/Processor.cpp
SOURCES += src/Memory.cpp
SOURCES += src/Application.cpp
SOURCES += src/albert.cpp

HEADERS += src/Processor.h
HEADERS += src/Memory.h
HEADERS += src/Application.h

CONFIG += qt

QMAKE_EXTRA_TARGETS += opcodes
opcodes.target = src/opcodes.cpp src/opnames.cpp
opcodes.commands = perl src/opcodes.pl
opcodes.depends = src/Processor.cpp
