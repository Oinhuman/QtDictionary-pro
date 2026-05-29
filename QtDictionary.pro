# Qt 模块：主窗口控件、音频反馈和英文朗读都在这里声明。
QT += core gui multimedia texttospeech
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 生成名为 QtDictionary 的可执行应用程序。
TARGET = QtDictionary
TEMPLATE = app
CONFIG += c++17

# 调用已废弃接口时给出编译警告，便于后续迁移。
DEFINES += QT_DEPRECATED_WARNINGS

# C++ 源文件。
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    rbtree.cpp \
    dictionary.cpp \
    showword.cpp

# 头文件。
HEADERS += \
        mainwindow.h \
    rbtree.h \
    dictionary.h \
    showword.h

# Qt Designer 生成的界面文件。
FORMS += \
        mainwindow.ui \
    showword.ui

# 保留显式 multimedia 声明，兼容旧 qmake 配置。
QT += multimedia

# 将 UTF-8 词库复制到可执行文件旁，运行时固定读取 txt/output_utf8_file.txt。
WORDLIST_DIR = $$PWD/txt
WORDLIST_OUTPUT_DIR = $$OUT_PWD/txt
win32 {
    CONFIG(debug, debug|release): WORDLIST_OUTPUT_DIR = $$OUT_PWD/debug/txt
    else: WORDLIST_OUTPUT_DIR = $$OUT_PWD/release/txt
}
wordlists.files = $$files($$WORDLIST_DIR/*)
wordlists.path = $$WORDLIST_OUTPUT_DIR
COPIES += wordlists

# 发布包需要带上转换脚本和原始/生成后的词库文件。
DISTFILES += \
    transfer.py \
    txt/2.txt \
    txt/phonetics.txt \
    txt/output_utf8_file.txt
