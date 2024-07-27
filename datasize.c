/* Linux Device Drivers 2nd Edition, Corbet, Rubini, Kroah-Hartman */
/*
 * datasize.c -- print the size of common data items
 *
 * This runs with any Linux kernel (not any Unix, because of <linux/types.h>)
 */
#include <stdio.h>
#include <sys/utsname.h>
#include <linux/types.h>
#include <stdint.h>

int main(void)
{
    struct utsname name;

    uname(&name); /* never fails :) */
    printf("arch   Size:  char  short  int  long   ptr long-long "
	   " u8 u16 u32 u64\n");
    printf(       "%-12s  %3i   %3i   %3i   %3i   %3i   %3i      "
	   "%3i %3i %3i %3i\n",
	   name.machine,
	   (int)sizeof(char), (int)sizeof(short), (int)sizeof(int),
	   (int)sizeof(long),
	   (int)sizeof(void *), (int)sizeof(long long), (int)sizeof(__u8),
	   (int)sizeof(__u16), (int)sizeof(__u32), (int)sizeof(__u64));

    printf("uint_fast32_t      uint_least32_t     uint32_t\n");
    printf(       "%3i   %3i   %3i\n", (int)sizeof(uint_fast32_t), (int)sizeof(uint_least32_t), (int)sizeof(uint32_t));

    return 0;
}
