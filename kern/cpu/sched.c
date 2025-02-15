#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>

uint32 isSchedMethodRR() {
	if (scheduler_method == SCH_RR)
		return 1;
	return 0;
}
uint32 isSchedMethodMLFQ() {
	if (scheduler_method == SCH_MLFQ)
		return 1;
	return 0;
}
uint32 isSchedMethodBSD() {
	if (scheduler_method == SCH_BSD)
		return 1;
	return 0;
}

//===================================================================================//
//============================ SCHEDULER FUNCTIONS ==================================//
//===================================================================================//

//===================================
// [1] Default Scheduler Initializer:
//===================================
void sched_init() {
	old_pf_counter = 0;

	sched_init_RR(INIT_QUANTUM_IN_MS);

	init_queue(&env_new_queue);
	init_queue(&env_exit_queue);
	scheduler_status = SCH_STOPPED;
}

//=========================
// [2] Main FOS Scheduler:
//=========================
void fos_scheduler(void) {
	//	cprintf("inside scheduler\n");

	chk1();
	scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	if (scheduler_method == SCH_RR) {
		// Implement simple round-robin scheduling.
		// Pick next environment from the ready queue,
		// and switch to such environment if found.
		// It's OK to choose the previously running env if no other env
		// is runnable.

		//If the curenv is still exist, then insert it again in the ready queue
		if (curenv != NULL) {
			enqueue(&(env_ready_queues[0]), curenv);
		}

		//Pick the next environment from the ready queue
		next_env = dequeue(&(env_ready_queues[0]));

		//Reset the quantum
		//2017: Reset the value of CNT0 for the next clock interval
		kclock_set_quantum(quantums[0]);
		//uint16 cnt0 = kclock_read_cnt0_latch() ;
		//cprintf("CLOCK INTERRUPT AFTER RESET: Counter0 Value = %d\n", cnt0 );

	} else if (scheduler_method == SCH_MLFQ) {
		next_env = fos_scheduler_MLFQ();
	} else if (scheduler_method == SCH_BSD) {
		next_env = fos_scheduler_BSD();
	}
	//temporarily set the curenv by the next env JUST for checking the scheduler
	//Then: reset it again
	struct Env* old_curenv = curenv;
	curenv = next_env;
	chk2(next_env);
	curenv = old_curenv;

	//sched_print_all();

	if (next_env != NULL) {
		//		cprintf("\nScheduler select program '%s' [%d]... counter = %d\n", next_env->prog_name, next_env->env_id, kclock_read_cnt0());
		//		cprintf("Q0 = %d, Q1 = %d, Q2 = %d, Q3 = %d\n", queue_size(&(env_ready_queues[0])), queue_size(&(env_ready_queues[1])), queue_size(&(env_ready_queues[2])), queue_size(&(env_ready_queues[3])));
		env_run(next_env);
	} else {
		/*2015*///No more envs... curenv doesn't exist any more! return back to command prompt
		curenv = NULL;
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);

		//cprintf("SP = %x\n", read_esp());

		scheduler_status = SCH_STOPPED;
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

//=============================
// [3] Initialize RR Scheduler:
//=============================
void sched_init_RR(uint8 quantum) {

	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
#if USE_KHEAP
	sched_delete_ready_queues();
	env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8));
#endif
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	init_queue(&(env_ready_queues[0]));

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;
	//=========================================
	//=========================================
}

//===============================
// [4] Initialize MLFQ Scheduler:
//===============================
void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel) {
#if USE_KHEAP
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	//=========================================
	//=========================================

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================
#endif
}

//===============================
// [5] Initialize BSD Scheduler:
//===============================
void sched_init_BSD(uint8 numOfLevels, uint8 quantum) {
#if USE_KHEAP
	//TODO: [PROJECT'23.MS3 - #4] [2] BSD SCHEDULER - sched_init_BSD
	//Your code is here
	//Comment the following line

	//panic("Not implemented yet");

	num_of_ready_queues = numOfLevels;
	env_ready_queues = kmalloc(sizeof(struct Env_Queue) * numOfLevels);
	quantums = kmalloc(sizeof(uint8));
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);

	for (int i = 0; i < numOfLevels; i++) {
		init_queue(&(env_ready_queues[i]));
	}
	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_BSD;
	//=========================================
	//=========================================

#endif
}

//=========================
// [6] MLFQ Scheduler:
//=========================
struct Env* fos_scheduler_MLFQ() {
	panic("not implemented");
	return NULL;
}

//=========================
// [7] BSD Scheduler:
//=========================
struct Env* fos_scheduler_BSD() {
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - fos_scheduler_BSD
	//Your code is here
	//Comment the following line
	//panic("Not implemented yet");


	struct Env* next_Env;
	struct Env* tmp;
//	LIST_FOREACH(tmp , &env_ready_queues){
//		//tmp->
//	}
//	cprintf("208 size: %d\n208 numofReadyQ: %d\n",env_ready_queues[0].size,num_of_ready_queues);

	//[1] Place the curenv (if any) in its correct queue
	if (curenv != NULL) {
		struct Env* curtmp = curenv;
		int num_of_processes_per_queue = (PRI_MAX / num_of_ready_queues);
		if (curenv->priority > PRI_MAX)
			curenv->priority = PRI_MAX;
		else if (curenv->priority < PRI_MIN)
			curenv->priority = PRI_MIN;

		for(int i = 0 ; i < num_of_ready_queues; i++){
			curtmp = find_env_in_queue(&env_ready_queues[i],curenv->env_id);
			if(curtmp != NULL){
				remove_from_queue(&env_ready_queues[i],curtmp);
				break;
			}
		}

		for (int j = 0; j < num_of_ready_queues - 1; j++) {
			if (j != 0) {
				if (curenv->priority >= (j * num_of_processes_per_queue) + 1
						&& curenv->priority
								<= ((j + 1) * num_of_processes_per_queue)) {
					//cprintf("292\n");
					enqueue(&env_ready_queues[j], curenv);
					break;
				} else if (j == num_of_ready_queues - 2) {
					//cprintf("296\n");
					enqueue(&env_ready_queues[j + 1], curenv);
					break;
				}
			} else {
				if (curenv->priority >= (j * num_of_processes_per_queue)
						&& curenv->priority
								<= ((j + 1) * num_of_processes_per_queue)) {
					//cprintf("309\n");
					enqueue(&env_ready_queues[j], curenv);
					break;
				}
			}
		}
	}


	// [2] Search for the next env in the queues according to their priorities
	for (int i = num_of_ready_queues - 1; i >= 0; i--) {
		//cprintf("208 size %d: %d\n\n", i + 1, env_ready_queues[i].size);
		if (/*env_ready_queues[i] != NULL || */env_ready_queues[i].size != 0) {
			// [3] If next env is found:
			// 		1. Set CPU quantum
			//      2. Remove the selected env from its queue and return it
			// Else,
			// 		1. Reset load_avg for next run
			// 		2. return NULL
			next_Env = dequeue(&(env_ready_queues[i]));
			tmp = next_Env;
			kclock_set_quantum(quantums[0]);
			return next_Env;
		}
	}
//	cprintf("208 return Null\n\n");
	load_avg = fix_int(0);
	return NULL;
}

//========================================
// [8] Clock Interrupt Handler
//	  (Automatically Called Every Quantum)
//========================================
void clock_interrupt_handler() {
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - Your code is here
	{
		int64 time = timer_ticks();
//		cprintf("275 time\n\n\n\n\n",time);
//		if(quantums[0]!= NULL)
		int num_of_ticks_perSecond = (1000 / quantums[0]);

		/* every tick */
		// update recent cpu for the running processes
		curenv->recent_cpu = curenv->recent_cpu + 1;

		int num_of_processes_per_queue = (PRI_MAX / num_of_ready_queues);

		if (/*4 ticks*/time % 4 == 0 /*&& time != 0*/) {
//			cprintf("time every 4: %d\n", time);
			//cprintf("timer 4\n");
			for (int i = 0; i < num_of_ready_queues; i++) {
				struct Env* envTmp;
				LIST_FOREACH(envTmp,&env_ready_queues[i])
				{
					//cprintf("22\n");

					// recentCPU / 4
					fixed_point_t x1 = fix_div(
							fix_int(env_get_recent_cpu(envTmp)), fix_int(4));
					// nice * 2
					fixed_point_t x2 = fix_mul(fix_int(envTmp->nice),
							fix_int(2));

					//					cprintf("Priority Before change: %d \n",envTmp->priority);

					// priority = PRI_MAX - (recentCPU /4) - (nice * 2)
					envTmp->priority = PRI_MAX - fix_round(fix_sub(x1, x2));
					if (envTmp->priority > PRI_MAX)
						envTmp->priority = PRI_MAX;
					else if (envTmp->priority < PRI_MIN)
						envTmp->priority = PRI_MIN;
					struct Env* tmp = envTmp;
					for (int j = 0; j < num_of_ready_queues - 1; j++) {
						if (j != 0) {
							if (envTmp->priority
									>= (j * num_of_processes_per_queue) + 1
									&& envTmp->priority
											<= ((j + 1)
													* num_of_processes_per_queue)) {
								//cprintf("292\n");
								remove_from_queue(&env_ready_queues[i], tmp);
								enqueue(&env_ready_queues[j], envTmp);
								break;
							} else if (j == num_of_ready_queues - 2) {
								//cprintf("296\n");
								remove_from_queue(&env_ready_queues[i], tmp);
								enqueue(&env_ready_queues[j + 1], envTmp);
								break;
							}
						} else {
							if (envTmp->priority
									>= (j * num_of_processes_per_queue)
									&& envTmp->priority
											<= ((j + 1)
													* num_of_processes_per_queue)) {
								//cprintf("309\n");
								remove_from_queue(&env_ready_queues[i], tmp);
								enqueue(&env_ready_queues[j], envTmp);
								break;
							}
						}
					}
					//					cprintf("Priority After change: %d \n",envTmp->priority);
					//					fixed_point_t x1 = fix_div(fix_int(envTmp->recent_cpu),
					//							fix_int(4));
					//					fixed_point_t x2 = fix_mul(fix_int(envTmp->nice),
					//							fix_int(2));
					//					envTmp->priority = PRI_MAX - fix_round(fix_sub(x1, x2));
				}
				/*for (int j = 0; j < env_ready_queues[i].size; j++) {
				 // update priority
				 env_ready_queues[i]
				 }*/
			}
		}

		if (/*seconds*//*time != 0 &&*/ time % num_of_ticks_perSecond == 0) {
//			cprintf("time every 1 sec: %d\n", time);
			int number_Of_RunningyOrReady = 0;
			for (int i = 0; i < num_of_ready_queues; i++) {
				number_Of_RunningyOrReady += env_ready_queues[i].size;
			}
			for (int i = 0; i < num_of_ready_queues; i++) {
				struct Env* envTmp;
				LIST_FOREACH(envTmp,&env_ready_queues[i])
				{
					// update load avg & all recent cpu

					/* update Load_Avg */
					// 59/60
					fixed_point_t l1 = fix_div(fix_int(59), fix_int(60));
					// 1/60
					fixed_point_t l2 = fix_div(fix_int(1), fix_int(60));
					// 59/60 * load_avg(t-1)
					fixed_point_t l3 = fix_mul(l1, load_avg);
					// 1/60 * ready(t)
					fixed_point_t l4 = fix_mul(l2,
							fix_int(number_Of_RunningyOrReady));
					load_avg = l4;

					/* update Recent_Cpu */
//					cprintf("11\n");
					// 2 * load_avg
					fixed_point_t x1 = fix_scale(load_avg, 2);
					// 2 * load_avg + 1
					fixed_point_t x2 = fix_add(x1, fix_int(1));
					// round or not ?? -> (2*load_avg)/(2*load_avg+1)
					fixed_point_t x3 = fix_div(x1, x2);
					// (2*load_avg)/(2*load_avg+1) * recent_cpu
					fixed_point_t x4 = fix_mul(x3, fix_int(envTmp->recent_cpu));
					// (2*load_avg)/(2*load_avg+1) * recent_cpu + nice
					fixed_point_t x5 = fix_add(x4, fix_int(envTmp->nice));
					envTmp->recent_cpu = fix_round(x5);

				}
			}
		}
	}

	/********DON'T CHANGE THIS LINE***********/
	ticks++;
	if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX)) {
		update_WS_time_stamps();
	}
//cprintf("Clock Handler\n") ;
	fos_scheduler();
	/*****************************************/
}

//===================================================================
// [9] Update LRU Timestamp of WS Elements
//	  (Automatically Called Every Quantum in case of LRU Time Approx)
//===================================================================
void update_WS_time_stamps() {
	struct Env *curr_env_ptr = curenv;

	if (curr_env_ptr != NULL) {
		struct WorkingSetElement* wse;
		{
			int i;
#if USE_KHEAP
			LIST_FOREACH(wse, &(curr_env_ptr->page_WS_list))
			{
#else
				for (i = 0; i < (curr_env_ptr->page_WS_max_size); i++)
				{
					wse = &(curr_env_ptr->ptr_pageWorkingSet[i]);
					if( wse->empty == 1)
					continue;
#endif
				//update the time if the page was referenced
				uint32 page_va = wse->virtual_address;
				uint32 perm = pt_get_page_permissions(
						curr_env_ptr->env_page_directory, page_va);
				uint32 oldTimeStamp = wse->time_stamp;

				if (perm & PERM_USED) {
					wse->time_stamp = (oldTimeStamp >> 2) | 0x80000000;
					pt_set_page_permissions(curr_env_ptr->env_page_directory,
							page_va, 0, PERM_USED);
				} else {
					wse->time_stamp = (oldTimeStamp >> 2);
				}
			}
		}

		{
			int t;
			for (t = 0; t < __TWS_MAX_SIZE; t++) {
				if (curr_env_ptr->__ptr_tws[t].empty != 1) {
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->__ptr_tws[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->__ptr_tws[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr->env_page_directory,
							table_va)) {
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp
								>> 2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr->env_page_directory,
								table_va);
					} else {
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp
								>> 2);
					}
				}
			}
		}
	}
}

