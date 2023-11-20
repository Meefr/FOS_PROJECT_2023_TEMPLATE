#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

#define kilo 1024
int FirstTimeFlag = 1;
void InitializeUHeap() {
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
	// Write your code here, remove the panic and write your code
//	panic("malloc() is not implemented yet...!!");
get_frame_info();

	// to check the first fit Strategy or not ?
	if (sys_isUHeapPlacementStrategyFIRSTFIT()) {
		if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
			return alloc_block_FF(size);
		} else {
			uint32 hardLimit = syscall(SYS_get_hard_limit, hardLimit, 0, 0, 0,
					0);
			uint32 ptr = hardLimit + (4 * kilo);
			while (ptr != NULL && ptr != USER_HEAP_MAX) {
				/*
				 * check the max size i can hold
				 * so if (user_heap_max - ptr which mean the rest of free size) >= needed size / 4
				 * (means how many (4 kilo) I can hold )
				 * */
				if (ptr != USER_HEAP_MAX && (USER_HEAP_MAX - ptr) < size)
					return NULL;

				/*
				 * the way to check depends on the way of marking in
				 * alloc_user_mem function
				 * */
				if (ptr == NULL) {
					sys_allocate_user_mem(ptr, size);
					size -= (4 * kilo);
					while (size >= (4 * kilo)) {
						sys_allocate_user_mem(ptr, size);
						ptr += (4 * kilo);
						size -= (4 * kilo);
					}
					return ptr;
				}
				ptr += (4 * kilo);

				struct Env* env = curenv;

				uint32* pageDir = env->env_page_directory;
				uint32* pageTable = NULL;
				for(uint32 va = (uint32) pageDir; va <= USER_HEAP_MAX; va += 4 * kilo) {
					uint32 dir_frame = get_frame_info(pageDir, va, &pageTable);
					uint32 page_table_entry = pageTable[PTX(va)];
					if(page_table_entry & PERM_AVAILABLE != 0) {
						page_table_entry = ~page_table_entry;
						allocate_user_mem(env, va, size);
						break;
					}
				}

			}
		}
		return NULL;
	}

	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address) {
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
//	panic("free() is not implemented yet...!!");
	uint32 hardLimit = syscall(SYS_get_hard_limit, hardLimit, 0, 0, 0,
						0);
	if(virtual_address >= (void *)USER_HEAP_START && virtual_address <= (void *)hardLimit){
		free_block(virtual_address);
	}else if(virtual_address >= (void *)(hardLimit + (4*kilo)) && virtual_address <= (void *)USER_HEAP_MAX){

	}else {
		panic("invalid address...!!");
	}
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
