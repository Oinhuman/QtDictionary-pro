#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<QCoreApplication>
#include<QFile>
#include<QTextStream>
#include <QTime>
#include<QFileDialog>
#include<QFileInfo>
#include<QMessageBox>
#include<QtGlobal>
#include<QAction>
#include<QApplication>
#include<QDir>
#include<QDialog>
#include<QDialogButtonBox>
#include<QGroupBox>
#include<QHBoxLayout>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QLocale>
#include<QRadioButton>
#include<QRandomGenerator>
#include<QStandardPaths>
#include<QStatusBar>
#include<QStringListModel>
#include<QTimer>
#include<QToolBar>
#include<QToolButton>
#include<QUrl>
#include<QVBoxLayout>
#include<QPixmap>
#include<algorithm>
#include<random>

static QString lightDialogStyle()
{
    return QStringLiteral(
        "QDialog,QMessageBox{background:#ffffff;color:#111827;}"
        "QLabel{color:#111827;font-size:15px;}"
        "QPushButton{background:#ffffff;color:#111827;border:1px solid #cbd5e1;border-radius:6px;padding:7px 14px;font-size:14px;}"
        "QPushButton:hover{background:#eff6ff;border-color:#60a5fa;}"
        "QListWidget{background:#ffffff;color:#111827;border:1px solid #cbd5e1;border-radius:6px;padding:6px;}"
        "QListWidget::item{color:#111827;padding:8px;}"
        "QListWidget::item:selected{background:#bfdbfe;color:#111827;}"
    );
}

static QString readableChinese(QString text)
{
    text.replace("(缩作 OK)",QStringLiteral("缩写 OK："));
    text.replace("（缩作 OK）",QStringLiteral("缩写 OK："));
    text.replace("(缩)",QStringLiteral("缩写："));
    text.replace("（缩）",QStringLiteral("缩写："));
    return text;
}

static QString readablePhonetic(const QString &word,QString phonetic)
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

static QStringList sortedWordsForDisplay(const QStringList &words)
{
    QStringList sorted=words;
    std::sort(sorted.begin(),sorted.end(),[](const QString &left,const QString &right)
    {
        int cmp=QString::localeAwareCompare(left.toLower(),right.toLower());
        if(cmp==0)
            cmp=QString::localeAwareCompare(left,right);
        return cmp<0;
    });
    return sorted;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString wordListPath = QCoreApplication::applicationDirPath()
            + QStringLiteral("/txt/output_utf8_file.txt");
    if(!QFile::exists(wordListPath))
        wordListPath = QDir(QCoreApplication::applicationDirPath())
                .absoluteFilePath("../txt/output_utf8_file.txt");
    qDebug()<<wordListPath;
    ui->setupUi(this);
    this->setWindowTitle("电子词典");
    //2010年大学英语四级词汇.txt作为输入和保存的文本,output_utf8_file是它utf8版本
    myDictionary=new Dictionary(wordListPath);
    completer=new QCompleter(myDictionary->Englishwords,this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);//设置大小写不敏感
    ui->lineEditinPut->setCompleter(completer);     //实现模糊搜索
    group=new QButtonGroup(this);
    group->addButton(ui->checkBox,1);
    group->addButton(ui->checkBox_2,2);
    group->addButton(ui->checkBox_3,3);
    wordWidget=new ShowWord;
    Textspeak=new QTextToSpeech(this);  //发声音，用于读单词
    Textspeak->setLocale(QLocale(QLocale::English,QLocale::UnitedStates));
    Textspeak->setRate(-0.18);
    Textspeak->setPitch(1.0);
    Textspeak->setVolume(1.0);
    effectPlayer=new QMediaPlayer(this);
    effectAudioOutput=new QAudioOutput(this);
    effectPlayer->setAudioOutput(effectAudioOutput);
    actionWrongBook=NULL;
    wrongBookPage=NULL;
    rememberTipLabel=NULL;
    wrongBookTipLabel=NULL;
    wrongBookList=NULL;
    practiceWrongButton=NULL;
    masterWrongButton=NULL;
    clearWrongButton=NULL;
    readWrongButton=NULL;
    rememberOptionsWidget=NULL;
    m_rememberOptionsGroup=NULL;
    rememberWrongOnly=false;
    rememberDone=0;
    rememberRight=0;
    rememberCurrentQuestionType=0;
    m_quizTimer=NULL;
    m_currentTimeLeft=0;
    m_quizPage=NULL;
    m_quizProgressLabel=NULL;
    m_timerLabel=NULL;
    m_quizQuestionLabel=NULL;
    m_quizOptionsGroup=NULL;
    m_quizRadioGroup=NULL;
    m_quizInputEdit=NULL;
    m_quizSubmitButton=NULL;
    m_currentQuizIndex=0;
    m_quizCorrectCount=0;
    m_quizUseWrongBookOnly=false;
    i=-1;

    //接受单词详细界面传来的英文单词，进行读
    connect(wordWidget,&ShowWord::SpeakEnglis,
            [=](QString e)
    {
        speakEnglish(e);
    }
            );

    ui->textEditMean->setReadOnly(true);
    ui->lineEditinPut->setPlaceholderText("输入英文单词，按回车查询");
    ui->lineEditSure->setPlaceholderText("输入完整单词，按回车提交并进入下一题");
    connect(ui->lineEditinPut,&QLineEdit::returnPressed,this,&MainWindow::on_ButtonFind_clicked);
    connect(ui->lineEditSure,&QLineEdit::returnPressed,this,&MainWindow::on_ButtonSure_clicked);

    QString dataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(dataPath.isEmpty())
        dataPath=QCoreApplication::applicationDirPath()+QStringLiteral("/txt");
    QDir().mkpath(dataPath);
    wrongBookPath=dataPath+QStringLiteral("/wrong_words.json");
    loadWrongBook();
    initRememberPage();
    initWrongBookPage();
    initQuizPage();
    loadQuizHistory();
    m_quizTimer=new QTimer(this);
    m_quizTimer->setSingleShot(false);
    connect(m_quizTimer,&QTimer::timeout,this,&MainWindow::updateTimer);
    initNavigation();
    initAppStyle();
}

MainWindow::~MainWindow()
{

    if(m_quizTimer!=NULL && m_quizTimer->isActive())
        m_quizTimer->stop();
    saveWrongBook();
    saveQuizHistory();
    delete ui;
    delete myDictionary;
}

bool MainWindow::exportReadmeScreens(const QString &outputDir)
{
    QDir dir(outputDir);
    if(!dir.exists() && !dir.mkpath("."))
        return false;

    resize(900,560);
    show();

    auto grabPage=[&](const QString &fileName)->bool
    {
        qApp->processEvents();
        QPixmap pixmap=this->grab();
        return pixmap.save(dir.absoluteFilePath(fileName));
    };

    ui->stackedWidget->setCurrentIndex(0);
    ui->lineEditinPut->setText("abandon");
    on_ButtonFind_clicked();
    statusBar()->clearMessage();
    if(!grabPage("optimized-search.png"))
        return false;

    ui->stackedWidget->setCurrentIndex(4);
    i=myDictionary->Englishwords.indexOf("abandon");
    if(i<0)
        i=0;
    ui->labelShow->setText(makeRememberQuestion(myDictionary->Englishwords.value(i)));
    ui->lineEditHint->setText(myDictionary->Chinesewords.value(i));
    ui->lineEditSure->setText("abandon");
    ui->ButtonSure->setEnabled(true);
    ui->ButtonNext->setEnabled(true);
    rememberTipLabel->setText("当前为随机练习，答错会自动进入错题本。");
    statusBar()->clearMessage();
    if(!grabPage("optimized-remember.png"))
        return false;

    QMap<QString,WrongWord> oldWrongBook=wrongBook;
    wrongBook.clear();
    WrongWord abandon;
    abandon.ChineseWord="v. 放弃；抛弃；离弃";
    abandon.Count=3;
    abandon.LastAnswer="abandn";
    abandon.LastTime=QDateTime::currentDateTime();
    wrongBook.insert("abandon",abandon);
    WrongWord brief;
    brief.ChineseWord="adj. 短暂的；简洁的";
    brief.Count=2;
    brief.LastAnswer="brif";
    brief.LastTime=QDateTime::currentDateTime();
    wrongBook.insert("brief",brief);
    WrongWord constant;
    constant.ChineseWord="adj. 不断的；恒定的";
    constant.Count=1;
    constant.LastAnswer="constent";
    constant.LastTime=QDateTime::currentDateTime();
    wrongBook.insert("constant",constant);
    refreshWrongBook();
    ui->stackedWidget->setCurrentWidget(wrongBookPage);
    statusBar()->clearMessage();
    bool ok=grabPage("optimized-wrongbook.png");
    wrongBook=oldWrongBook;
    refreshWrongBook();
    return ok;
}

//初始化顶部一级功能按钮，取消原来的菜单层级
void MainWindow::initNavigation()
{
    ui->menuBar->hide();
    ui->mainToolBar->clear();
    ui->mainToolBar->setMovable(false);
    ui->mainToolBar->setFloatable(false);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ui->mainToolBar->setStyleSheet("QToolBar{spacing:6px;padding:6px;}QToolButton{padding:6px 14px;}");

    ui->actionFind->setText("单词查找");
    ui->actionHistory->setText("查词记录");
    ui->actionall->setText("逐条翻看");
    ui->actionRemember->setText("背单词");
    ui->actionset->setText("设置");
    actionWrongBook=new QAction("错题本",this);
    connect(actionWrongBook,&QAction::triggered,[=]()
    {
        refreshWrongBook();
        ui->stackedWidget->setCurrentWidget(wrongBookPage);
    });
    QAction *actionQuiz=new QAction("开始测验",this);
    QAction *actionQuizHistory=new QAction("测验历史",this);
    connect(actionQuiz,&QAction::triggered,this,&MainWindow::on_actionQuiz_triggered);
    connect(actionQuizHistory,&QAction::triggered,this,&MainWindow::on_actionQuizHistory_triggered);

    ui->mainToolBar->addAction(ui->actionFind);
    ui->mainToolBar->addAction(ui->actionHistory);
    ui->mainToolBar->addAction(ui->actionall);
    ui->mainToolBar->addAction(ui->actionRemember);
    ui->mainToolBar->addAction(actionWrongBook);
    ui->mainToolBar->addAction(actionQuiz);
    ui->mainToolBar->addAction(actionQuizHistory);
    ui->mainToolBar->addAction(ui->actionset);
}

void MainWindow::initAppStyle()
{
    setMinimumSize(820,560);
    setStyleSheet(
        "QMainWindow{background:#f5f7fb;}"
        "QWidget#centralWidget{background:#ffffff;}"
        "QToolBar{background:#eef2f7;border:0;border-bottom:1px solid #ccd5e1;spacing:6px;padding:8px;}"
        "QToolButton{background:transparent;color:#334155;border:0;border-radius:6px;padding:7px 14px;font-size:14px;}"
        "QToolButton:hover{background:#dbeafe;color:#1d4ed8;}"
        "QPushButton{background:#ffffff;color:#1f2937;border:1px solid #cbd5e1;border-radius:6px;padding:7px 14px;font-size:14px;}"
        "QPushButton:hover{background:#eff6ff;border-color:#60a5fa;}"
        "QPushButton#ButtonFind,QPushButton#ButtonSure{background:#2878d0;color:white;border-color:#2878d0;font-weight:bold;}"
        "QPushButton#ButtonFind:hover,QPushButton#ButtonSure:hover{background:#1d64ad;}"
        "QLineEdit,QTextEdit,QListWidget{background:#ffffff;color:#1f2937;border:1px solid #cbd5e1;border-radius:6px;padding:6px;selection-background-color:#bfdbfe;}"
        "QListWidget::item{color:#1f2937;padding:6px;}"
        "QListWidget::item:selected{background:#bfdbfe;color:#111827;}"
        "QTextEdit{font-size:18px;}"
        "QLabel{color:#1f2937;}"
        "QGroupBox{background:#ffffff;color:#1f2937;border:1px solid #cbd5e1;border-radius:6px;margin-top:14px;padding:14px 10px 10px 10px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 4px;color:#334155;background:#ffffff;}"
        "QRadioButton{color:#111827;font-size:16px;padding:8px 4px;spacing:10px;}"
        "QRadioButton::indicator{width:18px;height:18px;border-radius:9px;border:1px solid #94a3b8;background:#ffffff;}"
        "QRadioButton::indicator:checked{border:5px solid #2563eb;background:#dbeafe;}"
        "QStatusBar{background:#f8fafc;color:#475569;}"
    );
}

//初始化背单词界面，让键盘操作更顺手
void MainWindow::initRememberPage()
{
    rememberTipLabel=new QLabel("输入答案后按回车提交，答错会自动进入错题本。",this);
    rememberTipLabel->setWordWrap(true);
    rememberTipLabel->setStyleSheet("color:#777;padding:4px 0;");

    rememberOptionsWidget=new QWidget(ui->stackedWidget->widget(4));
    QVBoxLayout *optionsLayout=new QVBoxLayout(rememberOptionsWidget);
    optionsLayout->setContentsMargins(0,0,0,0);
    optionsLayout->setSpacing(6);
    m_rememberOptionsGroup=new QButtonGroup(rememberOptionsWidget);
    for(int k=0;k<4;k++)
    {
        QPushButton *button=new QPushButton(rememberOptionsWidget);
        button->setMinimumHeight(36);
        button->setStyleSheet("text-align:left;padding-left:12px;");
        m_rememberOptionButtons.append(button);
        m_rememberOptionsGroup->addButton(button,k);
        optionsLayout->addWidget(button);
        connect(button,&QPushButton::clicked,[=]()
        {
            ui->lineEditSure->setText(QString(QChar('A'+k)));
            on_ButtonSure_clicked();
        });
    }
    rememberOptionsWidget->hide();

    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(ui->stackedWidget->widget(4)->layout());
    if(layout!=NULL)
    {
        layout->insertWidget(3,rememberOptionsWidget);
        layout->insertWidget(4,rememberTipLabel);
    }

    ui->ButtonSure->setText("提交");
    ui->ButtonNext->setText("跳过");
}

//初始化错题本页面
void MainWindow::initWrongBookPage()
{
    wrongBookPage=new QWidget(this);
    QVBoxLayout *mainLayout=new QVBoxLayout(wrongBookPage);
    QHBoxLayout *titleLayout=new QHBoxLayout;
    QLabel *titleLabel=new QLabel("错题本",wrongBookPage);
    titleLabel->setStyleSheet("font-size:18px;font-weight:bold;");
    wrongBookTipLabel=new QLabel(wrongBookPage);
    wrongBookTipLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(wrongBookTipLabel,1);

    wrongBookList=new QListWidget(wrongBookPage);
    wrongBookList->setAlternatingRowColors(true);

    QHBoxLayout *buttonLayout=new QHBoxLayout;
    practiceWrongButton=new QPushButton("复习错题",wrongBookPage);
    masterWrongButton=new QPushButton("标记掌握",wrongBookPage);
    readWrongButton=new QPushButton("读选中词",wrongBookPage);
    clearWrongButton=new QPushButton("清空错题",wrongBookPage);
    buttonLayout->addWidget(practiceWrongButton);
    buttonLayout->addWidget(masterWrongButton);
    buttonLayout->addWidget(readWrongButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(clearWrongButton);

    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(wrongBookList,1);
    mainLayout->addLayout(buttonLayout);
    ui->stackedWidget->addWidget(wrongBookPage);

    connect(practiceWrongButton,&QPushButton::clicked,[=]()
    {
        if(wrongBook.isEmpty())
        {
            QMessageBox::information(this,"提示","错题本暂时为空");
            return;
        }
        startRemember(true);
    });
    connect(masterWrongButton,&QPushButton::clicked,[=]()
    {
        QListWidgetItem *item=wrongBookList->currentItem();
        if(item==NULL)
        {
            QMessageBox::information(this,"提示","请先选择一个错题");
            return;
        }
        QString e=item->data(Qt::UserRole).toString();
        wrongBook.remove(e);
        saveWrongBook();
        refreshWrongBook();
        statusBar()->showMessage("已标记掌握："+e,2500);
    });
    connect(clearWrongButton,&QPushButton::clicked,[=]()
    {
        if(wrongBook.isEmpty())
            return;
        if(QMessageBox::question(this,"提示","确定清空全部错题吗？")==QMessageBox::Yes)
        {
            wrongBook.clear();
            saveWrongBook();
            refreshWrongBook();
        }
    });
    connect(readWrongButton,&QPushButton::clicked,[=]()
    {
        QListWidgetItem *item=wrongBookList->currentItem();
        if(item!=NULL)
            speakEnglish(item->data(Qt::UserRole).toString());
    });
    connect(wrongBookList,&QListWidget::itemDoubleClicked,this,&MainWindow::showWrongBookDetail);

    refreshWrongBook();
}

//开始背单词
void MainWindow::startRemember(bool wrongOnly)
{
    rememberWrongOnly=wrongOnly;
    rememberDone=0;
    rememberRight=0;
    rememberLastTip=wrongOnly ? "当前为错题复习，默写单词，答对会降低错题次数。" : "当前为选择题练习，答错会自动进入错题本。";
    if(rememberOptionsWidget!=NULL)
        rememberOptionsWidget->setVisible(!wrongOnly);
    ui->lineEditSure->setPlaceholderText(wrongOnly ? "输入完整单词，按回车提交" : "输入 A、B、C、D 后按回车，或直接点击选项");
    ui->stackedWidget->setCurrentIndex(4);
    showNextRememberWord();
}

//显示下一个要背的单词
void MainWindow::showNextRememberWord()
{
    if(myDictionary->Englishwords.isEmpty())
    {
        ui->labelShow->setText("词库为空");
        ui->lineEditHint->clear();
        ui->lineEditSure->clear();
        ui->ButtonSure->setEnabled(false);
        ui->ButtonNext->setEnabled(false);
        return;
    }

    if(rememberWrongOnly)
    {
        QStringList candidates;
        for(QMap<QString,WrongWord>::const_iterator it=wrongBook.constBegin();it!=wrongBook.constEnd();++it)
        {
            if(myDictionary->Englishwords.contains(it.key()))
                candidates.append(it.key());
        }
        if(candidates.isEmpty())
        {
            ui->labelShow->setText("错题已清空");
            ui->lineEditHint->setText("本轮错题复习完成");
            ui->lineEditSure->clear();
            ui->ButtonSure->setEnabled(false);
            ui->ButtonNext->setEnabled(false);
            rememberTipLabel->setText(rememberLastTip+" 本轮："+QString::number(rememberRight)+"/"+QString::number(rememberDone));
            refreshWrongBook();
            return;
        }
        QString e=candidates[QRandomGenerator::global()->bounded(candidates.size())];
        i=myDictionary->Englishwords.indexOf(e);

        ui->labelShow->setText(makeRememberQuestion(e));
        rememberCurrentCorrectAnswer=e;
        rememberCurrentOptions.clear();
        if(rememberOptionsWidget!=NULL)
            rememberOptionsWidget->hide();
        ui->lineEditSure->setPlaceholderText("输入完整单词，按回车提交");
    }
    else
    {
        i=QRandomGenerator::global()->bounded(myDictionary->Englishwords.size());
        QString e=myDictionary->Englishwords[i];
        QString c=readableChinese(myDictionary->Chinesewords.value(i));
        if(c.isEmpty())
            c=readableChinese(myDictionary->Find(e));
        if(c.isEmpty())
            c="暂无释义";

        QStringList options;
        rememberCurrentQuestionType=QRandomGenerator::global()->bounded(2);
        if(rememberCurrentQuestionType==0)
        {
            ui->labelShow->setText(e+"\n\n请选择正确的中文释义：");
            options.append(c);
            while(options.size()<4 && myDictionary->Chinesewords.size()>1)
            {
                int index=QRandomGenerator::global()->bounded(myDictionary->Chinesewords.size());
                QString candidate=readableChinese(myDictionary->Chinesewords.value(index));
                if(!candidate.isEmpty() && candidate!=c && !options.contains(candidate))
                    options.append(candidate);
            }
            while(options.size()<4)
                options.append("未知");
            rememberCurrentCorrectAnswer=c;
        }
        else
        {
            ui->labelShow->setText(c+"\n\n请选择正确的英文翻译：");
            options.append(e);
            while(options.size()<4 && myDictionary->Englishwords.size()>1)
            {
                int index=QRandomGenerator::global()->bounded(myDictionary->Englishwords.size());
                QString candidate=myDictionary->Englishwords.value(index);
                if(!candidate.isEmpty() && candidate!=e && !options.contains(candidate))
                    options.append(candidate);
            }
            while(options.size()<4)
                options.append("unknown");
            rememberCurrentCorrectAnswer=e;
        }

        std::shuffle(options.begin(),options.end(),std::mt19937(QRandomGenerator::global()->generate()));
        rememberCurrentOptions=options;
        for(int k=0;k<m_rememberOptionButtons.size();k++)
        {
            if(k<options.size())
            {
                m_rememberOptionButtons[k]->setText(QString("%1. %2").arg(QChar('A'+k)).arg(options[k]));
                m_rememberOptionButtons[k]->show();
            }
            else
            {
                m_rememberOptionButtons[k]->hide();
            }
        }
        if(rememberOptionsWidget!=NULL)
            rememberOptionsWidget->show();
        ui->lineEditSure->setPlaceholderText("输入 A、B、C、D 后按回车，或直接点击选项");
    }

    ui->lineEditHint->clear();
    ui->lineEditSure->clear();
    ui->ButtonSure->setEnabled(true);
    ui->ButtonNext->setEnabled(true);
    ui->lineEditSure->setFocus();

    QString score;
    if(rememberDone>0)
        score=" 本轮："+QString::number(rememberRight)+"/"+QString::number(rememberDone);
    rememberTipLabel->setText(rememberLastTip+score);
}

//生成背单词题面
QString MainWindow::makeRememberQuestion(QString word) const
{
    if(word.isEmpty())
        return QString();

    QString question=word;
    int length=word.size();
    int maskCount=qBound(1,length/3,length>8 ? 4 : 3);
    if(length<=3)
        maskCount=1;

    QList<int> positions;
    while(positions.size()<maskCount)
    {
        int begin=length>3 ? 1 : 0;
        int range=length>3 ? length-2 : length;
        int pos=begin+QRandomGenerator::global()->bounded(range);
        if(!positions.contains(pos))
            positions.append(pos);
    }

    for(int k=0;k<positions.size();k++)
        question.replace(positions[k],1,"_");
    return question;
}

//加入错题本
void MainWindow::addWrongWord(QString Englishword, QString answer)
{
    if(Englishword.isEmpty())
        return;

    WrongWord item;
    if(wrongBook.contains(Englishword))
        item=wrongBook.value(Englishword);
    else
    {
        QString p=readablePhonetic(Englishword,myDictionary->FindPhonetic(Englishword));
        QString c=readableChinese(myDictionary->Find(Englishword));
        item.ChineseWord=p.isEmpty() ? c : p+" "+c;
        item.Count=0;
    }

    item.Count++;
    item.LastAnswer=answer.isEmpty() ? "未填写" : answer;
    item.LastTime=QDateTime::currentDateTime();
    wrongBook[Englishword]=item;
    saveWrongBook();
    refreshWrongBook();
}

//读取错题本
void MainWindow::loadWrongBook()
{
    wrongBook.clear();
    QFile file(wrongBookPath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return;

    QJsonDocument doc=QJsonDocument::fromJson(file.readAll());
    file.close();
    if(!doc.isArray())
        return;

    QJsonArray array=doc.array();
    for(int k=0;k<array.size();k++)
    {
        QJsonObject obj=array[k].toObject();
        QString e=obj.value("english").toString();
        if(e.isEmpty())
            continue;
        WrongWord item;
        item.ChineseWord=obj.value("chinese").toString();
        item.Count=obj.value("count").toInt(1);
        item.LastAnswer=obj.value("lastAnswer").toString();
        item.LastTime=QDateTime::fromString(obj.value("lastTime").toString(),Qt::ISODate);
        wrongBook[e]=item;
    }
}

//保存错题本
void MainWindow::saveWrongBook()
{
    if(wrongBookPath.isEmpty())
        return;

    QDir().mkpath(QFileInfo(wrongBookPath).absolutePath());
    QFile file(wrongBookPath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return;

    QJsonArray array;
    for(QMap<QString,WrongWord>::const_iterator it=wrongBook.constBegin();it!=wrongBook.constEnd();++it)
    {
        QJsonObject obj;
        obj.insert("english",it.key());
        obj.insert("chinese",it.value().ChineseWord);
        obj.insert("count",it.value().Count);
        obj.insert("lastAnswer",it.value().LastAnswer);
        obj.insert("lastTime",it.value().LastTime.toString(Qt::ISODate));
        array.append(obj);
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    file.close();
}

//刷新错题本界面
void MainWindow::refreshWrongBook()
{
    if(wrongBookList==NULL)
        return;

    wrongBookList->clear();
    int total=0;
    for(QMap<QString,WrongWord>::const_iterator it=wrongBook.constBegin();it!=wrongBook.constEnd();++it)
    {
        total+=it.value().Count;
        QString timeText=it.value().LastTime.isValid() ? it.value().LastTime.toString("MM-dd hh:mm") : "暂无";
        QString text=QString("%1    错 %2 次    最近：%3").arg(it.key()).arg(it.value().Count).arg(timeText);
        QListWidgetItem *item=new QListWidgetItem(text,wrongBookList);
        item->setData(Qt::UserRole,it.key());
        item->setToolTip("中文："+it.value().ChineseWord+"\n最近错答："+it.value().LastAnswer);
    }

    wrongBookTipLabel->setText(QString("单词 %1 个，累计错 %2 次").arg(wrongBook.size()).arg(total));
    bool hasWrong=!wrongBook.isEmpty();
    practiceWrongButton->setEnabled(hasWrong);
    masterWrongButton->setEnabled(hasWrong);
    clearWrongButton->setEnabled(hasWrong);
    readWrongButton->setEnabled(hasWrong);
}

//显示错题详情
void MainWindow::showWrongBookDetail(QListWidgetItem *item)
{
    if(item==NULL)
        return;

    QString e=item->data(Qt::UserRole).toString();
    QString p=readablePhonetic(e,myDictionary->FindPhonetic(e));
    QString c=readableChinese(myDictionary->Find(e));
    if(c.isEmpty() && wrongBook.contains(e))
        c=wrongBook.value(e).ChineseWord;
    wordWidget->display(e,p,c);
    wordWidget->show();
}

//播放背词反馈音
void MainWindow::playRememberSound(bool correct)
{
    QString fileName=correct ? "正确.mp3" : "错误.mp3";
    QString path=QCoreApplication::applicationDirPath()+QStringLiteral("/music/")+fileName;
    if(QFile::exists(path))
    {
        effectPlayer->stop();
        effectPlayer->setSource(QUrl::fromLocalFile(path));
        effectPlayer->play();
    }
    else if(!correct)
        QApplication::beep();
}

void MainWindow::speakEnglish(QString word)
{
    word=word.trimmed();
    if(word.isEmpty() || Textspeak==NULL)
        return;

    Textspeak->stop();
    QTimer::singleShot(120,this,[this,word]()
    {
        if(Textspeak!=NULL)
            Textspeak->say(word);
    });
}

//设置界面的ok，用于删除、增加单词
void MainWindow::on_buttonBox_accepted()
{
 //qDebug()<< group->checkedId();
    QStringListModel *model=qobject_cast<QStringListModel*>(completer->model());
    if(group->checkedId()==1)       //第一种情况插入单词
    {

        QString e=ui->textEditEnglish->toPlainText().trimmed();
        QString c=ui->textEditChinese->toPlainText().trimmed();
        QString path=ui->lineEditFile->text();
        bool needSave=false;
        if(!path.isEmpty())//如果路径不为空执行文本插入
        {
            int count=myDictionary->insertFile(path);
            if(model!=NULL && count>0)
                model->setStringList(myDictionary->Englishwords);
            if(count>0)
            {
                QMessageBox::information(this,"提示","文本导入成功，共导入 "+QString::number(count)+" 个单词");
                needSave=true;
            }
            else
                QMessageBox::information(this,"提示","未导入新单词，请检查文件或重复单词");
        }

        if(!e.isEmpty() && !c.isEmpty() && myDictionary->insert(e,c))
        {
            if(model!=NULL)
                model->setStringList(myDictionary->Englishwords);
            QMessageBox::information(this,"提示","插入成功");
            needSave=true;
        }
        else if(!e.isEmpty() || !c.isEmpty())
            QMessageBox::information(this,"提示","插入失败");
        if(needSave)
            myDictionary->saveToFile();
    }
    else if (group->checkedId()==2) //第二种情况删除单词
    {
        QString e=ui->textEditEnglish->toPlainText().trimmed();
        if(myDictionary->remove(e))
        {
            if(model!=NULL)
                model->setStringList(myDictionary->Englishwords);
            wrongBook.remove(e);
            saveWrongBook();
            refreshWrongBook();
            myDictionary->saveToFile();
            QMessageBox::information(this,"提示","删除成功");
        }
        else
            QMessageBox::information(this,"提示","删除失败");
    }
    else if (group->checkedId()==3) //第三种情况修改单词
    {
        QString e=ui->textEditEnglish->toPlainText().trimmed();
        QString c=ui->textEditChinese->toPlainText().trimmed();
       if(myDictionary->update(e,c))
       {
           if(wrongBook.contains(e))
           {
               WrongWord item=wrongBook.value(e);
               QString p=readablePhonetic(e,myDictionary->FindPhonetic(e));
               QString displayChinese=readableChinese(c);
               item.ChineseWord=p.isEmpty() ? displayChinese : p+" "+displayChinese;
               wrongBook[e]=item;
               saveWrongBook();
               refreshWrongBook();
           }
           myDictionary->saveToFile();
           QMessageBox::information(this,"提示","更新成功");
       }
       else
       {
           QMessageBox::information(this,"提示","更新失败");
       }
    }
}

//设置界面的cancel
void MainWindow::on_buttonBox_rejected()
{
    ui->stackedWidget->setCurrentIndex(0);//跳回查询界面
}

//逐条查找，可以查找有单词
void MainWindow::on_actionall_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->listWidget->clear();
    ui->listWidget->addItems(sortedWordsForDisplay(myDictionary->Englishwords));
}

//查找界面
void MainWindow::on_actionFind_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->lineEditinPut->clear();
    ui->textEditMean->clear();
}

//按下查找按钮
void MainWindow::on_ButtonFind_clicked()
{
   QString e=ui->lineEditinPut->text().trimmed();
   if(e.isEmpty())
   {
       statusBar()->showMessage("请输入要查询的英文单词",2000);
       ui->lineEditinPut->setFocus();
       return;
   }
   ui->textEditMean->clear();
   QString c=readableChinese(myDictionary->Find(e));
   if(!c.isEmpty())
   {
       QString p=readablePhonetic(e,myDictionary->FindPhonetic(e));
       ui->textEditMean->setText(p.isEmpty() ? c : p+"\n"+c);
       if(history.contains(e))
       {
           history.removeAll(e);
       }
       history.append(e);
       statusBar()->showMessage("查询完成："+e,2000);
   }else
       QMessageBox::information(this,"提示","词库无此单词");

}
//查看历史
void MainWindow::on_actionHistory_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->listWidgetHistory->clear();
    ui->listWidgetHistory->addItems(history);
}
//设置，用于增加、删除、更新单词
void MainWindow::on_actionset_triggered()
{
     ui->stackedWidget->setCurrentIndex(3);
}
//读单词
void MainWindow::on_ButtonRead_clicked()
{
     QString e=ui->lineEditinPut->text();

     speakEnglish(e);
}
//背单词
void MainWindow::on_actionRemember_triggered()
{
    startRemember(false);
}
//背单词填写好的确定
void MainWindow::on_ButtonSure_clicked()
{
    if(i<0 || i>=myDictionary->Englishwords.size())
    {
        showNextRememberWord();
        return;
    }

    QString answer=ui->lineEditSure->text().trimmed();
    if(answer.isEmpty())
    {
        statusBar()->showMessage("请输入完整单词后再提交",2000);
        ui->lineEditSure->setFocus();
        return;
    }

    QString rightWord=myDictionary->Englishwords[i];
    bool correct=false;
    QString userAnswer=answer;
    if(rememberWrongOnly)
    {
        correct=answer.compare(rightWord,Qt::CaseInsensitive)==0;
    }
    else
    {
        QString optionKey=answer.toUpper();
        int selectedIndex=-1;
        if(optionKey=="A")
            selectedIndex=0;
        else if(optionKey=="B")
            selectedIndex=1;
        else if(optionKey=="C")
            selectedIndex=2;
        else if(optionKey=="D")
            selectedIndex=3;

        if(selectedIndex>=0 && selectedIndex<rememberCurrentOptions.size())
        {
            userAnswer=rememberCurrentOptions[selectedIndex];
            correct=(userAnswer==rememberCurrentCorrectAnswer);
        }
        else
        {
            correct=(answer.compare(rememberCurrentCorrectAnswer,Qt::CaseInsensitive)==0);
        }
    }

    rememberDone++;
    if(correct)
    {
        rememberRight++;
        if(rememberWrongOnly && wrongBook.contains(rightWord))
        {
            WrongWord item=wrongBook.value(rightWord);
            item.Count--;
            if(item.Count<=0)
            {
                wrongBook.remove(rightWord);
                rememberLastTip="上一题正确："+rightWord+"，已从错题本移除。";
            }
            else
            {
                wrongBook[rightWord]=item;
                rememberLastTip="上一题正确："+rightWord+"，错题次数减 1。";
            }
            saveWrongBook();
            refreshWrongBook();
        }
        else
            rememberLastTip="上一题正确："+rightWord;
    }
    else
    {
        addWrongWord(rightWord,userAnswer);
        QString correctText=rememberWrongOnly ? rightWord : rememberCurrentCorrectAnswer;
        rememberLastTip="上一题错误，正确答案："+correctText+"，已加入错题本。";
    }

    playRememberSound(correct);
    showNextRememberWord();

}
//提示中文
void MainWindow::on_Buttonhint_clicked()
{
    if(i<0 || i>=myDictionary->Chinesewords.size())
        return;
    if(!rememberWrongOnly)
    {
        ui->lineEditHint->setText("请点击选项，或输入 A、B、C、D 后按回车");
        return;
    }
    QString c=readableChinese(myDictionary->Chinesewords[i]);
    ui->lineEditHint->setText(c);
}
//对要背的英语单词进行读
void MainWindow::on_ButtonReadHint_clicked()
{
    if(i<0 || i>=myDictionary->Englishwords.size())
        return;
    QString e=myDictionary->Englishwords[i];
    speakEnglish(e);
}

//逐条浏览时点出单词的详细界面
void MainWindow::on_listWidget_itemPressed(QListWidgetItem *item)
{
    QString e=item->text();
    QString p=readablePhonetic(e,myDictionary->FindPhonetic(e));
    QString c=readableChinese(myDictionary->Find(e));
    wordWidget->display(e,p,c);
    wordWidget->show();
}

//查看历史时点出单词的详细界面
void MainWindow::on_listWidgetHistory_itemPressed(QListWidgetItem *item)
{
    QString e=item->text();
    QString p=readablePhonetic(e,myDictionary->FindPhonetic(e));
    QString c=readableChinese(myDictionary->Find(e));
    wordWidget->display(e,p,c);
    wordWidget->show();
}

//设置时进行查看,可以查看单词的主要信息
void MainWindow::on_Button_3_clicked()
{
    QString e=ui->textEditEnglish->toPlainText().trimmed();
    QString p=readablePhonetic(e,myDictionary->FindPhonetic(e));
    QString c=readableChinese(myDictionary->Find(e));
    if(c.isEmpty())
       QMessageBox::information(this,"提示","词库无此单词");
    else
    {
        wordWidget->display(e,p,c);
        wordWidget->show();
    }
}
//插入时可以通过文本插入
void MainWindow::on_ButtonFile_clicked()
{
  QString  fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image"), "E:/", tr("Text files (*.txt)"));
  //qDebug()<<fileName;
 // myDictionary->insertFile(fileName);
  ui->lineEditFile->setText(fileName);
}
//背单词界面下一个按键
void MainWindow::on_ButtonNext_clicked()
{
    if(i>=0 && i<myDictionary->Englishwords.size())
    {
        QString word=myDictionary->Englishwords[i];
        if(rememberWrongOnly)
        {
            rememberLastTip="已跳过："+word;
        }
        else
        {
            rememberLastTip="已跳过："+word+"，已加入错题本。";
            addWrongWord(word,"跳过未答");
        }
    }
    showNextRememberWord();
}

QString MainWindow::getCurrentTimeString() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

QString MainWindow::getRating(double score) const
{
    if(score>=90)
        return "优秀";
    if(score>=80)
        return "良好";
    if(score>=70)
        return "中等";
    if(score>=60)
        return "及格";
    return "需要继续练习";
}

QStringList MainWindow::getWrongBookWordList() const
{
    QStringList words;
    for(QMap<QString,WrongWord>::const_iterator it=wrongBook.constBegin();it!=wrongBook.constEnd();++it)
    {
        if(myDictionary->Englishwords.contains(it.key()))
            words.append(it.key());
    }
    return words;
}

void MainWindow::loadQuizHistory()
{
    QString dataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(dataPath.isEmpty())
        dataPath=QCoreApplication::applicationDirPath()+QStringLiteral("/txt");
    m_quizHistoryPath=dataPath+QStringLiteral("/quiz_history.json");
    m_quizHistoryList.clear();

    QFile file(m_quizHistoryPath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return;

    QJsonDocument doc=QJsonDocument::fromJson(file.readAll());
    file.close();
    if(!doc.isArray())
        return;

    QJsonArray array=doc.array();
    for(int k=0;k<array.size();k++)
    {
        QJsonObject obj=array[k].toObject();
        QuizRecord record;
        record.TestTime=obj.value("testTime").toString();
        record.TestRange=obj.value("testRange").toString();
        record.TotalQuestions=obj.value("totalQuestions").toInt();
        record.CorrectCount=obj.value("correctCount").toInt();
        record.Score=obj.value("score").toDouble();
        QJsonArray errors=obj.value("errorWords").toArray();
        for(int j=0;j<errors.size();j++)
            record.ErrorWords.append(errors[j].toString());
        if(!record.TestTime.isEmpty())
            m_quizHistoryList.append(record);
    }
}

void MainWindow::saveQuizHistory()
{
    if(m_quizHistoryPath.isEmpty())
        return;

    QDir().mkpath(QFileInfo(m_quizHistoryPath).absolutePath());
    QFile file(m_quizHistoryPath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return;

    QJsonArray array;
    for(const QuizRecord &record:m_quizHistoryList)
    {
        QJsonObject obj;
        obj.insert("testTime",record.TestTime);
        obj.insert("testRange",record.TestRange);
        obj.insert("totalQuestions",record.TotalQuestions);
        obj.insert("correctCount",record.CorrectCount);
        obj.insert("score",record.Score);
        QJsonArray errors;
        for(const QString &word:record.ErrorWords)
            errors.append(word);
        obj.insert("errorWords",errors);
        array.append(obj);
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    file.close();
}

void MainWindow::initQuizPage()
{
    m_quizPage=new QWidget(this);
    m_quizPage->setStyleSheet(
        "QWidget{background:#ffffff;color:#111827;}"
        "QLabel{color:#111827;}"
        "QGroupBox{background:#ffffff;color:#1f2937;border:1px solid #cbd5e1;border-radius:6px;margin-top:14px;padding:14px 12px 12px 12px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 4px;color:#334155;background:#ffffff;}"
        "QRadioButton{color:#111827;font-size:16px;padding:8px 4px;spacing:10px;}"
        "QRadioButton::indicator{width:18px;height:18px;border-radius:9px;border:1px solid #94a3b8;background:#ffffff;}"
        "QRadioButton::indicator:checked{border:5px solid #2563eb;background:#dbeafe;}"
        "QPushButton{background:#ffffff;color:#111827;border:1px solid #cbd5e1;border-radius:6px;padding:7px 14px;font-size:14px;}"
        "QPushButton:hover{background:#eff6ff;border-color:#60a5fa;}"
        "QLineEdit{background:#ffffff;color:#111827;border:1px solid #cbd5e1;border-radius:6px;padding:7px;}"
    );
    QVBoxLayout *mainLayout=new QVBoxLayout(m_quizPage);

    QHBoxLayout *topLayout=new QHBoxLayout;
    m_quizProgressLabel=new QLabel(m_quizPage);
    m_timerLabel=new QLabel(m_quizPage);
    m_timerLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    topLayout->addWidget(m_quizProgressLabel);
    topLayout->addWidget(m_timerLabel,1);
    mainLayout->addLayout(topLayout);

    m_quizQuestionLabel=new QLabel(m_quizPage);
    m_quizQuestionLabel->setAlignment(Qt::AlignCenter);
    m_quizQuestionLabel->setWordWrap(true);
    m_quizQuestionLabel->setMinimumHeight(120);
    m_quizQuestionLabel->setStyleSheet("font-size:20px;font-weight:bold;padding:18px;");
    mainLayout->addWidget(m_quizQuestionLabel);

    m_quizOptionsGroup=new QGroupBox("请选择答案",m_quizPage);
    QVBoxLayout *optionsLayout=new QVBoxLayout(m_quizOptionsGroup);
    optionsLayout->setContentsMargins(18,14,18,14);
    optionsLayout->setSpacing(6);
    m_quizRadioGroup=new QButtonGroup(m_quizOptionsGroup);
    for(int k=0;k<4;k++)
    {
        QRadioButton *radio=new QRadioButton(m_quizOptionsGroup);
        radio->setMinimumHeight(38);
        radio->setCursor(Qt::PointingHandCursor);
        radio->setStyleSheet("QRadioButton{color:#111827;font-size:16px;padding:8px 4px;spacing:10px;}QRadioButton::indicator{width:18px;height:18px;border-radius:9px;border:1px solid #94a3b8;background:#ffffff;}QRadioButton::indicator:checked{border:5px solid #2563eb;background:#dbeafe;}");
        m_quizRadioButtons.append(radio);
        m_quizRadioGroup->addButton(radio,k);
        optionsLayout->addWidget(radio);
        connect(radio,&QRadioButton::clicked,[radio]()
        {
            radio->setChecked(true);
        });
    }
    mainLayout->addWidget(m_quizOptionsGroup);

    m_quizInputEdit=new QLineEdit(m_quizPage);
    m_quizInputEdit->setPlaceholderText("请输入完整英文单词");
    m_quizInputEdit->hide();
    connect(m_quizInputEdit,&QLineEdit::returnPressed,this,&MainWindow::on_ButtonSubmitAnswer_clicked);
    mainLayout->addWidget(m_quizInputEdit);

    QHBoxLayout *buttonLayout=new QHBoxLayout;
    buttonLayout->addStretch();
    m_quizSubmitButton=new QPushButton("提交答案",m_quizPage);
    connect(m_quizSubmitButton,&QPushButton::clicked,this,&MainWindow::on_ButtonSubmitAnswer_clicked);
    buttonLayout->addWidget(m_quizSubmitButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    ui->stackedWidget->addWidget(m_quizPage);
}

void MainWindow::startQuiz(bool useWrongBookOnly)
{
    m_quizUseWrongBookOnly=useWrongBookOnly;
    m_currentQuizIndex=0;
    m_quizCorrectCount=0;
    generateQuizQuestions();

    if(m_quizQuestions.isEmpty())
    {
        QMessageBox::information(this,"提示","没有题目可供测验");
        return;
    }

    ui->stackedWidget->setCurrentWidget(m_quizPage);
    showCurrentQuizQuestion();
}

void MainWindow::generateQuizQuestions()
{
    m_quizQuestions.clear();

    QStringList sourceWords;
    if(m_quizUseWrongBookOnly)
    {
        sourceWords=getWrongBookWordList();
        m_quizRangeName="错题复习";
    }
    else
    {
        sourceWords=myDictionary->Englishwords;
        m_quizRangeName="随机练习";
    }

    if(sourceWords.isEmpty())
        return;

    std::shuffle(sourceWords.begin(),sourceWords.end(),std::mt19937(QRandomGenerator::global()->generate()));
    int questionCount=qMin(15,sourceWords.size());
    for(int k=0;k<questionCount;k++)
    {
        int type=(k%3)+1;
        generateQuestionOfType(sourceWords[k],type);
    }
    std::shuffle(m_quizQuestions.begin(),m_quizQuestions.end(),std::mt19937(QRandomGenerator::global()->generate()));
}

void MainWindow::generateQuestionOfType(const QString &word,int type)
{
    QuizQuestion question;
    question.English=word;
    int wordIndex=myDictionary->Englishwords.indexOf(word);
    question.Chinese=readableChinese(wordIndex>=0 ? myDictionary->Chinesewords.value(wordIndex) : myDictionary->Find(word));
    if(question.Chinese.isEmpty() && wrongBook.contains(word))
        question.Chinese=wrongBook.value(word).ChineseWord;
    if(question.Chinese.isEmpty())
        question.Chinese="暂无释义";
    question.Type=type;
    question.IsCorrect=false;

    if(type==1)
    {
        question.CorrectAnswer=question.Chinese;
        question.Options.append(question.Chinese);
        while(question.Options.size()<4 && myDictionary->Chinesewords.size()>1)
        {
            int index=QRandomGenerator::global()->bounded(myDictionary->Chinesewords.size());
            QString candidate=readableChinese(myDictionary->Chinesewords.value(index));
            if(!candidate.isEmpty() && candidate!=question.Chinese && !question.Options.contains(candidate))
                question.Options.append(candidate);
        }
        while(question.Options.size()<4)
            question.Options.append("未知");
        std::shuffle(question.Options.begin(),question.Options.end(),std::mt19937(QRandomGenerator::global()->generate()));
    }
    else if(type==2)
    {
        question.CorrectAnswer=question.English;
        question.Options.append(question.English);
        while(question.Options.size()<4 && myDictionary->Englishwords.size()>1)
        {
            int index=QRandomGenerator::global()->bounded(myDictionary->Englishwords.size());
            QString candidate=myDictionary->Englishwords.value(index);
            if(!candidate.isEmpty() && candidate!=question.English && !question.Options.contains(candidate))
                question.Options.append(candidate);
        }
        while(question.Options.size()<4)
            question.Options.append("unknown");
        std::shuffle(question.Options.begin(),question.Options.end(),std::mt19937(QRandomGenerator::global()->generate()));
    }
    else
    {
        question.CorrectAnswer=question.English;
    }

    m_quizQuestions.append(question);
}

void MainWindow::showCurrentQuizQuestion()
{
    if(m_currentQuizIndex>=m_quizQuestions.size())
    {
        showQuizResult();
        return;
    }

    if(m_quizTimer!=NULL && m_quizTimer->isActive())
        m_quizTimer->stop();

    QuizQuestion &question=m_quizQuestions[m_currentQuizIndex];
    m_quizProgressLabel->setText(QString("第 %1 / %2 题").arg(m_currentQuizIndex+1).arg(m_quizQuestions.size()));
    m_currentTimeLeft=question.Type==3 ? 10 : 5;
    m_timerLabel->setText(QString("剩余时间：%1 秒").arg(m_currentTimeLeft));
    m_timerLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#d97706;");

    if(question.Type==1)
        m_quizQuestionLabel->setText(question.English+"\n\n请选择正确的中文释义：");
    else if(question.Type==2)
        m_quizQuestionLabel->setText(question.Chinese+"\n\n请选择正确的英文翻译：");
    else
        m_quizQuestionLabel->setText(question.Chinese+"\n\n请默写完整英文单词：");

    m_quizRadioGroup->setExclusive(false);
    for(int k=0;k<m_quizRadioButtons.size();k++)
    {
        m_quizRadioButtons[k]->setAutoExclusive(false);
        m_quizRadioButtons[k]->setChecked(false);
        if(k<question.Options.size())
        {
            m_quizRadioButtons[k]->setText(QString("%1. %2").arg(QChar('A'+k)).arg(question.Options[k]));
            m_quizRadioButtons[k]->show();
        }
        else
        {
            m_quizRadioButtons[k]->hide();
        }
        m_quizRadioButtons[k]->setAutoExclusive(true);
    }
    m_quizRadioGroup->setExclusive(true);

    if(question.Type==3)
    {
        m_quizOptionsGroup->hide();
        m_quizInputEdit->show();
        m_quizInputEdit->clear();
        m_quizInputEdit->setFocus();
    }
    else
    {
        m_quizInputEdit->hide();
        m_quizOptionsGroup->show();
    }

    m_quizSubmitButton->setEnabled(true);
    if(m_quizTimer!=NULL)
        m_quizTimer->start(1000);
}

void MainWindow::updateTimer()
{
    if(m_currentTimeLeft>0)
    {
        m_currentTimeLeft--;
        m_timerLabel->setText(QString("剩余时间：%1 秒").arg(m_currentTimeLeft));
        if(m_currentTimeLeft<=3)
            m_timerLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#dc2626;");
    }

    if(m_currentTimeLeft<=0)
    {
        if(m_quizTimer!=NULL && m_quizTimer->isActive())
            m_quizTimer->stop();
        handleQuizTimeout();
    }
}

void MainWindow::handleQuizTimeout()
{
    if(m_currentQuizIndex>=m_quizQuestions.size())
        return;

    QuizQuestion &question=m_quizQuestions[m_currentQuizIndex];
    if(!question.UserAnswer.isEmpty())
        return;

    question.UserAnswer="超时未答";
    question.IsCorrect=false;
    addWrongWord(question.English,question.UserAnswer);
    showAnswerDialog(false,question.CorrectAnswer);
    m_currentQuizIndex++;
    showCurrentQuizQuestion();
}

void MainWindow::evaluateAndRecord()
{
    if(m_currentQuizIndex>=m_quizQuestions.size())
        return;

    QuizQuestion &question=m_quizQuestions[m_currentQuizIndex];
    if(!question.UserAnswer.isEmpty())
        return;

    if(question.Type==1 || question.Type==2)
    {
        int selectedId=m_quizRadioGroup->checkedId();
        if(selectedId>=0 && selectedId<question.Options.size())
        {
            question.UserAnswer=question.Options[selectedId];
            question.IsCorrect=(question.UserAnswer==question.CorrectAnswer);
        }
        else
        {
            question.UserAnswer="未选择";
            question.IsCorrect=false;
        }
    }
    else
    {
        question.UserAnswer=m_quizInputEdit->text().trimmed();
        question.IsCorrect=(question.UserAnswer.compare(question.English,Qt::CaseInsensitive)==0);
    }

    if(question.IsCorrect)
        m_quizCorrectCount++;
}

void MainWindow::showAnswerDialog(bool isCorrect,const QString &correctAnswer)
{
    QDialog dialog(this);
    dialog.setWindowTitle(isCorrect ? "回答正确" : "回答错误");
    dialog.setModal(true);
    dialog.setMinimumWidth(360);
    dialog.setStyleSheet(lightDialogStyle());

    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QLabel *titleLabel=new QLabel(isCorrect ? "回答正确" : "回答错误",&dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(isCorrect ? "font-size:22px;font-weight:bold;color:#15803d;" : "font-size:22px;font-weight:bold;color:#dc2626;");
    layout->addWidget(titleLabel);

    QLabel *answerLabel=new QLabel("正确答案："+correctAnswer,&dialog);
    answerLabel->setAlignment(Qt::AlignCenter);
    answerLabel->setWordWrap(true);
    answerLabel->setStyleSheet("color:#111827;font-size:16px;padding:8px;");
    layout->addWidget(answerLabel);

    QDialogButtonBox *buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok,&dialog);
    connect(buttonBox,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    layout->addWidget(buttonBox);

    QTimer::singleShot(3000,&dialog,&QDialog::accept);
    dialog.exec();
}

void MainWindow::on_ButtonSubmitAnswer_clicked()
{
    if(m_currentQuizIndex>=m_quizQuestions.size())
        return;

    if(m_quizTimer!=NULL && m_quizTimer->isActive())
        m_quizTimer->stop();

    evaluateAndRecord();
    QuizQuestion &question=m_quizQuestions[m_currentQuizIndex];
    showAnswerDialog(question.IsCorrect,question.CorrectAnswer);
    if(question.IsCorrect)
    {
        playRememberSound(true);
    }
    else
    {
        addWrongWord(question.English,question.UserAnswer);
        playRememberSound(false);
    }

    m_currentQuizIndex++;
    showCurrentQuizQuestion();
}

void MainWindow::showQuizResult()
{
    if(m_quizTimer!=NULL && m_quizTimer->isActive())
        m_quizTimer->stop();
    if(m_quizQuestions.isEmpty())
        return;

    int total=m_quizQuestions.size();
    double score=(double)m_quizCorrectCount/total*100.0;
    QStringList errorWords;
    for(const QuizQuestion &question:m_quizQuestions)
    {
        if(!question.IsCorrect)
            errorWords.append(question.English);
    }

    QuizRecord record;
    record.TestTime=getCurrentTimeString();
    record.TestRange=m_quizRangeName;
    record.TotalQuestions=total;
    record.CorrectCount=m_quizCorrectCount;
    record.Score=score;
    record.ErrorWords=errorWords;
    m_quizHistoryList.append(record);
    saveQuizHistory();

    QString resultText=QString("测验时间：%1\n测验范围：%2\n总题数：%3\n答对：%4\n得分：%5\n评级：%6\n")
            .arg(record.TestTime)
            .arg(record.TestRange)
            .arg(total)
            .arg(m_quizCorrectCount)
            .arg(score,0,'f',1)
            .arg(getRating(score));
    if(!errorWords.isEmpty())
    {
        resultText+="\n错词列表：\n";
        for(const QString &word:errorWords)
            resultText+="- "+word+"\n";
        resultText+="\n错词已加入错题本。";
    }
    else
    {
        resultText+="\n本次全部答对。";
    }

    QMessageBox resultBox(this);
    resultBox.setWindowTitle("测验结果");
    resultBox.setStyleSheet(lightDialogStyle());
    resultBox.setText(resultText);
    QPushButton *retryButton=resultBox.addButton("再测一次",QMessageBox::AcceptRole);
    resultBox.addButton("返回查词",QMessageBox::RejectRole);
    resultBox.exec();

    if(resultBox.clickedButton()==retryButton)
        startQuiz(m_quizUseWrongBookOnly);
    else
        ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_actionQuiz_triggered()
{
    QDialog dialog(this);
    dialog.setWindowTitle("选择测验范围");
    dialog.setStyleSheet(lightDialogStyle());
    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QLabel *label=new QLabel("请选择测验范围：",&dialog);
    layout->addWidget(label);
    QPushButton *randomButton=new QPushButton("随机练习（全部单词）",&dialog);
    QPushButton *wrongButton=new QPushButton("错题复习（仅错题本）",&dialog);
    QPushButton *cancelButton=new QPushButton("取消",&dialog);
    layout->addWidget(randomButton);
    layout->addWidget(wrongButton);
    layout->addWidget(cancelButton);

    bool shouldStart=false;
    bool wrongOnly=false;
    connect(randomButton,&QPushButton::clicked,[&]()
    {
        shouldStart=true;
        wrongOnly=false;
        dialog.accept();
    });
    connect(wrongButton,&QPushButton::clicked,[&]()
    {
        if(getWrongBookWordList().isEmpty())
        {
            QMessageBox messageBox(&dialog);
            messageBox.setWindowTitle("提示");
            messageBox.setText("错题本为空，无法进行错题复习");
            messageBox.setIcon(QMessageBox::Information);
            messageBox.setStandardButtons(QMessageBox::Ok);
            messageBox.setStyleSheet(lightDialogStyle());
            messageBox.exec();
            return;
        }
        shouldStart=true;
        wrongOnly=true;
        dialog.accept();
    });
    connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::reject);

    dialog.exec();
    if(shouldStart)
        startQuiz(wrongOnly);
}

void MainWindow::showQuizHistoryDialog()
{
    if(m_quizHistoryList.isEmpty())
    {
        QMessageBox messageBox(this);
        messageBox.setWindowTitle("测验历史");
        messageBox.setText("暂无测验记录");
        messageBox.setIcon(QMessageBox::Information);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setStyleSheet(lightDialogStyle());
        messageBox.exec();
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("测验历史");
    dialog.resize(640,420);
    dialog.setStyleSheet(lightDialogStyle());
    QVBoxLayout *layout=new QVBoxLayout(&dialog);
    QLabel *tipLabel=new QLabel("双击记录查看详情",&dialog);
    layout->addWidget(tipLabel);
    QListWidget *historyList=new QListWidget(&dialog);
    historyList->setAlternatingRowColors(true);
    layout->addWidget(historyList,1);

    QList<QuizRecord> records=m_quizHistoryList;
    std::reverse(records.begin(),records.end());
    for(const QuizRecord &record:records)
    {
        QString text=QString("%1  %2  得分：%3  正确：%4/%5  错词：%6")
                .arg(record.TestTime)
                .arg(record.TestRange)
                .arg(record.Score,0,'f',1)
                .arg(record.CorrectCount)
                .arg(record.TotalQuestions)
                .arg(record.ErrorWords.size());
        QListWidgetItem *item=new QListWidgetItem(text,historyList);
        item->setData(Qt::UserRole,record.TestTime);
    }

    QDialogButtonBox *buttonBox=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);
    connect(buttonBox,&QDialogButtonBox::rejected,&dialog,&QDialog::accept);
    layout->addWidget(buttonBox);

    connect(historyList,&QListWidget::itemDoubleClicked,[&](QListWidgetItem *item)
    {
        QString testTime=item->data(Qt::UserRole).toString();
        for(const QuizRecord &record:m_quizHistoryList)
        {
            if(record.TestTime!=testTime)
                continue;
            QString detail=QString("测验时间：%1\n测验范围：%2\n总题数：%3\n答对：%4\n得分：%5\n评级：%6\n")
                    .arg(record.TestTime)
                    .arg(record.TestRange)
                    .arg(record.TotalQuestions)
                    .arg(record.CorrectCount)
                    .arg(record.Score,0,'f',1)
                    .arg(getRating(record.Score));
            if(record.ErrorWords.isEmpty())
            {
                detail+="\n错词列表：无";
            }
            else
            {
                detail+="\n错词列表：\n";
                for(const QString &word:record.ErrorWords)
                    detail+="- "+word+"\n";
            }
            QMessageBox detailBox(&dialog);
            detailBox.setWindowTitle("测验成绩单");
            detailBox.setText(detail);
            detailBox.setIcon(QMessageBox::Information);
            detailBox.setStandardButtons(QMessageBox::Ok);
            detailBox.setStyleSheet(lightDialogStyle());
            detailBox.exec();
            break;
        }
    });

    dialog.exec();
}

void MainWindow::on_actionQuizHistory_triggered()
{
    showQuizHistoryDialog();
}
