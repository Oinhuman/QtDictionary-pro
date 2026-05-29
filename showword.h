#ifndef SHOWWORD_H
#define SHOWWORD_H

#include <QWidget>


namespace Ui {
class ShowWord;
}

// 单词详情窗口：展示英文、音标、释义，并把朗读请求交回主窗口处理。
class ShowWord : public QWidget
{
    Q_OBJECT

public:
    explicit ShowWord(QWidget *parent = 0);
    ~ShowWord();
     void display(QString e,QString c);             // 兼容旧调用：只展示英文和释义
     void display(QString e,QString p,QString c);   // 展示英文、音标和释义
signals:
     void SpeakEnglis(QString e);                   // 请求主窗口朗读英文
private slots:
     void on_pushButton_clicked();                  // 点击详情页朗读按钮

private:

    Ui::ShowWord *ui;                               // Qt Designer 生成的详情页控件
};

#endif // SHOWWORD_H
