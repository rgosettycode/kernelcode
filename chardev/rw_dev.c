#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main()
{
	int fd;
	char data[11];
	char out[32] = "Hello World";

	fd = open("/dev/mychardev-1", O_RDWR);
#if 1
	read(fd, data, 10);

	data[10] = '\0';

	printf("read from device: %s\n", data);

	sleep(5);
#endif
	write(fd, out, 20);

	sleep(5);
	ioctl(fd, 1, data);

	close(fd);
	return 0;
}
