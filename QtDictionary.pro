#引入 core、gui、multimedia、texttospeech 四个功能包，自动补上 widgets 控件库。
QT += core gui multimedia texttospeech
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#生成名为 QtDictionary 的可执行应用程序。
TARGET = QtDictionary
TEMPLATE = app
CONFIG += c++17

#当源码中调用已标记为废弃的接口时，编译器将输出提示信息，便于后续代码迁移。
DEFINES += QT_DEPRECATED_WARNINGS

#列出所有要编译的 .cpp 源文件
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    rbtree.cpp \
    dictionary.cpp \
    showword.cpp
#列出所有头文件。
HEADERS += \
        mainwindow.h \
    rbtree.h \
    dictionary.h \
    showword.h
#列出用 Qt Designer 拖控件画出来的 .ui 界面文件。
FORMS += \
        mainwindow.ui \
    showword.ui
#确保多媒体相关类库被正确引入编译链接。
QT += multimedia

# Keep the generated UTF-8 wordlist beside the executable as txt/output_utf8_file.txt.
WORDLIST_DIR = $$PWD/txt
WORDLIST_OUTPUT_DIR = $$OUT_PWD/txt
win32 {
    CONFIG(debug, debug|release): WORDLIST_OUTPUT_DIR = $$OUT_PWD/debug/txt
    else: WORDLIST_OUTPUT_DIR = $$OUT_PWD/release/txt
}
wordlists.files = $$files($$WORDLIST_DIR/*)
wordlists.path = $$WORDLIST_OUTPUT_DIR
COPIES += wordlists

DISTFILES += \
    transfer.py \
    txt/2.txt \
    txt/phonetics.txt \
    txt/output_utf8_file.txt
