#define TIMER_SOURCE_USER 1
#define TIMER_SOURCE_KERNEL 0
#define REASON_TIME_SLICE_EXPIRED 0
#define REASON_AWAKENED 1
#define SCHEDULER_DRR 0
#define SCHEDULER_SJF 1
#define SCHEDULER_SJF_PREEMPRIVE 2
#define SCHEDULER_SJF_EAGER_PREEMPRIVE 3
#define SCHEDULER_CFS 4
#define AFFINITY_ENABLED 1
#define AFFINITY_ENABLED_AGING 2
#define AFFINITY_DISABLED 0

struct schedulingStrategy{
    void (*initialize)();
    void (*perCoreInitialize)(int);
    int (*get)(int core);
    void (*put)(int,int);
    void (*timer)(int);
};

void sched_initialize();
void sched_perCoreInitialize(int core);
int sched_get(int core);
void sched_put(int processIndex,int reason);
void sched_timer(int user);
int sched_change(int type,int factor,int timeslice,int ease);
int sched_affinity(int enabled,int initialLaziness, int maxAge);

extern struct spinlock sched_lock;
extern int runningProc;
extern int affinityEN;