#ifndef DICTIONARY_H
#define DICTIONARY_H
#include"rbtree.h"
#include<QFile>
#include<QTextStream>
#include<QDebug>
#include<QStringList>
class Dictionary
{
public:
    //构造和析构函数
    Dictionary(QString path);
    ~Dictionary();

    bool insert(QString Englishword,QString Chineseword);   //插入单词
    bool insert(QString Englishword,QString Phoneticword,QString Chineseword); //插入带音标的单词
    int insertFile(QString path);                           //插入文本文件
    bool remove(QString Englishword);                       //删除单词
    bool update(QString Englishword,QString Chineseword);   //修改单词
    bool update(QString Englishword,QString Phoneticword,QString Chineseword); //修改带音标的单词
    QString Find(QString Englishword);                      //寻找单词
    QString FindPhonetic(QString Englishword);              //寻找音标
    bool saveToFile();                                      //保存到当前词库文件
    bool saveToFile(const QString &path);                   //保存到指定词库文件
    QString getSavePath() const { return savePath; }        //当前词库文件路径

    void setWords();                                        //得到英文、音标、中文集合，利用了中序遍历
    QStringList Englishwords;
    QStringList Phoneticwords;
    QStringList Chinesewords;
protected:
private:
    RBTree *DicTree;   //红黑树，字典需要用到
    QString savePath;   //最后单词存储路径

};

#endif // DICTIONARY_H
