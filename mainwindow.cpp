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
#include<QHBoxLayout>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QRandomGenerator>
#include<QStandardPaths>
#include<QStatusBar>
#include<QStringListModel>
#include<QToolBar>
#include<QToolButton>
#include<QUrl>
#include<QVBoxLayout>
#include<QPixmap>

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
    rememberWrongOnly=false;
    rememberDone=0;
    rememberRight=0;
    i=-1;

    //接受单词详细界面传来的英文单词，进行读
    connect(wordWidget,&ShowWord::SpeakEnglis,
            [=](QString e)
    {
        Textspeak->say(e);
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
    initNavigation();
    initAppStyle();
}

MainWindow::~MainWindow()
{

    saveWrongBook();
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

    ui->mainToolBar->addAction(ui->actionFind);
    ui->mainToolBar->addAction(ui->actionHistory);
    ui->mainToolBar->addAction(ui->actionall);
    ui->mainToolBar->addAction(ui->actionRemember);
    ui->mainToolBar->addAction(actionWrongBook);
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
        "QTextEdit{font-size:18px;}"
        "QLabel{color:#1f2937;}"
        "QStatusBar{background:#f8fafc;color:#475569;}"
    );
}

//初始化背单词界面，让键盘操作更顺手
void MainWindow::initRememberPage()
{
    rememberTipLabel=new QLabel("输入答案后按回车提交，答错会自动进入错题本。",this);
    rememberTipLabel->setWordWrap(true);
    rememberTipLabel->setStyleSheet("color:#777;padding:4px 0;");

    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(ui->stackedWidget->widget(4)->layout());
    if(layout!=NULL)
        layout->insertWidget(3,rememberTipLabel);

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
            Textspeak->say(item->data(Qt::UserRole).toString());
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
    rememberLastTip=wrongOnly ? "当前为错题复习，答对会降低错题次数。" : "当前为随机练习，答错会自动进入错题本。";
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

    QStringList candidates;
    if(rememberWrongOnly)
    {
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
    }
    else
    {
        i=QRandomGenerator::global()->bounded(myDictionary->Englishwords.size());
    }

    QString e=myDictionary->Englishwords[i];
    ui->labelShow->setText(makeRememberQuestion(e));
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
        QString p=myDictionary->FindPhonetic(Englishword);
        QString c=myDictionary->Find(Englishword);
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
    QString p=myDictionary->FindPhonetic(e);
    QString c=myDictionary->Find(e);
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
        if(!path.isEmpty())//如果路径不为空执行文本插入
        {
            int count=myDictionary->insertFile(path);
            if(model!=NULL && count>0)
                model->setStringList(myDictionary->Englishwords);
            if(count>0)
                QMessageBox::information(this,"提示","文本导入成功，共导入 "+QString::number(count)+" 个单词");
            else
                QMessageBox::information(this,"提示","未导入新单词，请检查文件或重复单词");
        }

        if(!e.isEmpty() && !c.isEmpty() && myDictionary->insert(e,c))
        {
            if(model!=NULL)
                model->setStringList(myDictionary->Englishwords);
            QMessageBox::information(this,"提示","插入成功");
        }
        else if(!e.isEmpty() || !c.isEmpty())
            QMessageBox::information(this,"提示","插入失败");
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
               item.ChineseWord=c;
               wrongBook[e]=item;
               saveWrongBook();
               refreshWrongBook();
           }
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
    ui->listWidget->addItems(myDictionary->Englishwords);
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
   QString c=myDictionary->Find(e);
   if(!c.isEmpty())
   {
       QString p=myDictionary->FindPhonetic(e);
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

     Textspeak->say(e);
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
    bool correct=answer.compare(rightWord,Qt::CaseInsensitive)==0;
    rememberDone++;
    if(correct)
    {
        rememberRight++;
        if(wrongBook.contains(rightWord))
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
        addWrongWord(rightWord,answer);
        rememberLastTip="上一题错误，正确答案："+rightWord+"，已加入错题本。";
    }

    playRememberSound(correct);
    showNextRememberWord();

}
//提示中文
void MainWindow::on_Buttonhint_clicked()
{
    if(i<0 || i>=myDictionary->Chinesewords.size())
        return;
    QString c=myDictionary->Chinesewords[i];
    ui->lineEditHint->setText(c);
}
//对要背的英语单词进行读
void MainWindow::on_ButtonReadHint_clicked()
{
    if(i<0 || i>=myDictionary->Englishwords.size())
        return;
    QString e=myDictionary->Englishwords[i];
    Textspeak->say(e);
}

//逐条浏览时点出单词的详细界面
void MainWindow::on_listWidget_itemPressed(QListWidgetItem *item)
{
    QString e=item->text();
    QString p=myDictionary->FindPhonetic(e);
    QString c=myDictionary->Find(e);
    wordWidget->display(e,p,c);
    wordWidget->show();
}

//查看历史时点出单词的详细界面
void MainWindow::on_listWidgetHistory_itemPressed(QListWidgetItem *item)
{
    QString e=item->text();
    QString p=myDictionary->FindPhonetic(e);
    QString c=myDictionary->Find(e);
    wordWidget->display(e,p,c);
    wordWidget->show();
}

//设置时进行查看,可以查看单词的主要信息
void MainWindow::on_Button_3_clicked()
{
    QString e=ui->textEditEnglish->toPlainText().trimmed();
    QString p=myDictionary->FindPhonetic(e);
    QString c=myDictionary->Find(e);
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
        rememberLastTip="已跳过："+myDictionary->Englishwords[i];
    showNextRememberWord();
}
