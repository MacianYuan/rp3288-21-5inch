#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <sys/utsname.h>
#include <cutils/list.h>
#include <cutils/sockets.h>
#include <cutils/iosched_policy.h>
#include <dirent.h>
#include <sys/stat.h>

#define LOG_TAG "RKWIFIHAL"

#include <log/log.h>

extern int wifi_load_driver();

int main(int argc, char *argv[]) {
	int ret;
	ret = wifi_load_driver();
	if(ret != 0) {
		ALOGD("insmod wifi ko failed\n");
	} else {
		ALOGD("insmod wifi ko success\n");
	}
	return 0;
}
