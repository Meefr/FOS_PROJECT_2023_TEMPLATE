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

uint32 oldSize = 0;
uint8 isFree = 1;
uint8 isSbrk = 0;
struct BlockMetaData*lastVa;

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
bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart,
		uint32 initSizeOfAllocatedSpace) {
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return;

	is_initialized = 1;
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
	if (size == 0) {
		//cprintf("size = zero");
		return NULL;
	}

	// code update for MS2
	if (!is_initialized) {
		uint32 required_size = size + sizeOfMetaData();
		uint32 da_start = (uint32) sbrk(required_size);
		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break = (uint32) sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
	}
//	cprintf("119\n");
	struct BlockMetaData *blk, *tmpBlk;
	//	tmpBlk->size = 0;
//	cprintf("here!!\n");
//		cprintf("isSbrk %d , size: %d, oldSize: %d, isFree: %d\n",isSbrk,size,oldSize,isFree);
//		cprintf("sbrsk: %d\n",sbrks);
	uint32 tmpSize = oldSize;
	if (isSbrk == 0 || size < oldSize || isFree == 1) {
		isFree = 1;
		LIST_FOREACH(blk, &memBlocks)
		{
			if ((blk->size - sizeOfMetaData()) >= size && blk->is_free == 1) {
				oldSize = size;
				isSbrk = 0;
//		cprintf("sn: %d , sb: %d \n",(size +sizeOfMetaData()) , blk->size);
//			cprintf("126\n");
//			blk size is not enough to hold data -> no split
				if ((blk->size - (sizeOfMetaData() + size)) < sizeOfMetaData()) {
//				cprintf("129\n");
//				cprintf("---------second if---------");
					blk->is_free = 0;
//				cprintf("if %x\n",((uint32) blk + sizeOfMetaData()));
					return (struct BlockMetaData *) ((uint32) blk
							+ sizeOfMetaData());
				}
				//blk size is big enough to hold data -> split
				else {
//				cprintf("136\n");
					tmpBlk = blk;
					blk = (struct BlockMetaData *) ((uint32) blk
							+ (size + sizeOfMetaData()));
					blk->size = tmpBlk->size - (size + sizeOfMetaData());
					blk->is_free = 1;

//				cprintf("blk: %x\ntmp: %x\n", blk, tmpBlk);
					LIST_INSERT_AFTER(&memBlocks, tmpBlk, blk);
					tmpBlk->size = size + sizeOfMetaData();
					tmpBlk->is_free = 0;

//				cprintf("  allco va if split : %x \n",((struct BlockMetaData *) ((uint32) tmpBlk
//						+ sizeOfMetaData())));
//				cprintf("else %x\n",((uint32) tmpBlk + sizeOfMetaData()));
					return (struct BlockMetaData *) ((uint32) tmpBlk
							+ sizeOfMetaData());
				}
			}
		}
	} else {
		struct BlockMetaData* blk = lastVa;
		while(blk != NULL)
		{
			if ((blk->size - sizeOfMetaData()) >= size && blk->is_free == 1) {
				//		cprintf("sn: %d , sb: %d \n",(size +sizeOfMetaData()) , blk->size);
				//			cprintf("126\n");
				//			blk size is not enough to hold data -> no split
				if ((blk->size - (sizeOfMetaData() + size)) < sizeOfMetaData()) {
					//				cprintf("129\n");
					//				cprintf("---------second if---------");
					blk->is_free = 0;
					//				cprintf("if %x\n",((uint32) blk + sizeOfMetaData()));
					lastVa = blk;
					return (struct BlockMetaData *) ((uint32) blk
							+ sizeOfMetaData());
				}
				//blk size is big enough to hold data -> split
				else {
					//				cprintf("136\n");
					tmpBlk = blk;
					blk = (struct BlockMetaData *) ((uint32) blk
							+ (size + sizeOfMetaData()));
					blk->size = tmpBlk->size - (size + sizeOfMetaData());
					blk->is_free = 1;

					//				cprintf("blk: %x\ntmp: %x\n", blk, tmpBlk);
					LIST_INSERT_AFTER(&memBlocks, tmpBlk, blk);
					tmpBlk->size = size + sizeOfMetaData();
					tmpBlk->is_free = 0;

					//				cprintf("  allco va if split : %x \n",((struct BlockMetaData *) ((uint32) tmpBlk
					//						+ sizeOfMetaData())));
					//				cprintf("else %x\n",((uint32) tmpBlk + sizeOfMetaData()));
					lastVa = tmpBlk;
					return (struct BlockMetaData *) ((uint32) tmpBlk
							+ sizeOfMetaData());
				}
			}
			blk = blk->prev_next_info.le_next;
		}
	}
	//no free space for required size -> no allocate + no space
	uint32* ptr = (uint32 *) sbrk((size + sizeOfMetaData()));
//	if(size < tmpSize)
	isFree = 0;
	if (ptr != (void *) -1) {
		isSbrk = 1;
		oldSize = size;
//		cprintf("size: %d , size + meta %d\n",size,size+sizeOfMetaData());
//		cprintf("160\n");
		struct BlockMetaData * tmp2 = (struct BlockMetaData *) ((uint32) ptr
				+ size + sizeOfMetaData());
		tmp2->is_free = 1;
		tmp2->size = PAGE_SIZE - (size + sizeOfMetaData())/*(PAGE_SIZE / 2)*/;
		tmpBlk = (struct BlockMetaData *) ptr;
		tmpBlk->size = size + sizeOfMetaData();
		tmpBlk->is_free = 0;
		LIST_INSERT_TAIL(&memBlocks, tmpBlk);
//		if ((PAGE_SIZE / 2) - (size + sizeOfMetaData()) >= sizeOfMetaData())
//			LIST_INSERT_TAIL(&memBlocks, split);
		LIST_INSERT_TAIL(&memBlocks, tmp2);
//		cprintf("sbrk %x\n",((uint32) tmpBlk + sizeOfMetaData()));
		lastVa = tmpBlk;
		return (struct BlockMetaData *) ((uint32) tmpBlk + sizeOfMetaData());
//		}

//		cprintf("  allco va if sprk no collision : %x \n",((struct BlockMetaData *) ((uint32) tmpBlk
//								+ sizeOfMetaData())));
	}
	//cprintf("179\n");
	return NULL;

}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size) {
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	//	panic("alloc_block_BF is not implemented yet");
	if (size == 0)
		return NULL;
	struct BlockMetaData *blk, *tmpBlk, *blkAt;
	uint32 minSize = -1;
	//	tmpBlk->size = 0;
	LIST_FOREACH(blk, &memBlocks)
	{
		//need to compaction ?
		//blk size is found -> allocate
		if ((blk->size - sizeOfMetaData()) >= size && blk->is_free == 1) {
			if (minSize == -1) {
				minSize = blk->size;
				blkAt = blk;
			} else if (minSize > blk->size) {
				minSize = blk->size;
				blkAt = blk;
			}
		}
	}
	//no free space for required size -> no allocate + no space
	if (minSize == -1) {
		uint32* ptr = (uint32 *) sbrk((size + sizeOfMetaData()));
		if (ptr != (uint32 *) -1) {
			tmpBlk = (struct BlockMetaData *) memBlocks.lh_last;
			if (tmpBlk->is_free == 1) {
				tmpBlk->size = size;
				tmpBlk->is_free = 0;
			} else {
				tmpBlk = (struct BlockMetaData *) ptr;
				tmpBlk->size = size + sizeOfMetaData();
				tmpBlk->is_free = 0;
			}
			return (struct BlockMetaData *) ((uint32) tmpBlk + sizeOfMetaData());
		}
		return NULL;
	} else {
		//blk size is not enough to hold data -> no split
		if ((blkAt->size - (sizeOfMetaData() + size)) < sizeOfMetaData()) {
			blkAt->is_free = 0;
			return (void *) ((uint32) blkAt + sizeOfMetaData());
		}
		//blk size is big enough to hold data -> split
		else {
			tmpBlk = blkAt;
			blkAt = (struct BlockMetaData *) ((uint32) blkAt
					+ (size + sizeOfMetaData()));
			blkAt->size = tmpBlk->size - (size + sizeOfMetaData());
			blkAt->is_free = 1;

			//				cprintf("blk: %x\ntmp: %x\n", blk, tmpBlk);
			LIST_INSERT_AFTER(&memBlocks, tmpBlk, blkAt);
			tmpBlk->size = size + sizeOfMetaData();
			tmpBlk->is_free = 0;
			return (void *) ((uint32) tmpBlk + sizeOfMetaData());
		}
	}
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
	struct BlockMetaData *ptr = ((struct BlockMetaData *) va - 1);
	struct BlockMetaData * next = ptr->prev_next_info.le_next;
	struct BlockMetaData * prev = ptr->prev_next_info.le_prev;
	if (ptr == NULL)
		return;

	isFree = 1;
	isSbrk = 0;
//	cprintf("  free va : %x \n",va);
	// ptr need to free is free -> no need to do anything
	// invalid address -> no need to do anything
	// check corners
	ptr->is_free = 1;
	// next and prev meta data is free
	if (prev != NULL && next != NULL && next->is_free == 1
			&& prev->is_free == 1) {
		prev->size = (ptr->size + next->size + prev->size);
		next->size = 0;
		next->is_free = 0;
		ptr->size = 0;
		ptr->is_free = 0;
		LIST_REMOVE(&memBlocks, next);
		LIST_REMOVE(&memBlocks, ptr);
	}
	// neither next or prev meta data is free
	else if (prev != NULL && next != NULL && next->is_free == 0
			&& prev->is_free == 0) {
		ptr->is_free = 1;
	}
//			// prev meta data is free only
	else if (prev != NULL && prev->is_free == 1) {
		prev->size = (ptr->size + prev->size);
		ptr->size = 0;
		ptr->is_free = 0;
		LIST_REMOVE(&memBlocks, ptr);
	}
	// next meta data is free only
	else if (next != NULL && next->is_free == 1) {
		ptr->size = (next->size + ptr->size);
		next->size = 0;
		next->is_free = 0;
		ptr->is_free = 1;
		LIST_REMOVE(&memBlocks, next);
	}
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size) {
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()

	//know we have to test if i want to increase my size
	//he test if small or equal and 3 if condetion he are tested

	// new_size == 0 -> free
	if (new_size == 0 && va != NULL) {
		free_block(va);
		return NULL;
	}

	// size != zero and va == NULL -> allocate the new size
	else if (new_size != 0 && va == NULL) {
		return alloc_block_FF(new_size);
	}

	// size == 0 and va == NULL
	else if (new_size == 0 && va == NULL) {
		return NULL;
	}

	// size != 0 and va != NULL
	new_size += sizeOfMetaData();
	struct BlockMetaData *ptr = ((struct BlockMetaData *) va - 1), *tmpBlk;
	if (new_size < ptr->size) {
		tmpBlk = ptr;
		if (ptr->size - new_size >= sizeOfMetaData()) {
			ptr = (struct BlockMetaData *) ((uint32) ptr + (new_size));
			ptr->size = tmpBlk->size - (new_size);
			ptr->is_free = 1;
			tmpBlk->size = new_size;
			/*
			 * take care if he can reallocate a free block of data so
			 * free function will change the needed size
			 */
			LIST_INSERT_AFTER(&memBlocks, tmpBlk, ptr);
			if (tmpBlk->is_free == 1) {
				tmpBlk->is_free = 0;
				free_block(
						(struct BlockMetaData *) ((uint32) ptr
								+ sizeOfMetaData()));
				tmpBlk->is_free = 1;
			} else {
				free_block(
						(struct BlockMetaData *) ((uint32) ptr
								+ sizeOfMetaData()));
			}
		}
		return (void *) ((uint32) tmpBlk + sizeOfMetaData());
	} else if (new_size > ptr->size) {
		//This if not handeled in test
		// no check if the next is free or not
		// no test if th next is null
		//no test if the next size < size i need (we compare next with its meta)
		if ((ptr->prev_next_info.le_next != NULL
				&& ptr->prev_next_info.le_next->is_free == 0)
				|| ptr->prev_next_info.le_next == NULL
				|| (ptr->prev_next_info.le_next != NULL
						&& (ptr->prev_next_info.le_next->size - sizeOfMetaData()
								< (new_size - ptr->size)))) {

			free_block(((struct BlockMetaData *) ptr + 1));
			return alloc_block_FF(new_size - sizeOfMetaData());

		} else if (ptr->prev_next_info.le_next != NULL
				&& ptr->prev_next_info.le_next->is_free == 1) {
			if ((ptr->prev_next_info.le_next->size - sizeOfMetaData()
					> (new_size - ptr->size))) {
				uint32 tmpsize = ptr->prev_next_info.le_next->size;
				struct BlockMetaData * tmpPtr = ptr->prev_next_info.le_next;
				tmpPtr->is_free = 0;
				tmpPtr->size = 0;
				LIST_REMOVE(&memBlocks, tmpPtr);
				tmpPtr = (struct BlockMetaData *) ((uint32) tmpPtr
						+ (new_size - ptr->size));
				tmpPtr->size = tmpsize - (new_size - ptr->size);
				tmpPtr->is_free = 1;
				ptr->size = new_size;
			}
//			LIST_INSERT_AFTER(&memBlocks,ptr->prev_next_info.le_next,tmpPtr);

			// here we have problem if the space i need is less than next size-meta so we should allocate

			// This else not handeled in Test

			else {
				ptr->size = new_size;
				ptr->prev_next_info.le_next->is_free = 0;
				ptr->prev_next_info.le_next->size = 0;
				struct BlockMetaData * tmpPtr = ptr->prev_next_info.le_next;
				LIST_REMOVE(&memBlocks, tmpPtr);
			}
			return (void *) ((uint32) ptr + sizeOfMetaData());
		}
	} else if (new_size == ptr->size) {
		return va;
	}
	//panic("realloc_block_FF is not implemented yet");
	return NULL;
}
