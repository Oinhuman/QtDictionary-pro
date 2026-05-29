#include "rbtree.h"

RBTree::RBTree()
{
    root=NULL;
}

RBTree::~RBTree()
{
    this->destroy();
}

// 从根节点开始查找英文单词。
RBTreeNode *RBTree::iterativeSearch(QString EnglishWord)
{
    return   iterativeSearch(root, EnglishWord);
}

// 非递归二叉搜索：按 QString 字典序在左右子树间移动。
RBTreeNode *RBTree::iterativeSearch(RBTreeNode *x, QString EnglishWord) const
{
    while ((x!=NULL) && (x->EnglishWord!=EnglishWord))
      {
          if (EnglishWord < x->EnglishWord)
               x = x->left;
           else
               x = x->right;
      }

      return x;
}

bool RBTree::insert(QString EnglishWord, QString PhoneticWord, QString ChinaWord)
{
       RBTreeNode *z=NULL;
       // new 失败时返回 false，避免上层误以为词条已插入。
       if ((z=new RBTreeNode(EnglishWord)) == NULL)
            return false;
       z->PhoneticWord=PhoneticWord;
       z->ChineseWord=ChinaWord;
       insert(z);
       return true;
}

bool RBTree::remove(QString EnglishWord)
{
       RBTreeNode *node=NULL;
      // 先定位英文单词对应的结点，找不到说明词库中没有该词。
     if ((node = iterativeSearch(root, EnglishWord)) == NULL)
             return false;

    remove(node);
    return true;
}

void RBTree::destroy()
{
    this->destroy(root);
}

// 非递归中序遍历，输出结果天然按英文单词升序排列。
void RBTree::inorder(QStringList &Englishwords,QStringList &Phoneticwords,QStringList &Chinesewords)
{
    QStack<RBTreeNode*> s;
    RBTreeNode *p=root;
    while(p!=NULL||!s.empty())
    {
        while(p!=NULL)
        {
           s.push(p);
           p=p->left;
        }

        if(!s.empty())
        {
           p=s.top();
           Englishwords.append(p->EnglishWord);
           Phoneticwords.append(p->PhoneticWord);
           Chinesewords.append(p->ChineseWord);
           s.pop();
           p=p->right;
        }

    }

}

// 递归释放子树结点。
void RBTree::destroy(RBTreeNode *&tree)
{
    if (tree==NULL)
          return ;

    if (tree->left != NULL)
            return destroy(tree->left);
    if (tree->right != NULL)
           return destroy(tree->right);

    delete tree;
    tree=NULL;
}

void RBTree::leftRotate(RBTreeNode *x)
{
        // y 上升，x 下沉到 y 的左侧。
        RBTreeNode*y = x->right;

        // y 的左子树转接为 x 的右子树。
        x->right = y->left;
        if (y->left != NULL)
            y->left->parent = x;

        // y 接到 x 原来的父结点位置。
        y->parent = x->parent;

        if (x->parent == NULL)
        {
            root = y;
        }
        else
        {
            if (x->parent->left == x)
                x->parent->left = y;
            else
                x->parent->right = y;
        }

        // x 成为 y 的左孩子。
        y->left = x;
        x->parent = y;
}

void RBTree::rightRotate(RBTreeNode *y)
{
        // x 上升，y 下沉到 x 的右侧。
        RBTreeNode *x = y->left;

        // x 的右子树转接为 y 的左子树。
        y->left = x->right;
        if (x->right != NULL)
            x->right->parent = y;

        // x 接到 y 原来的父结点位置。
        x->parent = y->parent;

        if (y->parent == NULL)
        {
            root = x;
        }
        else
        {
            if (y == y->parent->right)
                y->parent->right = x;
            else
                y->parent->left = x;
        }

        // y 成为 x 的右孩子。
        x->right = y;

        y->parent = x;
}

void RBTree::insert(RBTreeNode *node)
{
    RBTreeNode *y = NULL;
    RBTreeNode *x = root;
        // 先按二叉搜索树规则找到新结点的父结点。
        while (x != NULL)
        {
            y = x;
            if (node->EnglishWord < x->EnglishWord)
                x = x->left;
            else
                x = x->right;
        }

        node->parent = y;
        if (y!=NULL)
        {
            if (node->EnglishWord < y->EnglishWord)
                y->left = node;
            else
                y->right = node;
        }
        else
        {
            root = node;
            root->Color=COLOR_BLACK;
        }

        // 新结点构造时默认为红色，插入后修正红黑树性质。
        insertFixup(node);
}
/*
 * 插入修正：
 * 父红时才会破坏红黑树性质；通过变色和旋转把红色冲突向上消解。
 */
void RBTree::insertFixup(RBTreeNode *node)
{
        RBTreeNode *parent, *gparent;
        parent=node->parent;

        // 只有父结点为红色时需要修复。
        while (node->parent!=NULL && node->parent->Color==COLOR_RED)
        {
            gparent = node->parent->parent;

            if(gparent==NULL)
                break;
            // 父结点在祖父左侧时，叔叔结点在祖父右侧。
            if (parent == gparent->left)
            {

                RBTreeNode *uncle = gparent->right;
                if (uncle && uncle->Color==COLOR_RED) // Case 1：叔叔为红，父叔变黑，祖父变红。
                {

                    uncle->Color=COLOR_BLACK;

                    parent->Color=COLOR_BLACK;

                    gparent->Color=COLOR_RED;
                    node = gparent;

                }
                else if (parent->right == node) // Case 2：内侧插入，先旋成外侧形态。
                {
                    RBTreeNode *tmp;
                    leftRotate(parent);
                    tmp = parent;
                    parent = node;
                    node = tmp;
                }
                else // Case 3：外侧插入，祖父旋转并交换父祖颜色。
                {


                         parent->Color=COLOR_BLACK;

                         gparent->Color=COLOR_RED;
                         rightRotate( gparent);
                }
            }
            else // 父结点在祖父右侧，处理逻辑与左侧对称。
            {
                  RBTreeNode *uncle = gparent->left;
                  if (uncle && uncle->Color==COLOR_RED)
                  {
                      uncle->Color=COLOR_BLACK;
                      parent->Color=COLOR_BLACK;
                      gparent->Color=COLOR_RED;
                      node = gparent;

                  }
                else  if (parent->left == node) // Case 2：内侧插入。
                {
                    RBTreeNode *tmp;
                    rightRotate( parent);
                    tmp = parent;
                    parent = node;
                    node = tmp;
                }
                else
                {
                      parent->Color=COLOR_BLACK;
                      gparent->Color=COLOR_RED;
                      leftRotate( gparent);
                }

            }
        }

        // 根结点必须保持黑色。
        root->Color=COLOR_BLACK;
}

void RBTree::remove(RBTreeNode *node)
{
       RBTreeNode *child, *parent;
       Color_t color;

       // 左右孩子都存在时，用中序后继替换被删结点。
       if ((node->left!=NULL) && (node->right!=NULL) )
       {
          RBTreeNode *replace = node;

           // 后继是右子树中最小的结点。
           replace = replace->right;
           while (replace->left != NULL)
               replace = replace->left;

           // 将后继结点接到被删结点原来的父结点位置。
           if (node!=root)
           {
               if (node->parent->left == node)
                   node->parent->left = replace;
               else
                   node->parent->right = replace;
           }
           else
               root = replace;

           // 后继没有左孩子，右孩子 child 是删除后可能需要修复的结点。
           child = replace->right;
           parent = replace->parent;
           // 保存后继原颜色；黑色被移走才需要修复。
           color = replace->Color;

           // 后继就是 node 的直接右孩子时，parent 需要指向 replace。
           if (parent == node)
           {
               parent = replace;
           }
           else
           {
               if (child!=NULL)
                   child->parent=parent;
               parent->left = child;

               replace->right = node->right;
               node->right->parent=replace;
           }

           replace->parent = node->parent;
           replace->Color= node->Color;
           replace->left = node->left;
           node->left->parent = replace;
           if (color == COLOR_BLACK)
                removeFixup(child, parent);
           delete node;
           return ;
       }
       if (node->left !=NULL)
           child = node->left;
       else
           child = node->right;

       parent = node->parent;
       // 单孩子或叶子删除：记录被删结点颜色。
       color = node->Color;

       if (child)
           child->parent = parent;

       // 把 child 接到 node 原来的父结点位置。
       if (parent)
       {
           if (parent->left == node)
               parent->left = child;
           else
               parent->right = child;
       }
       else
           root = child;
       if (color == COLOR_BLACK)
           removeFixup(child,parent);
       delete node;
}

// 删除修正：node 代表缺失黑高的位置，parent 是它当前父结点。
void RBTree::removeFixup(RBTreeNode *node,RBTreeNode *parent)
{
    RBTreeNode *other;

        while ((!node || node->Color==COLOR_BLACK) && node != root)
        {
            if (parent->left == node)
            {
                other = parent->right;
                if (other->Color==COLOR_RED)
                {
                    // Case 1：兄弟为红，先旋转转化为黑兄弟场景。
                    other->Color=COLOR_BLACK;
                    parent->Color=COLOR_RED;
                    leftRotate( parent);
                    other = parent->right;
                }
                if ((!other->left || other->left->Color==COLOR_BLACK) &&
                    (!other->right || other->right->Color==COLOR_BLACK))
                {
                    // Case 2：兄弟及其孩子全黑，兄弟变红，缺失黑高上移。
                    other->Color=COLOR_RED;
                    node = parent;
                    parent = node->parent;
                }
                else
                {
                    if (!other->right || other->right->Color==COLOR_BLACK)
                    {
                        // Case 3：远侄为黑，先旋转兄弟转化为 Case 4。
                        other->left->Color=COLOR_BLACK;
                        other->Color=COLOR_RED;
                        rightRotate( other);
                        other = parent->right;
                    }
                    // Case 4：远侄为红，旋转父结点后完成修复。
                    other->Color=parent->Color;
                    parent->Color=COLOR_BLACK;
                    other->right->Color=COLOR_BLACK;
                    leftRotate(parent);
                    node = root;
                    break;
                }
            }
            else
            {
                other = parent->left;
                if (other->Color==COLOR_RED)
                {
                    // Case 1：对称处理，兄弟为红。
                    other->Color=COLOR_BLACK;
                    parent->Color=COLOR_RED;
                    rightRotate( parent);
                    other = parent->left;
                }
                if ((!other->left || other->left->Color==COLOR_BLACK) &&
                    (!other->right || other->right->Color==COLOR_BLACK))
                {
                    // Case 2：兄弟及其孩子全黑。
                    other->Color=COLOR_RED;
                    node = parent;
                    parent =node->parent;
                }
                else
                {
                    if (!other->left || other->left->Color==COLOR_BLACK)
                    {
                        // Case 3：近侄为红，远侄为黑。
                        other->right->Color=COLOR_BLACK;
                        other->Color=COLOR_RED;
                        leftRotate(other);
                        other = parent->left;
                    }
                    // Case 4：远侄为红，旋转父结点后结束。
                    other->Color=parent->Color;
                    parent->Color=COLOR_BLACK;
                    other->left->Color=COLOR_BLACK;
                    rightRotate(parent);
                    node = root;
                    break;
                }
            }
        }
        if (node)
            node->Color=COLOR_BLACK;
}
