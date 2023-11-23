#include <inc/lib.h>
#include <inc/syscall.h>
#include <lib/syscall.c>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

struct User_Heap_Data {
	uint32 va;
	int size;
	int num_of_pages;
	bool marked;
	LIST_ENTRY(User_Heap_Data)
	prev_next_info;
};

LIST_HEAD(User_Heap_Data_LIST, User_Heap_Data);
struct User_Heap_Data_LIST user_heap_data_list;
void initUserHeapData() {
	uint32 hardLimit = sys_get_hard_limit();
	struct User_Heap_Data *firstUserData = (struct User_Heap_Data *)(hardLimit + PAGE_SIZE);
	firstUserData->va = hardLimit + PAGE_SIZE;
	firstUserData->marked = 0;
	firstUserData->size = (USER_HEAP_MAX - (hardLimit + PAGE_SIZE));
	firstUserData->num_of_pages = (firstUserData->size / PAGE_SIZE);
	cprintf("userstart : %d \nva %d\nsize %d\n", firstUserData,
			firstUserData->va, firstUserData->size);
	LIST_INSERT_HEAD(&user_heap_data_list, firstUserData);
}
#define kilo 1024
int FirstTimeFlag = 1;
void InitializeUHeap() {
	initUserHeapData();
	if (FirstTimeFlag) {
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment) {
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0)
		return NULL;
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
//	// Write your code here, remove the panic and write your code
//	cprtinf(
//			"struct user data va : %d \nstruct user data size : %d\nstruct user data numOfPages : %d\n",
//			user_heap_data_list.lh_first->va,
//			user_heap_data_list.lh_first->size,
//			user_heap_data_list.lh_first->num_of_pages
//			);
//	panic("malloc() is not implemented yet...!!");

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
//		return alloc_block_FF(size);
//		if (sys_isUHeapPlacementStrategyFIRSTFIT()) {
//			return alloc_block_FF(size);
//		} else if (sys_isUHeapPlacementStrategyBESTFIT()) {
//			return alloc_block_BF(size);
//		}
	} else {
		size = ROUNDUP(size, PAGE_SIZE);
		struct User_Heap_Data *ptr;
		LIST_FOREACH(ptr,&user_heap_data_list)
		{
			if (ptr->size > size && !ptr->marked) {
				struct User_Heap_Data *newData = ptr
						+ sizeof(struct User_Heap_Data);
				newData->marked = 0;
				newData->size = ptr->size - size;
				newData->va = ptr->va + size;
				newData->num_of_pages = (ptr->size - size) / PAGE_SIZE;
				ptr->size = size;
				ptr->marked = 1;
				ptr->num_of_pages = size / PAGE_SIZE;
				LIST_INSERT_AFTER(&user_heap_data_list, ptr, newData);
				sys_allocate_user_mem(ptr->va, ptr->size);
				return (void *) ptr->va;
			}
		}
	}
	return NULL;
}
//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address) {
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	panic("free() is not implemented yet...!!");
//	uint32 hardLimit = syscall(SYS_get_hard_limit, hardLimit, 0, 0, 0,
//						0);
//	if(virtual_address >= (void *)USER_HEAP_START && virtual_address <= (void *)hardLimit){
//		free_block(virtual_address);
//	}else if(virtual_address >= (void *)(hardLimit + (4*kilo)) && virtual_address <= (void *)USER_HEAP_MAX){
//
//	}else {
//		panic("invalid address...!!");
//	}
}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0)
		return NULL;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address) {
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}

//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize) {
	panic("Not Implemented");

}
void shrink(uint32 newSize) {
	panic("Not Implemented");

}
void freeHeap(void* virtual_address) {
	panic("Not Implemented");

}
