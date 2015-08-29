/*
 *  hello.c
 *  Copyright © 2015 QSI Inc.
 *  All rights reserved.
 *  
 *       Author : Lester Yang <sab7412@gmail.com>
 *  Description : 
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/printk.h>

MODULE_DESCRIPTION("Hello World");
MODULE_AUTHOR("Lester Yang");
MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
    pr_err("hello init\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_err("hello exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
