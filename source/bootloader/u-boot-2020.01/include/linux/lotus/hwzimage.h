
#ifndef __LOTUS_IMAGE_H_
#define __LOTUS_IMAGE_H_    1

#include <config.h>

/* HW GZIP Image head */
static inline bool check_hwzimage_header(unsigned char *header)
{
	unsigned int magic_num0 = *((unsigned int *)(header + HEAD_MAGIC_NUM0_OFFSET));
	unsigned int magic_num1 = *((unsigned int *)(header + HEAD_MAGIC_NUM1_OFFSET));
	if ((magic_num0 != HEAD_MAGIC_NUM0) || (magic_num1 != HEAD_MAGIC_NUM1))
		return false;

	return true;
}
#endif /* __LOTUS_IMAGE_H_ */
