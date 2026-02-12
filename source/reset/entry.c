#include <main.h>
#include <startup.h>

void entry(void)
{
    // 初始化HAL库并配置MCU
    main();

    // 启动rtthread
    rtthread_startup();
}