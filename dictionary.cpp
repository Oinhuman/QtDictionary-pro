#include "dictionary.h"

static const QString WordlistHeader=QStringLiteral("# QtDictionary wordlist v2");

//字典的数据从path中读取，也就是说字典主要的数据都存在path对应的txt中，
//其后加入的单词也都会存储在里面，删除单词也将删除掉里面的单词
//其实最好的办法还是使用数据库
Dictionary::Dictionary(QString path)
{

    QFile file(path);
    savePath=path;
    DicTree=new RBTree;
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return;
    QTextStream in(&file);
    QString English;
    QString Phonetic;
    QString Chinese;
    QString firstLine;
    bool hasFirstLine=false;
    bool hasPhonetic=false;
    if(!in.atEnd())
    {
        firstLine=in.readLine().trimmed();
        hasPhonetic=(firstLine==WordlistHeader);
        hasFirstLine=!hasPhonetic;
    }

    while(hasFirstLine || !in.atEnd())
    {

       English=hasFirstLine ? firstLine : in.readLine().trimmed();
       hasFirstLine=false;
       if(in.atEnd())
           break;
       if(hasPhonetic)
       {
           Phonetic=in.readLine().trimmed();
           if(in.atEnd())
               break;
       }
       else
       {
           Phonetic.clear();
       }
       Chinese=in.readLine().trimmed();
       if(English.isEmpty())
           continue;
       Englishwords.append(English);
       Phoneticwords.append(Phonetic);
       Chinesewords.append(Chinese);
       DicTree->insert(English,Phonetic,Chinese);

    }
    file.close();

}
//析构函数会对单词进行储存，储存在读进来的文本当中
Dictionary::~Dictionary()
{
    //写入方式有待改进
    this->setWords();
    QFile file(savePath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        delete DicTree;
        return;
    }
    QTextStream out(&file);
    out<<WordlistHeader<<Qt::endl;
    for(int i=0;i<Englishwords.size();i++)
    {

      out<<Englishwords[i];
      out<<Qt::endl;
      out<<(i<Phoneticwords.size() ? Phoneticwords[i] : QString());
      out<<Qt::endl;
      out<<Chinesewords[i];
      out<<Qt::endl;
    }
    file.close();
    delete DicTree;
}

//插入函数，一个通过单个单词插入，一个通过文件插入,需要考虑重复插入
//需要注意的是出入进去的单词要最终写在txt上
bool Dictionary::insert(QString Englishword, QString Chineseword)
{
    return insert(Englishword,QString(),Chineseword);
}

bool Dictionary::insert(QString Englishword, QString Phoneticword, QString Chineseword)
{
   Englishword=Englishword.trimmed();
   Phoneticword=Phoneticword.trimmed();
   Chineseword=Chineseword.trimmed();
   if(Englishword.isEmpty() || Chineseword.isEmpty())
       return false;
   if(DicTree->iterativeSearch(Englishword)!=NULL)
       return false;
   if(DicTree->insert(Englishword,Phoneticword,Chineseword))
   {
       Englishwords.append(Englishword);
       Phoneticwords.append(Phoneticword);
       Chinesewords.append(Chineseword);
       return true;
   }
   return false;
}
//文本插入
int Dictionary::insertFile(QString path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return 0;
    QTextStream in(&file);
    QString English;
    QString Phonetic;
    QString Chinese;
    QString firstLine;
    int count=0;
    bool hasFirstLine=false;
    bool hasPhonetic=false;
    if(!in.atEnd())
    {
        firstLine=in.readLine().trimmed();
        hasPhonetic=(firstLine==WordlistHeader);
        hasFirstLine=!hasPhonetic;
    }

    while(hasFirstLine || !in.atEnd())
    {

       English=hasFirstLine ? firstLine : in.readLine().trimmed();
       hasFirstLine=false;
       if(in.atEnd())
           break;
       if(hasPhonetic)
       {
           Phonetic=in.readLine().trimmed();
           if(in.atEnd())
               break;
       }
       else
       {
           Phonetic.clear();
       }
       Chinese=in.readLine().trimmed();
       if(insert(English,Phonetic,Chinese))
           count++;

    }
    file.close();
    return count;
}
//删除单词
bool Dictionary::remove(QString Englishword)
{
    Englishword=Englishword.trimmed();
    if(DicTree->remove(Englishword))
    {
        int index=Englishwords.indexOf(Englishword);
        if(index!=-1)
        {
            Englishwords.removeAt(index);
            if(index<Phoneticwords.size())
                Phoneticwords.removeAt(index);
            Chinesewords.removeAt(index);
        }
        return true;
    }
    return false;
}
//更新单词
bool Dictionary::update(QString Englishword, QString Chineseword)
{
    return update(Englishword,FindPhonetic(Englishword),Chineseword);
}

bool Dictionary::update(QString Englishword, QString Phoneticword, QString Chineseword)
{
     Englishword=Englishword.trimmed();
     Phoneticword=Phoneticword.trimmed();
     Chineseword=Chineseword.trimmed();
     RBTreeNode *node=DicTree->iterativeSearch(Englishword);
     if(node==NULL)
       return false;

    node->PhoneticWord=Phoneticword;
    node->ChineseWord=Chineseword;
    int index=Englishwords.indexOf(Englishword);
    if(index!=-1 && index<Phoneticwords.size())
        Phoneticwords[index]=Phoneticword;
    if(index!=-1)
        Chinesewords[index]=Chineseword;
    return true;
}
//查询
QString Dictionary::Find(QString Englishword)
{
    Englishword=Englishword.trimmed();
    RBTreeNode *node=DicTree->iterativeSearch(Englishword);
    if(node==NULL)
      return QString();
    else
        return node->ChineseWord;
}
//查询音标
QString Dictionary::FindPhonetic(QString Englishword)
{
    Englishword=Englishword.trimmed();
    RBTreeNode *node=DicTree->iterativeSearch(Englishword);
    if(node==NULL)
      return QString();
    else
        return node->PhoneticWord;
}
//得到英文和中文集合
void Dictionary::setWords()
{
    Englishwords.clear();
    Phoneticwords.clear();
    Chinesewords.clear();
    DicTree->inorder(Englishwords,Phoneticwords,Chinesewords);
}








