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

    // 导出 README 截图：自动切到核心页面并保存当前窗口画面。
    bool exportReadmeScreens(const QString &outputDir);

private slots:
    void on_buttonBox_accepted();              // 设置页确认：按当前模式导入、删除或更新词条

    void on_buttonBox_rejected();              // 设置页取消：返回查词页

    void on_actionall_triggered();             // 进入全部词条浏览页

    void on_actionFind_triggered();            // 进入查词页并清空当前输入

    void on_ButtonFind_clicked();              // 查询输入框中的英文单词

    void on_actionHistory_triggered();         // 显示本次运行的查词记录

    void on_actionset_triggered();             // 进入词库维护页

    void on_ButtonRead_clicked();              // 朗读当前查词输入

    void on_actionRemember_triggered();        // 开始随机背词

    void on_ButtonSure_clicked();              // 提交背词答案并记录结果

    void on_Buttonhint_clicked();              // 显示背词提示

    void on_ButtonReadHint_clicked();          // 朗读当前背词单词

    void on_ButtonNext_clicked();              // 跳过当前背词题

    void on_listWidget_itemPressed(QListWidgetItem *item);        // 全部词条页：打开选中单词详情

    void on_listWidgetHistory_itemPressed(QListWidgetItem *item); // 历史页：打开选中单词详情

    void on_Button_3_clicked();                // 设置页预览词条详情

    void on_ButtonFile_clicked();              // 选择待导入词库文本

    void updateTimer();                        // 测验倒计时，每秒触发一次

    void on_actionQuiz_triggered();            // 打开测验范围选择框

    void on_actionQuizHistory_triggered();     // 查看已保存的测验历史

    void on_ButtonSubmitAnswer_clicked();      // 提交当前测验题答案



private:
    // 错题本条目：保存释义、累计错误次数、最近错答和时间。
    struct WrongWord
    {
        QString ChineseWord;
        int Count;
        QString LastAnswer;
        QDateTime LastTime;
    };

    // 单道测验题：Type 1 为英译中，2 为中译英，3 为默写。
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

    // 一次测验成绩：用于持久化历史记录和弹窗复盘。
    struct QuizRecord
    {
        QString TestTime;
        QString TestRange;
        int TotalQuestions;
        int CorrectCount;
        double Score;
        QStringList ErrorWords;
    };

    void initNavigation();                 // 顶部一级导航，替代原菜单层级
    void initAppStyle();                   // 全局 Qt 样式表，统一控件观感
    void initRememberPage();               // 背词页控件补强：选项按钮、提示、回车提交
    void initWrongBookPage();              // 动态创建错题本页并接入 stackedWidget
    void startRemember(bool wrongOnly);    // 开始背词；wrongOnly 为 true 时只练错题
    void showNextRememberWord();           // 生成并展示下一道背词题
    QString makeRememberQuestion(QString word) const; // 将单词部分字母遮蔽成默写题面
    void addWrongWord(QString Englishword,QString answer); // 累加错题并立即持久化
    void loadWrongBook();                  // 从用户数据目录读取 wrong_words.json
    void saveWrongBook();                  // 将错题本写回 wrong_words.json
    void refreshWrongBook();               // 重绘错题列表和按钮可用状态
    void showWrongBookDetail(QListWidgetItem *item); // 打开错题详情窗口
    void playRememberSound(bool correct);  // 播放背词正误反馈音，缺音频时错误用蜂鸣兜底
    void initQuizPage();                   // 动态创建测验页
    void startQuiz(bool useWrongBookOnly); // 开始测验；可限定错题本范围
    void generateQuizQuestions();          // 生成本轮最多 15 道混合题
    void generateQuestionOfType(const QString &word,int type); // 按题型生成单题
    void showCurrentQuizQuestion();        // 渲染当前测验题并启动倒计时
    void evaluateAndRecord();              // 判定当前答案，写入题目结果
    void showQuizResult();                 // 汇总成绩、保存历史、提示错词
    void saveQuizHistory();                // 保存 quiz_history.json
    void loadQuizHistory();                // 读取 quiz_history.json
    void showQuizHistoryDialog();          // 显示测验历史及单次详情
    void showAnswerDialog(bool isCorrect,const QString &correctAnswer); // 显示单题正误，3 秒自动关闭
    void handleQuizTimeout();              // 超时视为答错并进入下一题
    void speakEnglish(QString word);       // 统一英文朗读入口，避免连续点击叠音
    QString getCurrentTimeString() const;  // 返回历史记录使用的时间文本
    QString getRating(double score) const; // 将百分制成绩映射为中文评级
    QStringList getWrongBookWordList() const; // 过滤仍存在于词库的错题词

    Ui::MainWindow *ui;
    Dictionary *myDictionary;   // 当前词库模型，负责查找、增删改和保存
    QCompleter *completer;      // 查词输入补全器，数据源来自英文词表
    QButtonGroup *group;        // 设置页单选组：插入、删除、修改
    QStringList  history;       // 本次运行内的查词历史
    QTextToSpeech *Textspeak;   // 英文朗读引擎
    int i;                      // 当前背词单词在词表中的下标
    ShowWord *wordWidget;       // 单词详情窗口，复用于列表、历史和错题详情
    QMediaPlayer *effectPlayer; // 背词正误反馈音播放器
    QAudioOutput *effectAudioOutput; // 反馈音输出设备
    QAction *actionWrongBook;   // 顶部错题本入口
    QWidget *wrongBookPage;     // 动态创建的错题本页面
    QLabel *rememberTipLabel;   // 背词页状态提示
    QLabel *wrongBookTipLabel;  // 错题本统计提示
    QListWidget *wrongBookList; // 错题列表，UserRole 存英文单词
    QPushButton *practiceWrongButton; // 开始错题复习
    QPushButton *masterWrongButton;   // 从错题本移除选中词
    QPushButton *clearWrongButton;    // 清空全部错题
    QPushButton *readWrongButton;     // 朗读选中错题
    QWidget *rememberOptionsWidget;   // 随机背词模式的 A-D 选项区
    QButtonGroup *m_rememberOptionsGroup; // 背词选项按钮组，id 对应 A-D
    QList<QPushButton*> m_rememberOptionButtons; // 背词选项按钮缓存
    QMap<QString,WrongWord> wrongBook; // 个性化错题本，key 为英文单词
    QString wrongBookPath;     // 错题本 JSON 保存路径
    QString rememberLastTip;   // 上一题反馈文本
    bool rememberWrongOnly;    // 当前是否为错题复习模式
    int rememberDone;          // 本轮已答题数
    int rememberRight;         // 本轮答对数
    QString rememberCurrentCorrectAnswer; // 当前选择题标准答案
    QStringList rememberCurrentOptions;   // 当前选择题 A-D 选项文本
    int rememberCurrentQuestionType;      // 当前背词题型：0 英译中，1 中译英

    QTimer *m_quizTimer;       // 测验倒计时定时器
    int m_currentTimeLeft;     // 当前题剩余秒数
    QWidget *m_quizPage;       // 动态创建的测验页面
    QLabel *m_quizProgressLabel; // 测验进度文本
    QLabel *m_timerLabel;      // 倒计时文本
    QLabel *m_quizQuestionLabel; // 当前测验题面
    QGroupBox *m_quizOptionsGroup; // 选择题选项容器
    QButtonGroup *m_quizRadioGroup; // 测验选项互斥组
    QList<QRadioButton*> m_quizRadioButtons; // 测验 A-D 单选按钮
    QLineEdit *m_quizInputEdit; // 默写题输入框
    QPushButton *m_quizSubmitButton; // 测验提交按钮
    QList<QuizQuestion> m_quizQuestions; // 本轮测验题目缓存
    QList<QuizRecord> m_quizHistoryList; // 已读取的测验历史
    QString m_quizHistoryPath; // 测验历史 JSON 保存路径
    int m_currentQuizIndex;    // 当前测验题下标
    int m_quizCorrectCount;    // 本轮答对数量
    QString m_quizRangeName;   // 测验范围名称，用于成绩单
    bool m_quizUseWrongBookOnly; // 当前测验是否只取错题

};

#endif // MAINWINDOW_H
