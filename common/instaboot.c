#include <common.h>
#include <amlogic/storage_if.h>
#include <amlogic/instaboot.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
struct instaboot_header {
	char reserved[PAGE_SIZE - 10 -
		sizeof(struct new_utsname) - sizeof(u32)];
	struct new_utsname uts;
	u32 version_code;
	char	sig[10];
} __attribute__((packed));

static struct instaboot_header ib_header;

int get_instaboot_header(struct instaboot_info* ib_info)
{
	int rc = 0;
	u64 partSz = 0;

	rc = store_get_partititon_size((unsigned char*)"instaboot", &partSz);
	if (rc || !partSz) {
	    printf("can not get instaboot part size\n");
	    return -1;
	}
	rc = store_read_ops((unsigned char*)"instaboot",
		(unsigned char*)&ib_header, 0, PAGE_SIZE);
	if (rc) {
	    printf("read instaboo header error\n");
	    return -1;
	}
	if (!strncmp(ib_header.sig, INSTABOOT_SIG, strlen(INSTABOOT_SIG))) {
		memcpy(&ib_info->uts, &ib_header.uts, sizeof(struct new_utsname));
		ib_info->version_code = ib_header.version_code;
	} else {
		printf("signature: %s\n", ib_header.sig);
		return -1;
	}

	return 0;
}