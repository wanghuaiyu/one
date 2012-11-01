/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>
#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#endif

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif

#ifdef RT_USING_DFS
#include <dfs_init.h>
#include <dfs_fs.h>
#include <dfs_elm.h>
#endif

void rt_init_thread_entry(void* parameter)
{
	finsh_system_init();
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
	/* initialize the device file system */
	dfs_init();
	/* initialize the elm chan FatFS file systam*/
	elm_init();

    rt_platform_init();

    /* Filesystem Initialization */
#ifdef RT_USING_DFS
    {
#ifdef RT_USING_DFS_ELMFAT
        /* mount sd card fat partition 1 as root directory */
        if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
        {
            rt_kprintf("File System initialized!\n");
        }
        else
            rt_kprintf("File System initialzation failed!\n");
#endif
    }
#endif

    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        /* register ethernetif device */
        eth_system_device_init();

    	rt_wlan_init();
    	rt_kprintf("wlan initialized\n");

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");

    	WlanDirectConnect();
		wlan_pm_exit();
    }
#endif
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
		rt_init_thread_entry, RT_NULL,
		2048, RT_THREAD_PRIORITY_MAX/3, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

/*@}*/
