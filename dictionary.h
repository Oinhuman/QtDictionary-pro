#ifndef DICTIONARY_H
#define DICTIONARY_H
#include"rbtree.h"
#include<QFile>
#include<QTextStream>
#include<QDebug>
#include<QStringList>

// 词库模型：以红黑树提供快速查找，以三个顺序列表服务 UI 展示和补全。
class Dictionary
{
public:
    // 构造时读取词库文件；析构时写回当前保存路径。
    Dictionary(QString path);
    ~Dictionary();

    bool insert(QString Englishword,QString Chineseword);   // 插入无音标词条
    bool insert(QString Englishword,QString Phoneticword,QString Chineseword); // 插入完整词条，重复词返回 false
    int insertFile(QString path);                           // 批量导入词库文件，返回新增数量
    bool remove(QString Englishword);                       // 删除词条，并同步顺序列表
    bool update(QString Englishword,QString Chineseword);   // 修改释义，保留原音标
    bool update(QString Englishword,QString Phoneticword,QString Chineseword); // 修改音标和释义
    QString Find(QString Englishword);                      // 查找中文释义，找不到返回空串
    QString FindPhonetic(QString Englishword);              // 查找音标，找不到返回空串
    bool saveToFile();                                      // 保存到当前词库文件
    bool saveToFile(const QString &path);                   // 保存到指定词库文件
    QString getSavePath() const { return savePath; }        // 当前词库文件路径

    void setWords();                                        // 用中序遍历重建 UI 所需的顺序列表
    QStringList Englishwords;                               // 英文词表，供补全、列表和练习抽题使用
    QStringList Phoneticwords;                              // 与 Englishwords 同下标的音标
    QStringList Chinesewords;                               // 与 Englishwords 同下标的中文释义
protected:
private:
    RBTree *DicTree;   // 词条索引树，key 为英文单词
    QString savePath;  // 当前词库写回路径

};

#endif // DICTIONARY_H
