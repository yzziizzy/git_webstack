


#include "memtricks.h"
#include "sti/talloc.h"

struct alloc_info* g_current_alloc_info;
struct alloc_info g_default_alloc_info;
struct alloc_info g_shitty_arena_alloc_info;




void memtricks_init() {
	g_default_alloc_info.m_alloc = default_malloc;
	g_default_alloc_info.m_calloc = default_calloc;
	g_default_alloc_info.m_free = default_free;
	g_default_alloc_info.m_realloc = default_realloc;
	g_default_alloc_info.data = NULL;
	
	g_shitty_arena_alloc_info.m_alloc = memtricks_shitty_arena_malloc;
	g_shitty_arena_alloc_info.m_calloc = memtricks_shitty_arena_calloc;
	g_shitty_arena_alloc_info.m_free = memtricks_shitty_arena_free;
	g_shitty_arena_alloc_info.m_realloc = memtricks_shitty_arena_realloc;
	g_shitty_arena_alloc_info.data = NULL;
	
	memtricks_set_defaults();
}

void memtricks_set_defaults() {
	g_current_alloc_info = &g_default_alloc_info;
}

char* strdup_current_alloc(const char* const s) {
	if(!s) return NULL;
	
	size_t len = strlen(s) + 1;
	
	char* out = g_current_alloc_info->m_alloc(g_current_alloc_info, len);
	if(!out) return NULL;
	
	memcpy(out, s, len);
	
	return out;
}

char* strndup_current_alloc(const char* const s, size_t n) {
	if(!s) return NULL;
	
	size_t len = strlen(s);
	if(n < len) len = n;
	
	char* out = g_current_alloc_info->m_alloc(g_current_alloc_info, len + 1);
	if(!out) return NULL;
	
	memcpy(out, s, len);
	out[len] = 0;
	
	return out;
}



struct shitty_arena_alloc_chunk {
	void* ptrs[128];
	int fill;
	struct shitty_arena_alloc_chunk* next;
};




void memtricks_set_shitty_arena() {
	g_current_alloc_info = &g_shitty_arena_alloc_info;
	
	if(!g_shitty_arena_alloc_info.data) {
		struct shitty_arena_alloc_chunk* ch = default_malloc(NULL, sizeof(*ch));
		ch->next = NULL;
		ch->fill = 0;
		g_shitty_arena_alloc_info.data = ch;
	}
}


static void add_shitty_pointer(struct shitty_arena_alloc_chunk** chp, void* p) {
	if((*chp)->fill >= 128) {
		struct shitty_arena_alloc_chunk* ch = default_malloc(NULL, sizeof(*ch));
		ch->next = *chp;
		ch->fill = 0;
		*chp = ch;
	}
	
	(*chp)->ptrs[(*chp)->fill] = p;
	(*chp)->fill++;
}

static void replace_shitty_pointer(struct shitty_arena_alloc_chunk* chp, void* old, void* new) {
	while(chp) {
		for(int i = 0; i < chp->fill; i++) {
			if(chp->ptrs[i] != old) continue;
			
			chp->ptrs[i] = new;
			return;
		}
		
		chp = chp->next;
	}
}
#include <stdio.h>
static void free_all_shitty_pointers(struct shitty_arena_alloc_chunk* chp) {
	while(chp) {
		for(int i = 0; i < chp->fill; i++) {
			default_free(NULL, chp->ptrs[i]);
		}
		
		void* old = chp;
		chp = chp->next;
		default_free(NULL, old);
	}
}

void memtricks_shitty_arena_free_all() {
	
	free_all_shitty_pointers(g_shitty_arena_alloc_info.data);
	
	struct shitty_arena_alloc_chunk* ch = default_malloc(NULL, sizeof(*ch));
	ch->next = NULL;
	ch->fill = 0;
	g_shitty_arena_alloc_info.data = ch;
}

void memtricks_shitty_arena_exit() {
	memtricks_shitty_arena_free_all();
	
	memtricks_set_defaults();
}

int g_count = 0;
void* memtricks_shitty_arena_malloc(struct alloc_info*, size_t sz) {
	void* p = default_malloc(NULL, sz);
	add_shitty_pointer((struct shitty_arena_alloc_chunk**)&g_shitty_arena_alloc_info.data, p);
	return p;
}
void* memtricks_shitty_arena_realloc(struct alloc_info*, void* p, size_t sz) {
	void* np = default_realloc(NULL, p, sz);
	if(p != np) replace_shitty_pointer(g_shitty_arena_alloc_info.data, p, np);
	return np;
}
void* memtricks_shitty_arena_calloc(struct alloc_info*, size_t n, size_t sz) {
	void* p = memtricks_shitty_arena_malloc(n, sz);
	memset(p, 0, sz);
	return p;
}
void memtricks_shitty_arena_free(struct alloc_info*, void* p) {
	// does nothing. free the entire area all at once later.
}




#ifdef malloc
	#undef malloc
#endif
#ifdef free
	#undef free
#endif
#ifdef calloc
	#undef calloc
#endif
#ifdef realloc
	#undef realloc
#endif

void* default_malloc(struct alloc_info*, size_t sz) {
	return malloc(sz);
}
void* default_realloc(struct alloc_info*, void* p, size_t sz) {
	return realloc(p, sz);
}
void* default_calloc(struct alloc_info*, size_t n, size_t sz) {
	return calloc(n, sz);
}
void default_free(struct alloc_info*, void* p) {
	free(p);
}




