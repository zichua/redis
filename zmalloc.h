#ifndef ZMALLOC_H
#define ZMALLOC_H

#define zmalloc malloc
#define zrealloc realloc
#define zcallox(x) calloc(x,1)
#define zfree free
#define zstrdup strdup

#endif