#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	start =daStart;
		hLimit=daLimit;
		segmentbrk=start+initSizeToAllocate;
		segmentbrk=ROUNDUP(segmentbrk,PAGE_SIZE);

		if((initSizeToAllocate>daLimit)||((daStart + initSizeToAllocate)>daLimit))
			return E_NO_MEM;
		// handle if size = limit > sbrk
		// 0-4 // 4-8 // 8-12
		for(uint32 i=daStart;i<daStart+initSizeToAllocate;i+=PAGE_SIZE){
	//		uint32 *pageTable;
	//		struct Frame_Info *ptr=get_frame_info(ptr_page_directory,(void*)i,&pageTable);
	//		if(ptr!=NULL){
	//			continue;
	//		}
			struct FrameInfo* ptrr;
			int ret=allocate_frame(&ptrr);
			if(ret==E_NO_MEM)
			{
				return E_NO_MEM;
			}
			map_frame(ptr_page_directory,ptrr,i,PERM_WRITEABLE);

		}
		// access null in this fun call
		initialize_dynamic_allocator(daStart,initSizeToAllocate);
		return 0;

}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
   // edit in ms1 not finished yet
	if(increment>0)
		{
			// roundup then check if in the hard boundries
			// should i pass page_size or just 4
			increment=ROUNDUP(increment,PAGE_SIZE);
			uint32 prevSbrk=segmentbrk;
			if(segmentbrk+increment<=hLimit){
				segmentbrk+=increment;

				for(uint32 i=prevSbrk;i<segmentbrk;i+=(PAGE_SIZE)){
					struct Frame_Info* ptrr=NULL;
					int ret=allocate_frame((void*)&ptrr);
					if(ret==E_NO_MEM)
					{
						return (void*)E_NO_MEM;
					}
					map_frame(ptr_page_directory,(void*)ptrr,i,PERM_USER|PERM_WRITEABLE);

				}
			}
			else{
				panic("in sbrk func increment>0");
			}
			return (void*)prevSbrk;
		}
		else if(increment==0)
		{
			return (void*)segmentbrk;
		}
		else if(increment<0)
		{
			// dec sbrk to increment
			// if inc=10 , should i free 3 pages (12) or only 2 pages(8)
			increment=ROUNDUP(-increment,PAGE_SIZE);
			uint32 newSbrk=segmentbrk-increment;
			if(newSbrk<start){
				panic("in sbrk func increment<0 and newSbrk<start");
			}
			for(uint32 i=segmentbrk;i>newSbrk;i-=(PAGE_SIZE)){
				unmap_frame(ptr_page_directory,i);
				free_frame((struct FrameInfo*)i);
			}
			return (void*)newSbrk;
		}
		return (void*)-1 ;

}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	// 16

//	if(size<DYN_ALLOC_MAX_BLOCK_SIZE)
//	{
//		//
//	}
//	else
//	{
//		size=ROUNDUP(size,PAGE_SIZE);
//		//
//	}


	return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
