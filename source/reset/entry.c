#include <main.h>
#include <startup.h>

void entry(void)
{
    // 调用CubeMX的HAL初始化代码
    main();

    // 启动rtthread
    rtthread_startup();
}