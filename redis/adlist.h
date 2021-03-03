//链表结构

#ifndef __ADLIST_H_
#define __ADLIST_H_

//链表结点
typedef struct listNode
{
    struct listNode *prev; //前去结点
    struct listNode *next; //后继结点
    void *value; //万能指针，存放信息
} listNode;

//迭代器
typedef struct listIter
{
    listNode *next; //迭代器当前指向的结点
    int direction; //迭代方向，AL_START_HEAD和AL_START_TAIL
} listIter;

//链表表头
typedef struct list
{
    listNode *head; //链表头结点指针
    listNode *tail; //链表尾结点指针

    void *(*dup)(void *ptr); //复制链表结点保存的值
    void (*free)(void *ptr); //释放链表结点保存的值
    void (*match)(void *ptr,void *key); //比较链表结点所保存的结点值和另一key是否相等

    unsigned long len; //链表长度计数器
} list;

#define listLength(l) ((l)->len) //返回链表l结点的数量
#define listFirst(l) ((l)->head) //返回链表l的头结点地址
#define listLast(l) ((l)->tail) //返回链表l得到尾结点的地址
#define listPrevNode(n) ((n)->prev) //返回结点n的前驱结点地址
#define listNextNode(n) ((n)->next) //返回结点n的后继结点地址
#define listNodeValue(n) ((n)->value) //返回结点n的值

#define listSetDupMethod(l,m) ((l)->dup = (m)) //设置链表l的复制函数为m方法
#define listSetFreeMethod(l,m) ((l)->free = (m)) //设置链表l的释放函数为m方法
#define listSetMatchMethod(l,m) ((l)->match = (m)) //设置链表l的比较函数为m方法

#define listGetDupMethod(l) ((l)->dup) //返回链表l的赋值函数
#define listGetFree(l) ((l)->fre) //返回链表l的释放函数
#define listGetMatchMethod(l) ((l)->match) //返回链表l的比较函数

list *listCreate(void); //创建表头
void listRelease(list *list); //释放list表头和链表
list *listAddNodeHead(list *list,void *value); //将value添加到list链表的头部
list *listAddNodeTail(list *list,void *value); //将value添加到list链表的尾部
list *listInsertNode(list *list,listNode *old_node,void *value,int after); //在list中，根据after在old_node结点前后插入值为value的结点
void listDelNode(list *list,listNode *node); //从list删除node结点
listIter *liistGetIterator(list *list,int direction); //为list创建一个迭代器
listNode *listNext(listIter *iter); //返回迭代器iter指向的当前结点并更新iter
void listReleaseIterator(listIter *iter); //释放iter迭代器
list *listDup(list *orig); //拷贝表头为orig的链表并返回
listNode *listSearchKey(list *list,void *key); //在list中查找value为key的结点并返回
listNode *listIndex(list *list,long index); //返回下标为index的结点地址
void listRewind(list *list,listIter *li); //将迭代器li重置为list的头结点并设置正向迭代
void listREwindTail(list *list,listIter *li); //将迭代其li重置为list的尾结点并设置反向迭代
void listrotate(list *list); //将微机电插到头结点

#define AL_START_HEAD 0 //正向迭代,从表头向表尾迭代
#define AL_START_TAIL 1 //反向迭代，从表尾向表头迭代

#endif