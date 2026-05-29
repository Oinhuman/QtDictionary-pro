#ifndef RBTREE_H
#define RBTREE_H
#include<QString>
#include<QStack>
#include<QStringList>
#include<QList>
enum Color_t
{
    COLOR_BLACK,
    COLOR_RED
};


// 红黑树结点：英文单词作为排序键，音标和中文释义作为载荷。
struct RBTreeNode
{
    RBTreeNode(QString data)
    {
        left=NULL;
        right=NULL;
        parent=NULL;
        Color=COLOR_RED;
        EnglishWord=data;
    }
    RBTreeNode *left,*right,*parent;
    Color_t Color;
    QString EnglishWord;    //英语单词
    QString PhoneticWord;   //音标
    QString ChineseWord;      //中文意思
};

// 红黑树索引：支撑词库查找、插入、删除和有序遍历。
class RBTree
{

public:
    RBTree();
   ~RBTree();

    // 非递归查找英文单词对应的结点；找不到返回 NULL。
    RBTreeNode* iterativeSearch(QString EnglishWord);
    // 预留接口：查找大于 x 的最小结点。
    RBTreeNode* successor(RBTreeNode*x);
    // 预留接口：查找小于 x 的最大结点。
    RBTreeNode* predecessor(RBTreeNode *x);
    // 插入词条；英文单词作为排序键。
    bool insert(QString EnglishWord,QString PhoneticWord,QString ChinaWord);
    // 删除指定英文单词。
    bool remove(QString EnglishWord);
    // 销毁整棵树。
    void destroy();
    // 中序遍历输出有序的英文、音标、中文列表。
    void inorder(QStringList &Englishwords,QStringList &Phoneticwords,QStringList &Chinesewords);
protected:
    // 左旋：以 x 为支点，将右子树提升。
    void leftRotate(RBTreeNode* x);
    // 右旋：以 y 为支点，将左子树提升。
    void rightRotate(RBTreeNode* y);
    // 按二叉搜索树规则挂接新结点。
   void insert(RBTreeNode* node);
    // 插入后修复红黑树颜色和结构。
    void insertFixup(RBTreeNode* node);
    // 删除已定位的结点。
    void remove(RBTreeNode *node);
    // 删除黑结点后修复红黑树平衡。
    void removeFixup(RBTreeNode* node,RBTreeNode *parent);

    // 从指定子树开始非递归查找。
    RBTreeNode* iterativeSearch(RBTreeNode* x, QString EnglishWord) const;
    // 递归释放子树。
     void destroy(RBTreeNode* &tree);

private:
    RBTreeNode *root;       // 根节点

};

#endif // RBTREE_H
