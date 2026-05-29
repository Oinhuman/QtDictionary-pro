#include "dictionary.h"

// v2 词库首行标记：有该标记时按“英文/音标/中文”三行一组解析。
static const QString WordlistHeader=QStringLiteral("# QtDictionary wordlist v2");

// 从 path 读取词库。旧格式为“英文/中文”两行一组；v2 格式额外包含音标。
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

    // hasFirstLine 用于兼容旧格式：首行不是版本头时，它就是第一个英文单词。
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

// 析构时写回当前词库文件，确保增删改不会只停留在内存里。
Dictionary::~Dictionary()
{
    saveToFile();
    delete DicTree;
}

// 保存到构造时记录的词库路径。
bool Dictionary::saveToFile()
{
    if(savePath.isEmpty())
        return false;
    return saveToFile(savePath);
}

// 保存为 v2 格式；写入前先按红黑树中序遍历重建顺序列表。
bool Dictionary::saveToFile(const QString &path)
{
    this->setWords();
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        return false;
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
    return true;
}

// 插入无音标词条，委托完整插入接口处理去重和修剪。
bool Dictionary::insert(QString Englishword, QString Chineseword)
{
    return insert(Englishword,QString(),Chineseword);
}

// 插入完整词条：英文和释义必须存在，重复英文直接拒绝。
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

// 批量导入文件；支持 v2 和旧格式，返回成功插入的新增词条数。
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

    // 逐条复用 insert，保证批量导入和单词插入的去重规则一致。
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

// 删除单词：树删除成功后，再同步三个顺序列表。
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

// 只改释义时保留原音标。
bool Dictionary::update(QString Englishword, QString Chineseword)
{
    return update(Englishword,FindPhonetic(Englishword),Chineseword);
}

// 更新完整词条：树节点和顺序列表必须同时更新，保持 UI 数据一致。
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

// 查找中文释义；未命中返回空串，由界面决定如何提示。
QString Dictionary::Find(QString Englishword)
{
    Englishword=Englishword.trimmed();
    RBTreeNode *node=DicTree->iterativeSearch(Englishword);
    if(node==NULL)
      return QString();
    else
        return node->ChineseWord;
}

// 查找音标；旧格式词条可能没有音标，返回空串属于正常情况。
QString Dictionary::FindPhonetic(QString Englishword)
{
    Englishword=Englishword.trimmed();
    RBTreeNode *node=DicTree->iterativeSearch(Englishword);
    if(node==NULL)
      return QString();
    else
        return node->PhoneticWord;
}

// 重建顺序列表；保存文件和 UI 补全都依赖这三个列表同下标对应。
void Dictionary::setWords()
{
    Englishwords.clear();
    Phoneticwords.clear();
    Chinesewords.clear();
    DicTree->inorder(Englishwords,Phoneticwords,Chinesewords);
}








