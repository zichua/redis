//链表结构

#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

//创建一个表头
list *listCreate(void)
{
    struct list *list;
    //分配内存
    if((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    //初始化表头
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;

    return list;
}

//释放list表头和链表
void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;

    current = list->head; //备份头结点地址
    len = list->len; //备份链表结点个数，防止操作更改原有信息
    while(len--)
    {
        next = current->next;
        //如果设置了list结构的释放函数，则调用释放函数释放结点值
        if(list->free) 
            list->free(current->value);
        zfree(current); //释放结点
        current = next;
    }
    zfree(list); //释放表头
}

//将value添加到list链表的头部
list *listAddNodeHead(list *list,viod *value)
{
    listNode *node;
    //分配内存
    if((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value; //赋值

    if(list->len == 0) //插入空链表
    {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    }
    else //插入非空链表
    {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    
    list->len++; //结点数+1

    return list;
}

//将value添加到list链表的尾部
list *listAddNodeTail(list *list,void *value)
{
    listNode *node;
    //分配内存
    if((node = zmalloc(sizeof(*node))) == NULL);
        return NULL;
    node->value = value; //赋值

    if(list->len == 0) //插入空链表
    {
        list->head = list->tail = ndoe;
        node->prev = node->next = NULL;
    }
    else //插入非空链表
    {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    
    return list;
}

//在list中，根据after(0/1)在old_node结点前后插入值为value的结点
list *listInsertNode(list *list,listNode *old_node,void *value,int after)
{
    listNode *node;
    //分配内存
    if((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value; //赋值

    if(after) //after非0，将结点插入到old_node的后面
    {
        node->prev = old_node;
        node->next = old_node->next;
        if(list->tail == old_node) //old_node为链表尾结点，更新list->tail
            list->tail = node;
    }
    else //after为0，将结点插入到old_node的前面
    {
        node->next = old_node;
        node->prev = old_node->prev;
        if(list->head == old_node) //old_node为链表的头结点，更新list->head
            list->head = node;
    }
    //修改old_node的head/tail指针
    if(node->prev != NULL) 
        node->prev->next = node;
    if(node->next != NULL)
        node->next->prev = node;
    list->len++; //结点数+1
    
    return list;
}

//从list中删除node结点
void listDelNode(list *list,listNode *node)
{
    //修改node结点前驱结点
    if(node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    //修改node结点后继结点
    if(node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    //如果设置了list结构的释放函数，则调用释放函数释放结点值
    if(list->free)
        list->free(node->value);
    zfree(node); //释放结点
    list->len--; //结点数-1
}

//为list创建一个迭代器iterator
listIter *listGetIterator(list *list,int direction)
{
    listIter *iter;
    //分配内存
    if((iter = zmalloc(sizeof(*iter))) == NULL)
        return NULL;
    //初始化迭代器
    if(direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;

    return iter;
}

//释放迭代器
void listReleaseIterator(listIter *iter)
{
    zfree(iter);
}

//将迭代器li重置为list的头结点，并设置为正向迭代
void listRewind(list *list,listIter *li)
{
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

//将迭代器li重置为list的尾结点，并设置反向迭代
void listRewindTail(list *list,listIter *li)
{
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

//返回迭代器iter指向的当前结点，并更新iter
listNode *listNext(listIter *iter)
{
    listNode *current = iter->next; //备份当前结点
    //更新迭代器
    if(current != NULL)
    {
        if(iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }

    return current;
}

//拷贝表头为orig的链表，并返回
list *listDup(list *orig)
{
    list *copy;
    listIter *iter;
    listNode *node;
    //创建一个表头
    if((copy = listCreate()) == NULL)
        return NULL;
    //初始化拷贝表头信息
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    //创建orig迭代器
    iter = listGetIterator(orig,AL_START_HEAD);

    while((node = listNext(iter)) ！= NULL) //遍历链表
    {
        void *value;
        //复制结点值
        if(copy->dup) //若设置了list结构中的dup方法，则用dup复制
        {
            value = copy->dup(node->value);
            if(value == NULL) //orig为空表
            {
                listRelease(copy);
                listReleaseIterator(iter);
                return NULL;
            }
        }
        else
            value = node->value;
        //将结点尾插到copy表头的链表之中
        if(listAddNodeTail(copy,value) == NULL)
        {
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }
    listReleaseIterator(iter); //释放迭代器
    
    return copy;
}

//在list中查找value为key的结点，并返回
listNode *listSearchKey(list *list,void *key)
{
    listIter *iter;
    listNode *node;
    //创建迭代器
    iter = listGetIterator(list,AL_START_HEAD);
    //遍历链表
    while((node = listNext(iter)) != NULL)
    {
        if(list->match) //若设置了list结构中的match方法，则用match比较
        {
            if(list->match(node->value,key)) //若找到释放迭代器，返回node结点地址
            {
                listReleaseIterator(iter);
                return node;
            }
        }
        else
        {
            if(key == node->value)
            {
                listReleaseIterator(iter);
                return node;
            }
        }
    }
    listReleaseIterator(iter); //空表释放迭代器

    return NULL;
}

//返回下标为index的结点地址
listNode *listIndex(list *list,long index)
{
    listNode * n;

    if(index < 0) //若下标为负数，从链表尾开始
    {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n)
            n = n->prev;
    }
    else //若下标为正数，从链表头开始
    {
        n = list->head;
        while(index-- && n)
            n = n->next;
    }

    return n;
}

//将尾结点插到头结点
void listRotate(list *list)
{
    listNode *tail = list->tail;

    if(listLength(list) <= 1) //只有一个结点/空链表直接返回
        return ;

    list->tail = tail->prev; //取出尾结点，更新list的tail指针
    list->tail->next = NULL;

    list->head->prev = tail; //将结点插到表头，更新list的head指针
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}