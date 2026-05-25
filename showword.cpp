#include "showword.h"
#include "ui_showword.h"

static QString readableDetailChinese(QString text)
{
    text.replace("(缩作 OK)",QStringLiteral("缩写 OK："));
    text.replace("（缩作 OK）",QStringLiteral("缩写 OK："));
    text.replace("(缩)",QStringLiteral("缩写："));
    text.replace("（缩）",QStringLiteral("缩写："));
    return text;
}

static QString readableDetailPhonetic(const QString &word,QString phonetic)
{
    if(word.compare("a.m",Qt::CaseInsensitive)==0 && phonetic.contains("?"))
        return QStringLiteral("/ˌeɪ ˈem/");
    if(word.compare("B.C.",Qt::CaseInsensitive)==0 && phonetic.contains("?"))
        return QStringLiteral("/ˌbiː ˈsiː/");
    if(word.compare("P.M.",Qt::CaseInsensitive)==0 && phonetic.contains("?"))
        return QStringLiteral("/ˌpiː ˈem/");
    if(word.compare("living-room",Qt::CaseInsensitive)==0 && phonetic.contains("?"))
        return QStringLiteral("/ˈlɪvɪŋ ruːm/");
    if(word.compare("secondly",Qt::CaseInsensitive)==0 && phonetic.contains("?"))
        return QStringLiteral("/ˈsekəndli/");
    if(word.compare("surprisingly",Qt::CaseInsensitive)==0 && phonetic.contains("?"))
        return QStringLiteral("/səˈpraɪzɪŋli/");
    return phonetic;
}

ShowWord::ShowWord(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowWord)
{
    ui->setupUi(this);
    setWindowTitle("单词详情");
    setStyleSheet(
        "QWidget{background:#ffffff;color:#111827;}"
        "QLabel{color:#1f2937;font-size:15px;font-weight:bold;}"
        "QTextEdit{background:#ffffff;color:#111827;border:1px solid #cbd5e1;border-radius:6px;padding:8px;selection-background-color:#bfdbfe;}"
        "QPushButton{background:#2878d0;color:#ffffff;border:1px solid #2878d0;border-radius:6px;padding:7px 16px;font-size:14px;font-weight:bold;}"
        "QPushButton:hover{background:#1d64ad;border-color:#1d64ad;}"
    );
    ui->textEdit->setReadOnly(true);
    ui->textEdit_3->setReadOnly(true);
    ui->textEdit_2->setReadOnly(true);
}

ShowWord::~ShowWord()
{
    delete ui;
}

void ShowWord::display(QString e, QString c)
{
    display(e,QString(),c);
}

void ShowWord::display(QString e, QString p, QString c)
{

    ui->textEdit->setText(e);
    ui->textEdit_3->setText(readableDetailPhonetic(e,p));
    ui->textEdit_2->setText(readableDetailChinese(c));
}

void ShowWord::on_pushButton_clicked()
{
    QString e=ui->textEdit->toPlainText();
    emit SpeakEnglis(e);
}
