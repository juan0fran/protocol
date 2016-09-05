#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <assert.h>
#include <sys/time.h>
#include <stdint.h>
#include <protocol/StopAndWait.h>
#include <util/socket_utils.h>
#include <interfaces/packet.h>

//#define

static void printf_packet (BYTE * buff, int len){
	printf("Length: %d\n", len);
}

int write_to_net(int fd, BYTE * p, int len){
	/* Here we can just forward the packet */
	/* Example using socat -> simply write */
#ifdef __EXAMPLE_WITH_SOCAT__
	write(fd, p, len);
#else
	//write(fd, &len, sizeof(int32_t));
	write(fd, p, len);
#endif
	return 0;
}

int check_headers_net(BYTE * p, int * len){
	/* now this does nothing */
	return 0;
}

int read_packet_from_net(int fd, BYTE * p, int timeout){
	struct pollfd pfd;
	int len;
	int rv;
	int ret;
	int timeout_count = 0;
	pfd.fd = fd;
	pfd.events = POLLIN;
	rv = poll(&pfd, 1, timeout);
	if (rv == -1){
		perror("poll");
		return -1;
	}else if (rv == 0){
		printf("Timeout\n");
	}else{
		if (pfd.revents & POLLIN){
			/* In case of socat -> read until \n */
		#ifndef __EXAMPLE_WITH_SOCAT__
			/* ret = read(fd, &len, sizeof(int32_t)); */
			/* if (ret != sizeof(int32_t))
				return -1; */
			ret = read(fd, p, MTU_SIZE);
			/*if (ret > 0){
				while (ret != len){
					ret += read(fd, p+ret, len);
				}
			}*/
		#else
			ret = 0;
			len = read(fd, p, 1);
			if (len <= 0)
				return -1;
			printf("%c", p[len-1]);
			while (p[len-1] != '\n'){
				ret = read(fd, p+len, 1);
				if (ret  <= 0)
					return -1;
				len += ret;
				printf("%c", p[len-1]);
			}
		#endif
			printf("Received packet from net-> ");
			//printf_packet(p, len);
			printf("Read %d bytes from device %s\n", nread, "tun0");
			return len;
		}	
		/* packet read! */
	}
	return 0;
}