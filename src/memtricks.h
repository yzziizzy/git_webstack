#ifndef __memtricks_h__
#define __memtricks_h__

// these files need to be included before the macros below so that their include guards are triggered 
//  when user files include them after this file.
#include <stdlib.h>
#include <memory.h>
#include <string.h>



struct alloc_info {
	void* (*m_alloc)(struct alloc_info*, size_t);
	void* (*m_calloc)(struct alloc_info*, size_t, size_t);
	void (*m_free)(struct alloc_info*, void*);
	void* (*m_realloc)(struct alloc_info*, void*, size_t);
	void* data;
};

extern struct alloc_info* g_current_alloc_info;
extern struct alloc_info g_default_alloc_info;

void* default_malloc(struct alloc_info*, size_t sz);
void* default_realloc(struct alloc_info*, void* p, size_t sz);
void* default_calloc(struct alloc_info*, size_t n, size_t sz);
void default_free(struct alloc_info*, void* p);

void memtricks_init();
void memtricks_set_defaults();

char* strdup_current_alloc(const char* const s);
char* strndup_current_alloc(const char* const s, size_t n);


#ifndef NO_MEM_TRICKS_DEFINES
	#define malloc(sz) (g_current_alloc_info->m_alloc(g_current_alloc_info, sz))
	#define calloc(n, sz) (g_current_alloc_info->m_calloc(g_current_alloc_info, n, sz))
	#define realloc(p, sz) (g_current_alloc_info->m_realloc(g_current_alloc_info, p, sz))
	#define free(p) (g_current_alloc_info->m_free(g_current_alloc_info, p))
	
	#define strdup(s) strdup_current_alloc(s)
	#define strndup(s, n) strndup_current_alloc(s, n)
#endif

extern struct alloc_info g_shitty_arena_alloc_info;


void memtricks_set_shitty_arena();
void memtricks_shitty_arena_free_all();
void memtricks_shitty_arena_exit();

void* memtricks_shitty_arena_malloc(struct alloc_info*, size_t sz);
void* memtricks_shitty_arena_realloc(struct alloc_info*, void* p, size_t sz);
void* memtricks_shitty_arena_calloc(struct alloc_info*, size_t n, size_t sz);
void memtricks_shitty_arena_free(struct alloc_info*, void* p);





#endif // __memtricks_h__
