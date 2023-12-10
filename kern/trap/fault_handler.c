/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE) {
	assert(
			LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE;
}
void setPageReplacmentAlgorithmCLOCK() {
	_PageRepAlgoType = PG_REP_CLOCK;
}
void setPageReplacmentAlgorithmFIFO() {
	_PageRepAlgoType = PG_REP_FIFO;
}
void setPageReplacmentAlgorithmModifiedCLOCK() {
	_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;
}
/*2018*/void setPageReplacmentAlgorithmDynamicLocal() {
	_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;
}
/*2021*/void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps) {
	_PageRepAlgoType = PG_REP_NchanceCLOCK;
	page_WS_max_sweeps = PageWSMaxSweeps;
}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE) {
	return _PageRepAlgoType == LRU_TYPE ? 1 : 0;
}
uint32 isPageReplacmentAlgorithmCLOCK() {
	if (_PageRepAlgoType == PG_REP_CLOCK)
		return 1;
	return 0;
}
uint32 isPageReplacmentAlgorithmFIFO() {
	if (_PageRepAlgoType == PG_REP_FIFO)
		return 1;
	return 0;
}
uint32 isPageReplacmentAlgorithmModifiedCLOCK() {
	if (_PageRepAlgoType == PG_REP_MODIFIEDCLOCK)
		return 1;
	return 0;
}
/*2018*/uint32 isPageReplacmentAlgorithmDynamicLocal() {
	if (_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL)
		return 1;
	return 0;
}
/*2021*/uint32 isPageReplacmentAlgorithmNchanceCLOCK() {
	if (_PageRepAlgoType == PG_REP_NchanceCLOCK)
		return 1;
	return 0;
}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt) {
	_EnableModifiedBuffer = enableIt;
}
uint8 isModifiedBufferEnabled() {
	return _EnableModifiedBuffer;
}

void enableBuffering(uint32 enableIt) {
	_EnableBuffering = enableIt;
}
uint8 isBufferingEnabled() {
	return _EnableBuffering;
}

void setModifiedBufferLength(uint32 length) {
	_ModifiedBufferLength = length;
}
uint32 getModifiedBufferLength() {
	return _ModifiedBufferLength;
}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va) {
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory,
				(uint32) fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va) {
#if USE_KHEAP
	struct WorkingSetElement *victimWSElement = NULL;
	uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
	int iWS =curenv->page_last_WS_index;
	uint32 wsSize = env_page_ws_get_size(curenv);
#endif

//	cprintf("ws size : %d \n max %d\n",wsSize,curenv->page_WS_max_size);
	fault_va= ROUNDDOWN(fault_va,PAGE_SIZE);

	if (isPageReplacmentAlgorithmFIFO()) {

		if (wsSize < (curenv->page_WS_max_size)) {

			//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
			//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
			// Write your code here, remove the panic and write your code
			//		cprintf("fault_va: %x\n",fault_va);
			struct FrameInfo* frame;
			allocate_frame(&frame);
			map_frame(curenv->env_page_directory, frame, fault_va,
			PERM_WRITEABLE | PERM_USER);
			int read_page = pf_read_env_page(curenv, (void*) fault_va);
			if (read_page == E_PAGE_NOT_EXIST_IN_PF) {
				//int update = pf_update_env_page(curenv, fault_va, frame);
				//			if ((fault_va<USER_HEAP_START&&fault_va>=USER_HEAP_MAX)||(fault_va<=USTACKBOTTOM&&fault_va>USTACKTOP)) {
				//			if ((fault_va < USER_HEAP_START || fault_va > USTACKTOP)) {
				if (!((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)
						|| (fault_va <= USTACKTOP && fault_va > USTACKBOTTOM))) {
					//			cprintf("kill at read in placement\n va: %x\n",fault_va);
					unmap_frame(curenv->env_page_directory, fault_va);
					sched_kill_env(curenv->env_id);
				}
			}
			struct WorkingSetElement* new_workingset =
					env_page_ws_list_create_element(curenv, fault_va);
			int index = (fault_va / PAGE_SIZE);
			wsVM[index] = new_workingset;
			if (curenv->page_last_WS_element != NULL) {

				LIST_INSERT_BEFORE(&(curenv->page_WS_list),
						curenv->page_last_WS_element, new_workingset);
			} else {
				LIST_INSERT_TAIL(&(curenv->page_WS_list), new_workingset);
				if (curenv->page_WS_max_size == curenv->page_WS_list.size) {
					//			cprintf("inside maxSize");
					curenv->page_last_WS_element =
							curenv->page_WS_list.lh_first;
					curenv->page_last_WS_index = 0;
				} else {
					//			cprintf("inside last WS = null");
					curenv->page_last_WS_index++;
					curenv->page_last_WS_element = NULL;
				}
			}

			//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");

			//refer to the project presentation and documentation for details
		} else {
			//cprintf("REPLACEMENT=========================WS Size = %d\n",
				//	wsSize);
			//refer to the project presentation and documentation for details
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your code
//			panic(
//					"page_fault_handler() FIFO Replacement is not implemented yet...!!");

			// delete the first entered element but check first if it was modified or not and if it was -> update the disk
			struct WorkingSetElement *removed_elm = curenv->page_last_WS_element;
			uint32 page_permissions = pt_get_page_permissions(
					curenv->env_page_directory,
					(uint32) removed_elm->virtual_address);
			// get the frame info for this fault address
			uint32 *pageTable = NULL;
			struct FrameInfo *frameInfo = get_frame_info(
					curenv->env_page_directory, removed_elm->virtual_address,
					&pageTable);
			if ((page_permissions & PERM_MODIFIED) == PERM_MODIFIED) {
				// Modified -> then update the disk
				int ret = pf_update_env_page(curenv,
						(uint32) removed_elm->virtual_address, frameInfo);
				if (ret == 0) {
					//	cprintf("succ updated in disk\n");
					//cprintf("the frameInfo= %x",frameInfo);
				} else if (ret == E_NO_PAGE_FILE_SPACE) {
					//cprintf("page file is full\n");
				}
			}
			// delete it from the WS either it was modified or not
			env_page_ws_invalidate(curenv, removed_elm->virtual_address);
			// now update the empty space in WS with the fault va (placement)

			map_frame(curenv->env_page_directory, frameInfo, fault_va,
			PERM_WRITEABLE | PERM_USER);
			int read_page = pf_read_env_page(curenv, (void*) fault_va);
			if (read_page == E_PAGE_NOT_EXIST_IN_PF) {
				if (!((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)
						|| (fault_va <= USTACKTOP && fault_va > USTACKBOTTOM))) {
					//cprintf("kill at read in placement\n va: %x\n", fault_va);
					unmap_frame(curenv->env_page_directory, fault_va);
					sched_kill_env(curenv->env_id);
				}
			}
			unmap_frame(curenv->env_page_directory,
					removed_elm->virtual_address);
			//cprintf("the ref %d", frameInfo->references);

			struct WorkingSetElement* new_workingset =
					env_page_ws_list_create_element(curenv, fault_va);
			int index = (fault_va / PAGE_SIZE);
			wsVM[index] = new_workingset;
			if (curenv->page_last_WS_element == NULL) {
				//cprintf("inside null\n");
				LIST_INSERT_TAIL(&(curenv->page_WS_list), new_workingset);
				curenv->page_last_WS_element = curenv->page_WS_list.lh_first;
			} else if (curenv->page_last_WS_element != NULL) {
				//		cprintf("inside last!=null %xlast %x\n",frameInfo,curenv->page_last_WS_element);
				LIST_INSERT_BEFORE(&(curenv->page_WS_list),
						curenv->page_last_WS_element, new_workingset);
			}
			if (curenv->page_WS_max_size == curenv->page_WS_list.size) {
				//cprintf("inside maxSize");
//				curenv->page_last_WS_element = curenv->page_WS_list.lh_first;
//				curenv->page_last_WS_index = 0;

			} else {
				//	cprintf("inside last WS = null");
//				curenv->page_last_WS_index++;
//				curenv->page_last_WS_element = NULL;
			}
			env_page_ws_print(curenv);
		}
	} else {
		if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX)) {
			//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
			// Write your code here, remove the panic and write your code
//			panic(
//					"page_fault_handler() LRU Replacement is not implemented yet...!!");
			//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
			uint32 activeSize = curenv->ActiveList.size;
			uint32 secondSize = curenv->SecondList.size;
			int activeMaxSize = curenv->ActiveListSize;
			int secondMaxSize = curenv->SecondListSize;
			// create the elm

			struct WorkingSetElement* new_workingset =
					env_page_ws_list_create_element(curenv, fault_va);
			int index = (fault_va / PAGE_SIZE);
			wsVM[index] = new_workingset;
			uint32 page_permissions = pt_get_page_permissions(
					curenv->env_page_directory,
					(uint32) new_workingset->virtual_address);

			if (activeSize + secondSize < curenv->page_WS_max_size) {
				// LRU placement

				if (activeSize < activeMaxSize) {
					//placement only in activeList
					struct FrameInfo* frame;
					allocate_frame(&frame);
					map_frame(curenv->env_page_directory, frame, fault_va,
					PERM_WRITEABLE | PERM_USER);
					int read_page = pf_read_env_page(curenv, (void*) fault_va);
					if (read_page == E_PAGE_NOT_EXIST_IN_PF) {
						if (!((fault_va >= USER_HEAP_START
								&& fault_va < USER_HEAP_MAX)
								|| (fault_va <= USTACKTOP
										&& fault_va > USTACKBOTTOM))) {
							unmap_frame(curenv->env_page_directory, fault_va);
							sched_kill_env(curenv->env_id);
						}
					}
					LIST_INSERT_HEAD(&(curenv->ActiveList), new_workingset);
					pt_set_page_permissions(curenv->env_page_directory,
							new_workingset->virtual_address, PERM_PRESENT, 0);
					return;
				} else if (activeSize == activeMaxSize) {

					//placement in second list and update the activeList
					// first check if the fault_va exist in secondList
				//	cprintf("if size!=0\n");
					bool found = 0;
					struct WorkingSetElement* elm;
					LIST_FOREACH(elm,&(curenv->SecondList))
					{
					//	cprintf("elm=%x and ws=%x\n", elm->virtual_address,
								//new_workingset->virtual_address);
						if (new_workingset->virtual_address
								== elm->virtual_address) {
							found = 1;
							break;
						} else
							continue;
					}

					if (found) {
						//cprintf("in found placement\n");
						// set perm to present & move it to activeList
						struct WorkingSetElement* ptrActiveToSecond = LIST_LAST(
								&(curenv->ActiveList));
						LIST_REMOVE(&(curenv->ActiveList), ptrActiveToSecond);
						LIST_REMOVE(&(curenv->SecondList), elm);
						LIST_INSERT_HEAD(&(curenv->SecondList),
								ptrActiveToSecond);
						// insert the required elm to the activeList after making free space for it
						LIST_INSERT_HEAD(&(curenv->ActiveList), new_workingset);
						pt_set_page_permissions(curenv->env_page_directory,
								ptrActiveToSecond->virtual_address, 0,
								PERM_PRESENT);
						pt_set_page_permissions(curenv->env_page_directory,
								elm->virtual_address,
								PERM_PRESENT, 0);
						return;
					}

					else {
						cprintf("not found");
						// the required doesnot exist in secondList then read it from PF and add it to ActiveList
						struct FrameInfo* frame;
						allocate_frame(&frame);
						map_frame(curenv->env_page_directory, frame, fault_va,
						PERM_WRITEABLE | PERM_USER);
						int read_page = pf_read_env_page(curenv,
								(void*) fault_va);
						if (read_page == E_PAGE_NOT_EXIST_IN_PF) {
							if (!((fault_va >= USER_HEAP_START
									&& fault_va < USER_HEAP_MAX)
									|| (fault_va <= USTACKTOP
											&& fault_va > USTACKBOTTOM))) {
								unmap_frame(curenv->env_page_directory,
										fault_va);
								sched_kill_env(curenv->env_id);
							}
						}
						struct WorkingSetElement* ptrActiveToSecond = LIST_LAST(
								&(curenv->ActiveList));
						LIST_REMOVE(&(curenv->ActiveList), ptrActiveToSecond);
						LIST_INSERT_HEAD(&(curenv->SecondList),
								ptrActiveToSecond);
						pt_set_page_permissions(curenv->env_page_directory,
								ptrActiveToSecond->virtual_address, 0,
								PERM_PRESENT);
						// insert the new to Activce list
						LIST_INSERT_HEAD(&(curenv->ActiveList), new_workingset);
						pt_set_page_permissions(curenv->env_page_directory,
								new_workingset->virtual_address, PERM_PRESENT,
								0);
						return;
					}
				}

			} else {
				//replacement
				// check first if the wanted va exist or not in secondList

				bool found = 0;
				struct WorkingSetElement* elm;
				LIST_FOREACH(elm,&(curenv->SecondList))
				{
					//cprintf("elm=%x and ws=%x\n", elm->virtual_address,
						//	new_workingset->virtual_address);
					if (ROUNDDOWN(new_workingset->virtual_address,PAGE_SIZE)
							== elm->virtual_address) {
						found = 1;
						break;
					} else
						continue;
				}

				if (found) {
					//cprintf("in found\n");
					// set perm to present & move it to activeList
					struct WorkingSetElement* ptrActiveToSecond = LIST_LAST(
							&(curenv->ActiveList));
					LIST_REMOVE(&(curenv->ActiveList), ptrActiveToSecond);
					LIST_REMOVE(&(curenv->SecondList), elm);
					LIST_INSERT_HEAD(&(curenv->SecondList), ptrActiveToSecond);
					// insert the required elm to the activeList after making free space for it
					LIST_INSERT_HEAD(&(curenv->ActiveList), new_workingset);
					pt_set_page_permissions(curenv->env_page_directory,
							ptrActiveToSecond->virtual_address, 0,
							PERM_PRESENT);
					pt_set_page_permissions(curenv->env_page_directory,
							elm->virtual_address,
							PERM_PRESENT, 0);


						return;
				} else {

					struct WorkingSetElement *removed_elm = LIST_LAST(
							&(curenv->SecondList));
					uint32 page_permissions = pt_get_page_permissions(
							curenv->env_page_directory,
							(uint32) removed_elm->virtual_address);
					// get the frame info for this fault address
					uint32 *pageTable = NULL;
					struct FrameInfo *frameInfo = get_frame_info(
							curenv->env_page_directory,
							removed_elm->virtual_address, &pageTable);
					if (frameInfo == NULL) {
					//	cprintf("frame is null\n");
					}
					if ((page_permissions & PERM_MODIFIED) == PERM_MODIFIED) {
						// Modified -> then update the disk
						int ret = pf_update_env_page(curenv,
								(uint32) removed_elm->virtual_address,
								frameInfo);
						if (ret == 0) {
						//	cprintf("succ updated in disk\n");
							//cprintf("the frameInfo= %x",frameInfo);
						} else if (ret == E_NO_PAGE_FILE_SPACE) {
							//cprintf("page file is full\n");
						}
					}
					// remove the last elm of SecondList
					env_page_ws_invalidate(curenv,
							removed_elm->virtual_address);
//					unmap_frame(curenv->env_page_directory,
//							removed_elm->virtual_address);
					//move elm from active to second to free space
					struct WorkingSetElement* ptrActiveToSecond = LIST_LAST(
							&(curenv->ActiveList));
					LIST_REMOVE(&(curenv->ActiveList), ptrActiveToSecond);
					LIST_INSERT_HEAD(&(curenv->SecondList), ptrActiveToSecond);
					pt_set_page_permissions(curenv->env_page_directory,
							ptrActiveToSecond->virtual_address, 0,
							PERM_PRESENT);
					// insert the new fault
					struct FrameInfo* frame;
					allocate_frame(&frame);
					map_frame(curenv->env_page_directory, frame, fault_va,
					PERM_WRITEABLE | PERM_USER);
					int read_page = pf_read_env_page(curenv, (void*) fault_va);
			//		cprintf("the page = %d\n",read_page);
					if (read_page == E_PAGE_NOT_EXIST_IN_PF) {
						if (!((fault_va >= USER_HEAP_START
								&& fault_va < USER_HEAP_MAX)
								|| (fault_va <= USTACKTOP
										&& fault_va > USTACKBOTTOM))) {
						//	cprintf("here in read_page kill\n");
							unmap_frame(curenv->env_page_directory, fault_va);
							sched_kill_env(curenv->env_id);
						}
					}

					LIST_INSERT_HEAD(&(curenv->ActiveList), new_workingset);
					pt_set_page_permissions(curenv->env_page_directory,
							new_workingset->virtual_address, PERM_PRESENT, 0);
					return;
				}

			}

		}
	}
}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va) {
	panic("this function is not required...!!");
}
