#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"dictionary.h"
#include<QCompleter>
#include<QButtonGroup>
#include<QStringList>
#include<QTextToSpeech>
#include<QListWidgetItem>
#include<QListWidget>
#include<QMediaPlayer>
#include<QAudioOutput>
#include<QDateTime>
#include<QMap>
#include<QAction>
#include<QLabel>
#include<QPushButton>
#include<QDialogButtonBox>
#include<QGroupBox>
#include<QLineEdit>
#include<QRadioButton>
#include<QTimer>
#include"showword.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool exportReadmeScreens(const QString &outputDir);

private slots:
    void on_buttonBox_accepted();//设置中的确定按键，ok

    void on_buttonBox_rejected();//设置中的取消案件，会返回查询界面

    void on_actionall_triggered();//查询所有单词，进行逐条查询

    void on_actionFind_triggered();//用于跳到查询

    void on_ButtonFind_clicked();//填写好英文单词后的的确定，接着会查询

    void on_actionHistory_triggered();//跳转到历史记录界面

    void on_actionset_triggered();//跳转到设置界面

    void on_ButtonRead_clicked();//用于查询界面的发出声音，即读单词

    void on_actionRemember_triggered();//跳转到记单词界面

    void on_ButtonSure_clicked();//用于记单词界面的确定按键

    void on_Buttonhint_clicked();//用于记单词界面的提示按键

    void on_ButtonReadHint_clicked();//记单词中读单词按键

    void on_ButtonNext_clicked();//用于记单词界面选择下一个单词

    void on_listWidget_itemPressed(QListWidgetItem *item);//用于可以在所有单词界面中点击单词就可以查询

    void on_listWidgetHistory_itemPressed(QListWidgetItem *item);//用于可以在历史记录界面点单词就可以查询

    void on_Button_3_clicked();//用于设置界面查看单词

    void on_ButtonFile_clicked();//用于设置界面选择文本

    void updateTimer(); //测验倒计时

    void on_actionQuiz_triggered(); //开始测验

    void on_actionQuizHistory_triggered(); //查看测验历史

    void on_ButtonSubmitAnswer_clicked(); //提交测验答案



private:
    struct WrongWord
    {
        QString ChineseWord;
        int Count;
        QString LastAnswer;
        QDateTime LastTime;
    };

    struct QuizQuestion
    {
        QString English;
        QString Chinese;
        int Type;
        QString CorrectAnswer;
        QString UserAnswer;
        QStringList Options;
        bool IsCorrect;
    };

    struct QuizRecord
    {
        QString TestTime;
        QString TestRange;
        int TotalQuestions;
        int CorrectCount;
        double Score;
        QStringList ErrorWords;
    };

    void initNavigation();                 //初始化顶部一级功能按钮
    void initAppStyle();                   //统一界面样式
    void initRememberPage();               //初始化背单词界面
    void initWrongBookPage();              //初始化错题本界面
    void startRemember(bool wrongOnly);    //开始背单词
    void showNextRememberWord();           //显示下一个要背的单词
    QString makeRememberQuestion(QString word) const; //生成背单词题面
    void addWrongWord(QString Englishword,QString answer); //加入错题本
    void loadWrongBook();                  //读取错题本
    void saveWrongBook();                  //保存错题本
    void refreshWrongBook();               //刷新错题本界面
    void showWrongBookDetail(QListWidgetItem *item); //显示错题详情
    void playRememberSound(bool correct);  //播放背词反馈音
    void initQuizPage();                   //初始化测验界面
    void startQuiz(bool useWrongBookOnly); //开始测验
    void generateQuizQuestions();          //生成测验题目
    void generateQuestionOfType(const QString &word,int type); //生成单题
    void showCurrentQuizQuestion();        //显示当前测验题
    void evaluateAndRecord();              //判定当前答案
    void showQuizResult();                 //显示测验结果
    void saveQuizHistory();                //保存测验历史
    void loadQuizHistory();                //读取测验历史
    void showQuizHistoryDialog();          //显示测验历史
    void showAnswerDialog(bool isCorrect,const QString &correctAnswer); //显示单题结果
    void handleQuizTimeout();              //处理超时
    void speakEnglish(QString word);       //优化英文朗读
    QString getCurrentTimeString() const;  //当前时间文本
    QString getRating(double score) const; //成绩评级
    QStringList getWrongBookWordList() const; //错题词列表

    Ui::MainWindow *ui;
    Dictionary *myDictionary;   //字典类
    QCompleter *completer;      //自动补全，实现模糊查询
    QButtonGroup *group;        //用于单选框
    QStringList  history;       //用于历史记录
    QTextToSpeech *Textspeak;   //用于发声音
    int i;//用于背单词
    ShowWord *wordWidget;       //显示详细单词用，在所有单词，历史界面中都有用
    QMediaPlayer *effectPlayer; //背单词时的反馈音效
    QAudioOutput *effectAudioOutput; //反馈音效输出
    QAction *actionWrongBook;   //错题本一级入口
    QWidget *wrongBookPage;     //错题本页面
    QLabel *rememberTipLabel;   //背单词提示
    QLabel *wrongBookTipLabel;  //错题本统计提示
    QListWidget *wrongBookList; //错题列表
    QPushButton *practiceWrongButton; //复习错题
    QPushButton *masterWrongButton;   //标记掌握
    QPushButton *clearWrongButton;    //清空错题
    QPushButton *readWrongButton;     //朗读错题
    QWidget *rememberOptionsWidget;   //背单词选择题选项区域
    QButtonGroup *m_rememberOptionsGroup; //背单词选择题按钮组
    QList<QPushButton*> m_rememberOptionButtons; //背单词选项按钮
    QMap<QString,WrongWord> wrongBook;//个性化错题本
    QString wrongBookPath;     //错题本保存路径
    QString rememberLastTip;   //上一题反馈
    bool rememberWrongOnly;    //是否只练错题
    int rememberDone;          //本轮已做题数
    int rememberRight;         //本轮正确数
    QString rememberCurrentCorrectAnswer; //当前选择题正确答案
    QStringList rememberCurrentOptions;   //当前选择题选项
    int rememberCurrentQuestionType;      //当前选择题类型

    QTimer *m_quizTimer;       //测验倒计时
    int m_currentTimeLeft;     //当前题剩余秒数
    QWidget *m_quizPage;       //测验页面
    QLabel *m_quizProgressLabel; //测验进度
    QLabel *m_timerLabel;      //倒计时显示
    QLabel *m_quizQuestionLabel; //测验题面
    QGroupBox *m_quizOptionsGroup; //选择题区域
    QButtonGroup *m_quizRadioGroup; //测验选项组
    QList<QRadioButton*> m_quizRadioButtons; //测验选项按钮
    QLineEdit *m_quizInputEdit; //默写输入框
    QPushButton *m_quizSubmitButton; //提交测验答案
    QList<QuizQuestion> m_quizQuestions; //本次测验题
    QList<QuizRecord> m_quizHistoryList; //测验历史
    QString m_quizHistoryPath; //测验历史路径
    int m_currentQuizIndex;    //当前题号
    int m_quizCorrectCount;    //答对数量
    QString m_quizRangeName;   //测验范围名称
    bool m_quizUseWrongBookOnly; //是否只测错题

};

#endif // MAINWINDOW_H
