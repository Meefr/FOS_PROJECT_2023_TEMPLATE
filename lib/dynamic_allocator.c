/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================

uint32 get_block_size(void* va) {
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *) va - 1);
	return curBlkMetaData->size;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================

int8 is_free_block(void* va) {
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *) va - 1);
	return curBlkMetaData->is_free;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY) {
	void *va = NULL;
	switch (ALLOC_STRATEGY) {
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list) {
	cprintf("=========================================\n");
	struct BlockMetaData* blk;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free);
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart,
		uint32 initSizeOfAllocatedSpace) {
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//	panic("initialize_dynamic_allocator is not implemented yet");
	struct BlockMetaData *head = (struct BlockMetaData *) daStart;
	head->size = initSizeOfAllocatedSpace;
	head->is_free = 1;
	LIST_INSERT_HEAD(&memBlocks, head);
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size) {
	//
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
	//	panic("alloc_block_FF is not implemented yet");
	if (size == 0)
		return NULL;
	struct BlockMetaData *blk, *tmpBlk;
	//	tmpBlk->size = 0;
	LIST_FOREACH(blk, &memBlocks)
	{
		//need to compaction ?

		//blk size is found -> allocate
		if ((blk->size - sizeOfMetaData()) >= size && blk->is_free == 1) {
			//blk size is not enough to hold data -> no split
			if ((blk->size - (sizeOfMetaData() + size)) < sizeOfMetaData()) {
				blk->is_free = 0;
				return (struct BlockMetaData *) ((uint32) blk + sizeOfMetaData());
			}
			//blk size is big enough to hold data -> split
			else {
				tmpBlk = blk;
				blk = (struct BlockMetaData *) ((uint32) blk
						+ (size + sizeOfMetaData()));
				blk->size = tmpBlk->size - (size + sizeOfMetaData());
				blk->is_free = 1;

//				cprintf("blk: %x\ntmp: %x\n", blk, tmpBlk);
				LIST_INSERT_AFTER(&memBlocks, tmpBlk, blk);
				tmpBlk->size = size + sizeOfMetaData();
				tmpBlk->is_free = 0;
				return (struct BlockMetaData *) ((uint32) tmpBlk
						+ sizeOfMetaData());
			}
		}
	}
	//no free space for required size -> no allocate + no space
	uint32* ptr = (uint32 *) sbrk((size + sizeOfMetaData()));
	if (ptr != (uint32 *) -1) {
		tmpBlk = (struct BlockMetaData *) ((uint32) memBlocks.lh_last);
		tmpBlk->size = size + sizeOfMetaData();
		tmpBlk->is_free = 0;
		return (struct BlockMetaData *) ((uint32) tmpBlk + sizeOfMetaData());
	}
	return NULL;

}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size) {
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	panic("alloc_block_BF is not implemented yet");
	return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size) {
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size) {
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va) {
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	//	panic("free_block is not implemented yet");
	struct BlockMetaData *ptr = ((struct BlockMetaData *) va - 1), *blk;

	// ptr need to free is free -> no need to do anything
	// invalid address -> no need to do anything
	// check corners
	LIST_FOREACH(blk, &memBlocks)
	{
		if (blk == ptr) {
			// next and prev meta data is free
			if (ptr->prev_next_info.le_prev != NULL
					&& ptr->prev_next_info.le_next != NULL
					&& ptr->prev_next_info.le_next->is_free == 1
					&& ptr->prev_next_info.le_prev->is_free == 1) {
				ptr->prev_next_info.le_prev->size = (ptr->size
						+ ptr->prev_next_info.le_next->size
						+ ptr->prev_next_info.le_prev->size);
				ptr->prev_next_info.le_next->size = 0;
				ptr->prev_next_info.le_next->is_free = 0;
				ptr->size = 0;
				ptr->is_free = 0;
				LIST_REMOVE(&memBlocks, ptr->prev_next_info.le_next);
				LIST_REMOVE(&memBlocks, ptr);
			}
			// neither next or prev meta data is free
			else if (ptr->prev_next_info.le_prev != NULL
					&& ptr->prev_next_info.le_next != NULL
					&& ptr->prev_next_info.le_next->is_free == 0
					&& ptr->prev_next_info.le_prev->is_free == 0) {
				ptr->is_free = 1;
			}
			// prev meta data is free only
			else if (ptr->prev_next_info.le_prev != NULL
					&& ptr->prev_next_info.le_prev->is_free == 1) {
				ptr->prev_next_info.le_prev->size = (ptr->size
						+ ptr->prev_next_info.le_prev->size);
				ptr->size = 0;
				ptr->is_free = 0;
				LIST_REMOVE(&memBlocks, ptr);
			}
			// next meta data is free only
			else if (ptr->prev_next_info.le_next != NULL
					&& ptr->prev_next_info.le_next->is_free == 1) {
				ptr->size = (ptr->prev_next_info.le_next->size + ptr->size);
				ptr->prev_next_info.le_next->size = 0;
				ptr->prev_next_info.le_next->is_free = 0;
				ptr->is_free = 1;
				LIST_REMOVE(&memBlocks, ptr->prev_next_info.le_next);
			} else
				ptr->is_free = 1;
		}
	}
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size) {
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	panic("realloc_block_FF is not implemented yet");
	return NULL;
}
