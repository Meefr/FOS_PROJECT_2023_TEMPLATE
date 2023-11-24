#include <inc/lib.h>
//#include <inc/syscall.h>
//#include <lib/syscall.c>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
#define numofVmem (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE
uint32 vMem[numofVmem];

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

	//	// Write your code here, remove the panic and write your code
	//	panic("malloc() is not implemented yet...!!");
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
		return alloc_block_FF(size);
//		if (sys_isUHeapPlacementStrategyFIRSTFIT()) {
//			return alloc_block_FF(size);
//		} else if (sys_isUHeapPlacementStrategyBESTFIT()) {
//			return alloc_block_BF(size);
//		}
	} else {
		size = ROUNDUP(size, PAGE_SIZE);
		int number_of_pages = size / PAGE_SIZE;
		int count = 0;
//		cprintf("%d\n" , vMem[0]);
		for (int i = 0; i < numofVmem; i++) {
			if (vMem[i] == 0)
				count++;
			else {
				count = 0;
			}
			if (count == number_of_pages) {
				i -= (number_of_pages - 1);
				uint32 startAddress = (sys_get_hard_limit() + PAGE_SIZE)
						+ (PAGE_SIZE * i);
				for (int j = 0; j < number_of_pages; j++) {
					vMem[i] = startAddress;
					i++;
				}
				cprintf("Va %d\nstart of heap %d\n", startAddress,
						(sys_get_hard_limit() + PAGE_SIZE));
				sys_allocate_user_mem(startAddress, size);
				return (void *) startAddress;
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
	//	panic("free() is not implemented yet...!!");
	if ((uint32) virtual_address == 0 || (uint32) virtual_address >= USER_LIMIT) {
		panic("invalid address to free!");
	} else if ((uint32) virtual_address >= USER_HEAP_START
			&& (uint32) virtual_address < sys_get_hard_limit()) {
		free_block(virtual_address);
	} else {
		int count = 0;
		uint8 flag = 0;
		for (int i = 0; i < numofVmem; i++) {
			if (vMem[i] == (uint32) virtual_address) {
				flag = 1;
				count++;
				vMem[i] = 0;
			} else if (vMem[i] != (uint32) virtual_address && flag) {
				sys_free_user_mem((uint32) virtual_address,
						(count * PAGE_SIZE));
				return;
			}
		}
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
