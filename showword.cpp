#include "showword.h"
#include "ui_showword.h"

// 详情页释义展示层清洗：只修正文案，不改词库文件。
static QString readableDetailChinese(QString text)
{
    text.replace("(缩作 OK)",QStringLiteral("缩写 OK："));
    text.replace("（缩作 OK）",QStringLiteral("缩写 OK："));
    text.replace("(缩)",QStringLiteral("缩写："));
    text.replace("（缩）",QStringLiteral("缩写："));
    return text;
}

// 修复少量源数据音标乱码，保证详情页读感稳定。
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

    // 详情窗口使用独立浅色样式，弹出时不受主窗口局部样式影响。
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

// 兼容旧接口：没有音标时仍可展示英文和释义。
void ShowWord::display(QString e, QString c)
{
    display(e,QString(),c);
}

// 填充详情页三个文本框；释义和音标只做展示层格式化。
void ShowWord::display(QString e, QString p, QString c)
{

    ui->textEdit->setText(e);
    ui->textEdit_3->setText(readableDetailPhonetic(e,p));
    ui->textEdit_2->setText(readableDetailChinese(c));
}

// 点击朗读按钮时只发送信号，朗读引擎由主窗口集中管理。
void ShowWord::on_pushButton_clicked()
{
    QString e=ui->textEdit->toPlainText();
    emit SpeakEnglis(e);
}
