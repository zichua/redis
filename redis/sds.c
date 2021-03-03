//简单动态字符串sds

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "sds.h"
#include "zmalloc.h"

//创建一个长度为initlen的字符串，并保存init中的值
sds sdsnewlen(const void *init,size_t initlen)
{
    struct sdshdr *sh;

    //申请空间
    if(init) //init非空，申请空间为表头+initlen+\0
        sh = zmalloc(sizeof(struct sdshdr) + initlen + 1);
    else //init为空,申请空间初始化为0
        sh = zcalloc(sizeof(struct sdshdr) + initlen + 1);

    if(sh == NULL)
        return NULL;
    //初始化sh
    sh->len = initlen;
    sh->free = 0;
    if(initlen && init)
        memcpy(sh->buf,init,initlen); //拷贝
    sh->buf[initlen] = '\0';

    return (char *) sh->buf;
}

//创建一个只有表头，字符串为空‘\0’的sds
sds sdsempty(void)
{
    return sdsnewlen("",0);
}

//根据字符串init,创建一个与init一样长度的sds
sds sdsnew(const char *init)
{
    size_t initlen = (init == NULL ? 0 : strlen(init));

    return sdsnewlen(init,initlen);
}

//拷贝一份s的副本
sds sdsdup(const sds s)
{
    return sdsnewlen(s,sdslen(s));
}

//释放s字符串和表头
void sdsfree(sds s)
{
    if(s == NULL)
        return;
    zfree(s-sizeof(struct sdshdr));
}

//更新字符串s的长度
void sdsupdatelen(sds s)
{
    //让sh指向s的起始地址
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
    int reallen = strlen(s);

    sh->free += (sh->len - reallen);
    sh->len = reallen;
}

//将字符串重置保存空间，惰性释放
void sdsclear(sds s)
{
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
    sh->free += sh->len;
    sh->len = 0;
    sh->buf[0] = '\0';
}

//对sds中buf的长度进行扩展
sds sdsMakeRoomFor(sds s,size_t addlen)
{
    struct sdshdr *sh,*newsh;
    size_t free = sdsavail(s); //获得s的未使用空间长度
    size_t len , newlen;

    if(free >= addlen) //free长度够用，不用扩展直接返回
        return s;
    
    len = sdslen(s);
    sh = (void *)(s-(sizeof(struct sdshdr))); //获得s表头地址
    newlen = (len + addlen); //扩展后的新长度

    //空间预分配策略
    if(newlen < SDS_MAX_PREALLOC) //newlen < 1mb，free = newlen
        newlen *= 2;
    else //newlen > 1mb, free = 1mb
        newlen += SDS_MAX_PREALLOC;
    
    newsh = zrealloc(sh,sizeof(struct sdshdr) + newlen + 1); //分配空间，获得地址
    if(newsh == NULL)
        return NULL;

    newsh->free = newlen - len; //更新free

    return newsh->buf;
}

//回收sds中的未使用空间
sds sdsRemoveFreeSpace(sds s)
{
    struct sdshdr *sh;

    sh = (void *)(s-(sizeof(struct sdshdr)));
    sh = zrealloc(sh,sizeof(struct sdshdr) + sh->len + 1); //只分配len空间
    sh->free = 0; //更新free

    return sh->buf;
}

//获得sds所分配的空间
size_t sdsAllocSize(sds s)
{
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));

    return sizeof(*sh) + sh->len + sh->free + 1;
}

//根据incr的正负，移动字符串末尾的'\0'标志
void sdsIncrLen(sds s,int incr)
{
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));

    //表达式正确，无操作，错误，返回异常终止程序执行
    if(incr > 0) //保证free >= 要扩展的空间incr
        assert(sh->free >= (unsigned int)incr);
    else
        assert(sh->len  >= (unsigned int)(-incr));

    sh->len += incr;
    sh->free -= incr;
    s[sh->len] = '\0';
}

//将sds扩展指定长度，并赋值为0
sds sdsgrowzero(sds s,size_t len)
{
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
    size_t totlen , curlen = sh->len;

    if(len <= curlen) //小于返回
        return s;

    s = sdsMakeRoomFor(s,len-curlen); //扩展free
    if(s == NULL)
        return NULL;

    sh = (void *)(s-(sizeof(struct sdshdr))); //获得表头
    memset(s+curlen,0,(len-curlen+1)); //扩展清0

    totlen = sh->len + sh->free; //总长度
    sh->len = len;
    sh->free = totlen - sh->len;

    return s;
}

//将字符串追加到s表头的buf末尾，追加len个字节
sds sdscatlen(sds s,const void *t,size_t len)
{
    struct sdshdr *sh;
    size_t curlen = sdslen(s);

    s = sdsMakeRoomFor(s,len);
    if(s == NULL)
        return NULL;
    
    sh = (void *)(s-(sizeof(struct sdshdr)));
    memcpy(s+curlen,t,len);

    sh->len = curlen + len;
    sh->free = sh->free - len;
    s[curlen+len] = '\0';

    return s;
}

//将字符串t拼接到是s的末尾
sds sdscat(sds s,const char *t)
{
    return sdscatlen(s,t,strlen(t));
}

//将sds追加到s的末尾
sds sdscatsds(sds s,const sds t)
{
    return sdscatlen(s,t,sdslen(t));
}

//将字符串t覆盖到s表头的buf中，拷贝len个字节
sds sdscpylen(sds s,const char *t,size_t len)
{
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
    size_t totlen = sh->free + sh->len; //s的总长度

    if(totlen < len) //总长度小于len
    {
        s = sdsMakeRoomFor(s,len-sh->len); //扩展free
        if(s == NULL)
            return NULL;
        sh = (void *)(s-(sizeof(struct sdshdr)));
        totlen = sh->free + sh->len;
    }
    memcpy(s,t,len); //覆盖
    s[len] = '\0';

    sh->len = len;
    sh->free = totlen - len;

    return s;
}

#define SDS_LLSTR_SIZE 21

//将字符串t覆盖到s中
sds sdscpy(sds s,const char *t)
{
    return sdscpylen(s,t,strlen(t));
}

//将一个long long 类型的value传化为字符串，返回字符串长度
int sdsll2str(char *s,long long value)
{
    char *p,aux;
    unsigned long long v;
    size_t l;

    v = (value < 0) ? -value : value;
    p = s;

    do
    {
        *p++ = '0' + (v%10);
        v /= 10;
    }while(v);
    if(value < 0)
        *p++ = '-';

    l = p - s;
    *p = '\0';

    p--;
    while(s < p) //反转字符串，s存储转化后的字符串
    {
        aux = *s;
        *s = *p;
        *p = aux;

        s++;
        p--;
    }    

    return l;
}

//将一个unsigned long long 类型的value转化为字符串，返回字符串长度
int sdsull2str(char *s,long long value)
{
    char *p,aux;
    size_t l;

    p = s;
    do
    {
        *p++ = '0' + (v%10);
        v /= 10;
    }while(v);

    l = p - s;
    *p = '\0';

    p--;
    while(s < p)
    {
        aux = *s;
        *s = *p;
        *p = aux;

        s++;
        p--;
    }

    return l;
}

//根据long long value创建一个sds
sds sdsfromlonglong(long long value)
{
    char buf[SDS_LLSTR_SIZE];
    int len = sdsll2str(buf,value);

    return sdsnewlen(buf,len);
}

//打印函数，被 sdscatprintf 所调用
sds sdscatvprintf(sds s,const char *fmt,va_list ap)
{
    
}