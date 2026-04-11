#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/blk-mq.h>
#include <linux/delay.h>

static struct kprobe kp = {
    .symbol_name = "blk_mq_start_request",
};

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    udelay(10); 
    
    static unsigned long last_log = 0;
    if (jiffies - last_log > HZ * 2) {
        printk(KERN_INFO "LAB4_KERNEL: [IO-PROBE] Задержка внесена! Диск работает.\n");
        last_log = jiffies;
    }

    return 0;
}

static int __init lab4_init(void) {
    kp.pre_handler = handler_pre;
    return register_kprobe(&kp);
}
static void __exit lab4_exit(void) { unregister_kprobe(&kp); }
module_init(lab4_init);
module_exit(lab4_exit);
MODULE_LICENSE("GPL");
