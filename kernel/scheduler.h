struct scheduler{
    void (*initialize)();
    int (*get)();
    void (*put)(int);
};