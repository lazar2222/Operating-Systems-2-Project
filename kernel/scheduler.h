#define TIMER_SOURCE_USER 1
#define TIMER_SOURCE_KERNEL 0
#define REASON_TIME_SLICE_EXPIRED 0
#define REASON_AWAKENED 1

struct schedulingStrategy{
    void (*initialize)();
    void (*perCoreInitialize)(int);
    int (*get)();
    void (*put)(int,int);
    void (*timer)(int);
};

extern struct schedulingStrategy currentSchedulingStrategy;