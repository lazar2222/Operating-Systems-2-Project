#define TIMER_SOURCE_USER 1
#define TIMER_SOURCE_KERNEL 0
#define REASON_TIME_SLICE_EXPIRED 0
#define REASON_AWAKENED 1
#define SCHEDULER_DRR 0
#define SCHEDULER_SJF 1
#define SCHEDULER_SJF_PREEMPRIVE 2
#define SCHEDULER_SJF_EAGER_PREEMPRIVE 3
#define SCHEDULER_CFS 4

struct schedulingStrategy{
    void (*initialize)();
    void (*perCoreInitialize)(int);
    int (*get)();
    void (*put)(int,int);
    void (*timer)(int);
};

extern struct schedulingStrategy currentSchedulingStrategy;