//简单动态字符串sds

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024) //预先分配内存的最大长度

#include <sys/types.h>
#include <stdarg.h>

typedef char *sds; //sds

//sds表头
struct sdshdr
{
    unsigned int len; //buf中已用空间的长度
    unsigned int free; //buf中剩余可用空间的长度
    char buf[]; //初始化sds分配的数据空间，并且是柔性数组
};

//计算buf中字符串的长度
static inline size_t sdslen(const sds s)
{
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

//计算buf中未使用空间的长度
static inline size_t sdsavail(const sds s)
{
    struct sdshdr *sh = (void *)(s-sizeof(struct sdshdr));
    return sh->free;
}

sds sdsnewlen(const void *init,size_t initlen); //创建一个长为initlen的字符串，并保存init的值
sds sdsnew(const char *init); //创建一个默认长度的字符串
sds sdsempty(void); //创建一个只有表头，字符串为空"\0"的sds
size_t sdslen(const sds s); //计算buf中字符串的长度
sds sdsdup(const sds s); //复制一份s的副本
void sdsfree(sds s); //释放s字符串和表头
size_t sdsavail(const sds s); //计算buf中的未使用空间的长度
sds sdsgrowzero(sds s,size_t len); //将sds扩展指定长度，并赋值为0
sds sdscatlen(sds s,const void *t,size_t len); //将字符串t追加到s表头的buf末尾，追加len个字节
sds sdscat(sds s,const char *t); //将t字符串拼接到s的末尾
sds sdscatsds(sds s,const sds t); //将t追加到s的末尾
sds sdscpylen(sds s,const char *t,size_t len); //将字符串t覆盖到表头的buf中，拷贝len个字节
sds sdscpy(sds s,const char *t); //将字符串t覆盖到s的表头buf中

sds sdscatvprintf(sds s,const char *fmt,va_list ap); //打印函数，被sdscatprintf调用
#ifdef __GNUC__
sds sdscatprintf(sds s,const char *fmt,...) //打印任意数量个字符串，并将这些字符串追加到给定s的末尾
    __attribute__((format(prinitf,2,3)));
#else
sds sdscatprintf(sds s,const charr *fmt,...);
#endif

sds sdscatfmt(sds s,char const *fmt,...); //格式化打印多个字符串，并将这些字符串追加到给定s的末尾
sds sdstrim(sds s,const char *cset); //去除sds中包含由cset字符串出现字符的字符
void sdsrange(sds s,int start,int end); //根据start和end截取字符串
void sdsupdatelen(sds s); //跟新字符串s的长度
void sdsclear(sds s); //将字符串重置保存空间，惰性释放
int sdscmp(const sds s1,const sds s2); //比较两个sds的大小，相等返回0
//使用长度为seplen的sep分隔符对长度为len的s进行分割，返回一个sds数组的地址，*count被设置为数组元素数量
sds *sdssplitlen(const char *s,int len,const char *sep,int seplen,int *count);
void sdsfreesplitres(sds *tokens,int count); //释放tokens中的count个sds元素
void sdstolower(sds s); //将sds所有字符转为小写
void sdstoupper(sds s); //将sds所有字符转为大写
sds sdsfromlonglong(long long value); //根据value创建一个sds
sds sdscatrepr(sds s,const char *p,size_t len); //将长度为len的字符串p以带引号“”的格式追加到s末尾
sds *sdssplitargs(const char *line,int *argc); //参数拆分，用于config.c中对配置未见进行分析
sds sdsmapchars(sds s,const char *from,const char *to,size_t setlen); //将s中所有在from中字符串，替换称to中的字符串
sds sdsjoin(char **argv,int argc,char *sep); //以分隔符连接字符串子数组构成新的字符串

sds sdsMakeRoomFor(sds s,size_t addlen); //对sds中buf的长度进行扩展
void sdsIncrlen(sds s,int incr); //根据incr的正负，移动字符串末尾\0标志
sds sdsRemoveFreeSpace(sds s); //回收sds中的未使用空间
size_t sdsAllocSize(sds s); //获得sds所有分配的空间

#endif