
#include <stddef.h>
#include <hlist.h>
#include <stdio.h>

#define MEM_ALLOC_MAGIC_NUM  0xACCA5995
typedef struct _mem_node {
	uint32 magicnum;
	struct node nodeinfo;
	/* Size & flag of the current node (the high two bits represent a flag,and the rest bits specify the size) */
	uint32 size_flag;
}mem_node;

uint32 g_heap_start = 0;
uint32 g_heap_end = 0;

#define around(size, align)     (((size) + (align) - 1) & (~((align) - 1)))

#define MEM_NODE_HEAD_SIZE             sizeof(struct _mem_node)
#define MEM_ALIGN_SIZE                 4
#define MEM_NODE_USED_FLAG             0x80000000U
#define MEM_NODE_ALIGNED_FLAG          0x40000000U
#define MEM_NODE_ALIGNED_AND_USED_FLAG (MEM_NODE_USED_FLAG | MEM_NODE_ALIGNED_FLAG)

#define mem_node_get_used_flag(size_flag) ((size_flag) & MEM_NODE_USED_FLAG)
#define mem_node_set_used_flag(size_flag)  ((size_flag) = ((size_flag) | MEM_NODE_USED_FLAG))
#define mem_node_get_aligned_gapsize(sizeAndFlag) ((sizeAndFlag) & ~MEM_NODE_ALIGNED_FLAG)
#define mem_node_get_aligned_flag(size_flag)  ((size_flag) & MEM_NODE_ALIGNED_FLAG)
#define mem_node_set_aligned_flag(size_flag) ((size_flag) = ((size_flag) | MEM_NODE_ALIGNED_FLAG))
#define mem_node_get_size(size_flag) ((size_flag) & ~MEM_NODE_ALIGNED_AND_USED_FLAG)

#define mem_node_clear_used_aligned_flag(size_flag) ((size_flag) = (size_flag) & (~ (MEM_NODE_ALIGNED_AND_USED_FLAG)))

#define is_pow_two(value)             ((((uintptr_t)(value)) & ((uintptr_t)(value) - 1)) == 0)
#define is_aligned(value, alignSize)  ((((uintptr_t)(value)) & ((uintptr_t)((alignSize) - 1))) == 0)

void memlib_init(struct list *listhead, unsigned int base, unsigned int size)
{
	mem_node *firstnode = (mem_node *)(uintptr_t)(base);
	if (mem_node_get_used_flag(size) || mem_node_get_aligned_flag(size)) {
		printf("malloc init failed, size:0x%x\n", size);
		return;
	}
	list_init(listhead);
	firstnode->magicnum = MEM_ALLOC_MAGIC_NUM;
	firstnode->size_flag = size;
	list_add_tail(listhead, &firstnode->nodeinfo);
	g_heap_start = base;
	g_heap_end = base + size;
#ifdef CONFIG_DEBUG_INFO
	puts("malloc init,base: 0x");puthex(g_heap_start);puts(", size: ");putdec(size);puts("\n");
#endif
}

static void *mem_find_freeblock(struct list *listhead, uint32 allocsize)
{
	mem_node *entry = NULL;
	mem_node *tmp = NULL;
	mem_node *newnode = NULL;
	unsigned int nodesize;
	list_for_each_entry_safe(entry, tmp, listhead, mem_node, nodeinfo) {
		nodesize = mem_node_get_size(entry->size_flag);
		if ((!mem_node_get_used_flag(entry->size_flag)) && nodesize >= allocsize) {
			if ((nodesize - allocsize) < (MEM_NODE_HEAD_SIZE + MEM_ALIGN_SIZE)) {
				mem_node_set_used_flag(entry->size_flag);
				return entry + 1;
			} else {
				newnode = (mem_node *)((uintptr_t)entry + allocsize);
				newnode->magicnum = MEM_ALLOC_MAGIC_NUM;
				newnode->size_flag = entry->size_flag - allocsize;
				entry->size_flag = allocsize;
				mem_node_set_used_flag(entry->size_flag);
				list_insert_backward(&entry->nodeinfo, &newnode->nodeinfo); /* newnode 插入到 entry 后面 */
				return entry + 1;
			}
		}
	}
	printf("no suitable free mem block\n");
	return NULL;
}

void *memlib_malloc(struct list *listhead, uint32 bytes)
{
	void *ptr = NULL;
	if (bytes == 0) {
		return NULL;
	}
	uint32 allocsize = around(bytes + MEM_NODE_HEAD_SIZE, MEM_ALIGN_SIZE);
	ptr = mem_find_freeblock(listhead, allocsize);
	return ptr;
}

void *memlib_memalign(struct list *listhead, uint32 alignment, uint32 bytes)
{
	uint32 gap_size;
	uint32 use_size;
	void *ptr = NULL;
	void *align_ptr = NULL;
	mem_node *node = NULL;
	if (bytes == 0 || alignment == 0 || !is_pow_two(alignment) || !is_aligned(alignment, sizeof(void*))) {
		return NULL;
	}

	/*
	 * sizeof(gap_size) bytes stores offset between align_ptr and ptr,
	 * the ptr has been OS_MEM_ALIGN_SIZE(4 or 8) aligned, so maximum
	 * offset between alignedPtr and ptr is alignment - OS_MEM_ALIGN_SIZE
	 */
	if ((alignment - sizeof(gap_size)) > ((uint32)(-1) - bytes)) {
		return NULL;
	}
	use_size = around(bytes + MEM_NODE_HEAD_SIZE + alignment - sizeof(gap_size), MEM_ALIGN_SIZE);
	if (mem_node_get_used_flag(use_size) || mem_node_get_aligned_flag(use_size)) {
		printf("size is too large:0x%x\n", use_size);
		return NULL;
	}
	ptr = mem_find_freeblock(listhead, use_size);
	if (ptr == NULL) {
		return NULL;
	}
	align_ptr = (void *)around((uintptr_t)ptr, alignment);
	if (ptr == align_ptr) {
		return ptr;
	}
	gap_size = (uintptr_t)align_ptr - (uintptr_t)ptr;
	mem_node_set_aligned_flag(gap_size);
	node = (mem_node *)ptr - 1;
	mem_node_set_aligned_flag(node->size_flag);
	*(uint32 *)((uintptr_t)align_ptr - sizeof(gap_size)) = gap_size;
	ptr = align_ptr;
	return ptr;
}
void *memptr_to_node(void *ptr)
{
	uint32 gapsize;
	if (((uintptr_t)ptr) & (MEM_ALIGN_SIZE - 1)) {
		printf("ptr not align by 4byte\n");
		return NULL;
	}
	gapsize = *(uint32 *)((uintptr_t)ptr - sizeof(uint32));
	if (mem_node_get_aligned_flag(gapsize) && mem_node_get_used_flag(gapsize)) {
		printf("gapsize:0x%x error\n", gapsize);
		return NULL;
	}
	if (mem_node_get_aligned_flag(gapsize)) {
		gapsize = mem_node_get_aligned_gapsize(gapsize);
		if ((gapsize & (MEM_ALIGN_SIZE - 1)) || (gapsize > ((uintptr_t)ptr - MEM_NODE_HEAD_SIZE))) {
			return NULL;
		}
		ptr = (void *)((uintptr_t)ptr - gapsize);
	}
	return (void *)((uintptr_t)ptr - MEM_NODE_HEAD_SIZE);
}

void memlib_free(struct list *listhead, void *ptr)
{
	mem_node *entry = NULL;
	mem_node *next_entry = NULL;
	mem_node *pre_entry = NULL;
	if (ptr == NULL) {
		return;
	}
	entry = (mem_node *)memptr_to_node(ptr);
	if (entry == NULL) {
		return;
	}
	if (entry->magicnum != MEM_ALLOC_MAGIC_NUM) {
		printf("MAGIC NUM is wrong! magicnum:0x%x\n", entry->magicnum);
		return;
	}
	if (!mem_node_get_used_flag(entry->size_flag)) {
		return;
	}
	mem_node_clear_used_aligned_flag(entry->size_flag);
	next_entry = list_next_entry(entry, listhead, mem_node, nodeinfo);
	if (next_entry != NULL) {
		if (!mem_node_get_used_flag(next_entry->size_flag)) {
			entry->size_flag += next_entry->size_flag;
			next_entry->magicnum = 0;
			list_remove(&next_entry->nodeinfo);
		}
	}
	pre_entry = list_prev_entry(entry, listhead, mem_node, nodeinfo);
	if (pre_entry != NULL) {
		if (!mem_node_get_used_flag(pre_entry->size_flag)) {
			pre_entry->size_flag += entry->size_flag;
			entry->magicnum = 0;
			list_remove(&entry->nodeinfo);
		}
	}
}

void memlib_show_memnode(struct list *listhead, unsigned int cnt)
{
	mem_node *entry = NULL;
	uint32 i = 0;
	list_for_each_entry(entry, listhead, mem_node, nodeinfo) {
		printf("%d:entry, magic:0x%x, size: %d, used:0x%x, aligned:0x%x\n", i++, entry->magicnum,
				mem_node_get_size(entry->size_flag), mem_node_get_used_flag(entry->size_flag),
				mem_node_get_aligned_flag(entry->size_flag));
		if (cnt != 0 && i == cnt) {
			break;
		}
	}
}
