#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

#define num_of_all_pages 32722
int initialize_kheap_dynamic_allocator(uint32 daStart,
		uint32 initSizeToAllocate, uint32 daLimit) {

	start = daStart;
	hLimit = daLimit;
	segmentbrk = start + initSizeToAllocate;
	segmentbrk = ROUNDUP(segmentbrk, PAGE_SIZE);
	if ((initSizeToAllocate > daLimit)
			|| ((daStart + initSizeToAllocate) > daLimit))
		return E_NO_MEM;
	// handle if size = limit > sbrk
	// 0-4 // 4-8 // 8-12
	for (uint32 i = daStart; i < daStart + initSizeToAllocate; i += PAGE_SIZE) {
//		uint32 *pageTable;
//		struct Frame_Info *ptr=get_frame_info(ptr_page_directory,(void*)i,&pageTable);
//		if(ptr!=NULL){
//			continue;
//		}
		struct FrameInfo* ptrr;
		int ret = allocate_frame(&ptrr);
		ptrr->va = i;
		if (ret == E_NO_MEM) {
			return E_NO_MEM;
		}
		map_frame(ptr_page_directory, ptrr, i, PERM_WRITEABLE);

	}
	// access null in this fun call
	initialize_dynamic_allocator(daStart, initSizeToAllocate);
	return 0;
}

void* sbrk(int increment) {
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
	if (increment > 0) {
		// roundup then check if in the hard boundries
		// should i pass page_size or just 4
		increment = ROUNDUP(increment, PAGE_SIZE);
		segmentbrk = ROUNDUP(segmentbrk,PAGE_SIZE);
		uint32 prevSbrk = segmentbrk;
		if (segmentbrk + increment <= hLimit) {
			segmentbrk += increment;

			for (uint32 i = prevSbrk; i < segmentbrk; i += (PAGE_SIZE)) {
				struct FrameInfo* ptrr;
				int ret = allocate_frame(&ptrr);
				ptrr->va = i;
				if (ret == E_NO_MEM) {
					return (void*) E_NO_MEM;
				}
				map_frame(ptr_page_directory, ptrr, i, PERM_WRITEABLE);

			}
		}
		return (void*) prevSbrk;
	} else if (increment == 0) {
		return (void*) segmentbrk;
	} else if (increment < 0) {
		// dec sbrk to increment
		// if inc=10 , should i free 3 pages (12) or only 2 pages(8)
		//increment = ROUNDUP(-increment, PAGE_SIZE);
		increment = increment * -1;
		uint32 newSbrk = segmentbrk - ((increment / PAGE_SIZE) * PAGE_SIZE);
		if (newSbrk < start) {
			panic("in sbrk func increment<0 and newSbrk<start");
		}
		for (uint32 i = segmentbrk; i > newSbrk; i -= (PAGE_SIZE)) {
			unmap_frame(ptr_page_directory, i);
//			free_frame((struct FrameInfo*) i);
		}
		segmentbrk -= increment;
//		segmentbrk -= ((increment / PAGE_SIZE) * PAGE_SIZE);
		return (void*) segmentbrk;
	}
	else if(segmentbrk+increment>hLimit) {
				cprintf("segmentbrk=%d and hlimit=%d",segmentbrk,hLimit);
				panic("in sbrk func increment>0");
			}
	return (void*) -1;

}

void* kmalloc(unsigned int size) {
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	// 16
//	cprintf("free pages number: %d, non free pages %d\n",tmp, ((KERNEL_HEAP_MAX - (hLimit + PAGE_SIZE))/PAGE_SIZE)-tmp);
	//here we need to know roundUp where ?
	//cprintf("%d \n",(hLimit + PAGE_SIZE));
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
		if (isKHeapPlacementStrategyFIRSTFIT()) {
			// here we need type cast;
			return alloc_block_FF(size);
		}
		if (isKHeapPlacementStrategyBESTFIT()) {
			return alloc_block_BF(size);
		}
	} else {
		//need cast ?
		uint32 new_size = ROUNDUP(size, PAGE_SIZE);
		int number_of_pages = new_size / PAGE_SIZE;
		int count = 0;
		struct FrameInfo * ptr_fram_Info;
		uint32 *pageTable;
		uint32 address;
		// need to rev?
		for (uint32 i = (hLimit + PAGE_SIZE); i < KERNEL_HEAP_MAX; i +=
				(PAGE_SIZE)) {

//			ptr_fram_Info = get_frame_info(&ptr_page_directory, &i,
//					&pageTable);
			ptr_fram_Info = get_frame_info(ptr_page_directory, i, &pageTable);
			uint32 page_table_entry = pageTable[PTX(i)];
//			cprintf("entry: %x\npresent: %x\n",page_table_entry,presentBit);
			if (!(page_table_entry & PERM_PRESENT))
				count++;
			else {
				count = 0;
			}

			if (count == number_of_pages) {
				i = i - ((number_of_pages - 1) * PAGE_SIZE);
				address = i;
				for (uint32 j = 0; j < number_of_pages; j++) {
					struct FrameInfo* ptrr;
					int ret = allocate_frame(&ptrr);
					ptrr->va = i;
					if (ret == E_NO_MEM) {
						return (void*) E_NO_MEM;
					}
					ret = map_frame(ptr_page_directory, ptrr, i,PERM_WRITEABLE);
					if (ret == E_NO_MEM) {
						unmap_frame(ptr_page_directory, i);
						return (void*) E_NO_MEM;
					}
					i += PAGE_SIZE;
				}
//				struct vmBlock *ptr;
//				LIST_INSERT_HEAD(&vmBlocks, ptr);
				for (int j = 0; j < num_of_all_pages; j++) {
					if (vmS[j] == 0) {
						vmS[j] = address;
						numOfPages[j] = number_of_pages;
//						cprintf("number of all pages %d \n", (KERNEL_HEAP_MAX - (hLimit+ PAGE_SIZE))/ PAGE_SIZE);
//						cprintf("%d ,%d, %d\n", vmS[j], numOfPages[j],
//								number_of_pages);
						break;
					}
				}
//				if(address == -134197248 || address == -131051520 || address == -130002944)
//					cprintf("%d ,%d, %d\n",address,size,number_of_pages);
				return (void *) address;
			}
		}
	}

	return NULL;
}

void kfree(void* virtual_address) {
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	if ((uint32) virtual_address >= start
			&& (uint32) virtual_address < hLimit) {
		free_block(virtual_address);
		return;
	} else if ((uint32) virtual_address >= hLimit + PAGE_SIZE
			&& (uint32) virtual_address < KERNEL_HEAP_MAX) {
//		virtual_address = (void *)ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
		for (int i = 0; i < num_of_all_pages; i++) {
			if (vmS[i] == (uint32) virtual_address) {
				for (int j = 0; j < numOfPages[i]; j++) {
//					cprintf("%d \n",(vmS[i] + (PAGE_SIZE * j)));
					unmap_frame(ptr_page_directory, (vmS[i] + (PAGE_SIZE * j)));
				}
				vmS[i] = 0;
				numOfPages[i] = 0;
				return;
			}
		}
	}
//	panic("invalid virtual address");
}

unsigned int kheap_virtual_address(unsigned int physical_address) {
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	struct FrameInfo *ptr_frame_info;
	ptr_frame_info = to_frame_info((uint32)physical_address);
	if (ptr_frame_info->references > 0)
		{
		uint32 offset = physical_address;
		offset = offset << 20;
		offset = offset >> 20;
		return (unsigned int)ptr_frame_info->va + offset;
		}
	//change this "return" according to your answer
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address) {
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
//	ptr_fram_Info = get_frame_info(ptr_page_directory,i,&pageTable);
//	uint32 page_table_entry = pageTable[PTX(i)];
//	uint32 presentBit = page_table_entry;
//	presentBit << 31;
//	presentBit >> 31;

	uint32* ptr;
	int check = get_page_table(ptr_page_directory, virtual_address, &ptr);

	if (check == TABLE_IN_MEMORY) {
		uint32 entry = ptr[PTX(virtual_address)];
		//uint32 offset=entry%PAGE_SIZE;
		uint32 ans = ((entry >> 12) * PAGE_SIZE)
				+ (virtual_address & 0x00000FFF);
		//cprintf("the pa is %x",ans);
		return (int) (ans);
	}
	return 0;
}

void kfreeall() {
	panic("Not implemented!");

}

void kshrink(uint32 newSize) {
	panic("Not implemented!");
}

void kexpand(uint32 newSize) {
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

void *krealloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	if(virtual_address==NULL){
		return kmalloc(new_size);
	}
	else if(new_size==0){
		kfree(virtual_address);
	}
	else{
		if(new_size<= DYN_ALLOC_MAX_BLOCK_SIZE){
			return realloc_block_FF(virtual_address,new_size);
		}
		else{
			uint32 NewNumberOfPages=ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
			uint32 NumberOfCurrentPages=0;
			int indexOfva;
			for (int i = 0; i < num_of_all_pages; i++) {
				if (vmS[i] == (uint32) virtual_address) {
					NumberOfCurrentPages=numOfPages[i];
					indexOfva=i;
					break;
				}
			}
			// the va may not found in the array ?
			if(NumberOfCurrentPages==0){
				cprintf("aaaa0\n");
				return kmalloc(NewNumberOfPages);
			}
			else{
				cprintf("va %d , CurrentPages %d , NewNumberOfPages %d \n",virtual_address,NumberOfCurrentPages,NewNumberOfPages);
				if(NumberOfCurrentPages==NewNumberOfPages){
					//numOfPages[indexOfva]=NewNumberOfPages;
					cprintf("aaaa1\n");
					return virtual_address;
				}
				else if(NumberOfCurrentPages>NewNumberOfPages){
					cprintf("aaaa2\n");
//					uint32 NumberOfPagesToDelet=(NumberOfCurrentPages-NewNumberOfPages);
//					uint32 StartPtr=(uint32)virtual_address+(NewNumberOfPages*PAGE_SIZE);
//					numOfPages[indexOfva]=NewNumberOfPages;
//					for(uint32 i=0;i<NumberOfPagesToDelet;i++){
//						unmap_frame(ptr_page_directory, (StartPtr + (PAGE_SIZE * i)));
//					}
					return virtual_address;
				}
				else{
					cprintf("aaaa3\n");
					struct FrameInfo * ptr_fram_Info;
					uint32 *pageTable;
					uint32 NumberOfPagesNeedToalloc=NewNumberOfPages-NumberOfCurrentPages;
					int count=0;
					uint32 address;
					for(uint32 i=(uint32)virtual_address+(PAGE_SIZE*NumberOfCurrentPages);
							i<(uint32)virtual_address+(PAGE_SIZE*NewNumberOfPages);i+=PAGE_SIZE){
						ptr_fram_Info = get_frame_info(ptr_page_directory, i, &pageTable);
						uint32 page_table_entry = pageTable[PTX(i)];
						if (!(page_table_entry & PERM_PRESENT))
							count++;
						else {
							kfree(virtual_address);


							return kmalloc(new_size);
						}
					}
					if(count>0){
						numOfPages[indexOfva]=NewNumberOfPages;
						for(uint32 i=(uint32)virtual_address+(PAGE_SIZE*NumberOfCurrentPages);
						i<(uint32)virtual_address+(PAGE_SIZE*NewNumberOfPages);i+=PAGE_SIZE){
							struct FrameInfo* ptrr;
							int ret = allocate_frame(&ptrr);
							ptrr->va=i;
							if (ret == E_NO_MEM) {
								return (void*) E_NO_MEM;
							}
							ret = map_frame(ptr_page_directory, ptrr, i,PERM_WRITEABLE);
							if (ret == E_NO_MEM) {
								unmap_frame(ptr_page_directory, i);
								return (void*) E_NO_MEM;
							}
						}
						return virtual_address;
					}
				}
			}
		}
	}
	return NULL;
	//panic("krealloc() is not implemented yet...!!");
}
