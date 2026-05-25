#include "mainwindow.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(a.arguments().contains("--export-readme-screens"))
    {
        QString outputDir=QDir::current().absoluteFilePath("image");
        int index=a.arguments().indexOf("--export-readme-screens");
        if(index+1<a.arguments().size())
            outputDir=a.arguments().at(index+1);
        return w.exportReadmeScreens(outputDir) ? 0 : 1;
    }
    w.show();

    return a.exec();
}
