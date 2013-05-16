//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// jkg_threading.h
// JKG Multi-Threading Wrapper Library
// WIN32 only
// (c) 2013 Jedi Knight Galaxies

#ifndef _JKG_THREADING
#define _JKG_THREADING

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#undef INFINITE
#include <windows.h>
#endif

#ifdef __linux__
#include <pthread.h>
#endif

#ifdef _WIN32
extern CRITICAL_SECTION cs_G_Printf;
#endif

#define MAX_ASYNC_TASKS 1024

#define TASKFLAG_MALLOCED_INITDATA	0x00000001
#define TASKFLAG_MALLOCED_FINALDATA	0x00000002
#define TASKFLAG_JSON_INITDATA		0x00000004
#define TASKFLAG_JSON_FINALDATA		0x00000008
#define TASKFLAG_PENDING			0x00001000		// Set this flag in the thread func to stop the task from being 
													// finalized after returning
													// To finish the task, set the task state to TASKSTATE_DATACOLLECT
#define TASKFLAG_RESET				0x00002000		// Set this flag in the thread func to restart the task in the next frame
													// This flag is automatically removed
#define TASKFLAG_UITASK				0x01000000		// This task has been created by UI (absense means it's been created by cgame)
//#define TASKFLAG_PROCESS_ANYTIME	0x00000004
//#define TASKFLAG_NEVERFREE		0x00000008



typedef enum {
	TASKSTATE_FREE = 0,		// Free, ready to be created.
	TASKSTATE_CREATING,		// Locked for creation.
	TASKSTATE_QUEUED,		// Waiting in the processing queue.
	TASKSTATE_PROCESSING,	// Processing the task.
	TASKSTATE_DATACOLLECT,	// Task ended, waiting for data to be used.
	TASKSTATE_FINISHED		// Finished, task manager will free memory.
} taskState_t;

typedef struct asyncTask_s {
	taskState_t		state;			// Task state
	int				createTime;		// Make sure tasks are processed in the order they were created
	int				flags;			// Misc flags for the task
	void			*initData;		// Generic data buffer pointer, assigned on creation
	void			*finalData;		// Generic data buffer pointer, assigned in processing
	int				(*threadFunc)(struct asyncTask_s *taskPointer);	// Actual function to process the task
	int				errorCode;		// Error code received from the threadFunc
	void			(*finalCallback)(struct asyncTask_s *taskPointer);	// Callback function gets called from main thread on task finish

#ifdef _DEBUG
	int	inList;		// Debugging purposes only: Whether or not this task is on a queue or list
#endif

	struct asyncTask_s *next;		// Used by stacks/queues
} asyncTask_t;

void JKG_ThreadSleep ( int msec );

void JKG_MainThreadPoller ( int ui, int purge );
void JKG_PrintTasksTable ( );
void JKG_InitThreading ( void );
void JKG_ShutdownThreading ( int maxWaitTime );
int testThreadFunc (struct asyncTask_s *taskPointer);
asyncTask_t *JKG_NewAsyncTask ( int (*threadFunc)(struct asyncTask_s *taskPointer), void *initData, int flags, void (*finalCallback)(struct asyncTask_s *taskPointer), int ui );

void		JKG_Task_Init();
void		JKG_Task_Terminate();
asyncTask_t *JKG_Task_New();
void		JKG_Task_Free(asyncTask_t *task);
void		JKG_Task_Queue(asyncTask_t *task);
asyncTask_t *JKG_Task_GetQueued();
void		JKG_Task_DelayTask(asyncTask_t *task);
void		JKG_Task_ProcessDelayedTasks();
void		JKG_Task_Finished(asyncTask_t *task);
asyncTask_t *JKG_Task_GetFinished();

#endif