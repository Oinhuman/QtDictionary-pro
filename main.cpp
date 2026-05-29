#include "mainwindow.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // README 截图模式：用于批量导出页面图片，不进入正常事件循环。
    if(a.arguments().contains("--export-readme-screens"))
    {
        QString outputDir=QDir::current().absoluteFilePath("image");
        int index=a.arguments().indexOf("--export-readme-screens");
        if(index+1<a.arguments().size())
            outputDir=a.arguments().at(index+1);
        return w.exportReadmeScreens(outputDir) ? 0 : 1;
    }

    // 正常模式：显示主窗口并交给 Qt 事件循环。
    w.show();

    return a.exec();
}
