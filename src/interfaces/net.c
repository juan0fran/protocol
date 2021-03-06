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

#include <util/log.h>

//#define
#undef __EXAMPLE_WITH_SOCAT__

static void printf_packet (BYTE * buff, int len){
	printf("Length: %d\n", len);
}

static int input_timeout (int filedes, unsigned int milliseconds){
	fd_set set;
	struct timeval timeout;
	/* Initialize the file descriptor set. */
	FD_ZERO (&set);
	FD_SET (filedes, &set);

	/* Initialize the timeout data structure. */
	timeout.tv_sec = 0;
	timeout.tv_usec = milliseconds * 1000; /* microsec*1000

	/* select returns 0 if timeout, 1 if input available, -1 if error. */
	return (select (FD_SETSIZE, &set, NULL, NULL, &timeout));
}

int write_to_net(int fd, BYTE * p, int len){
	/* Here we can just forward the packet */
	/* Example using socat -> simply write */
#ifdef __EXAMPLE_WITH_SOCAT__
	write(fd, p, len);
#else
	/* At the end we are writing ip packets, so no need to send the Length since is in the IP packet */
#ifdef __MAC_OS_X__
	write(fd, &len, sizeof(int32_t));
#endif
	write(fd, p, len);
#endif
	return 0;
}

int check_headers_net(BYTE * p, int * len){
	/* now this does nothing */
	return 0;
}

void flush_net(int fd){
	BYTE buffer[MTU_SIZE + MTU_OVERHEAD];
	while(input_timeout(fd, 0) > 0){
		read(fd, buffer, MTU_SIZE + MTU_OVERHEAD);
	}
}

int read_packet_from_net(int fd, BYTE * p, int timeout){
	struct pollfd pfd;
	int len;
	uint16_t ip_len;
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
		log_message(LOG_INFO, "Timeout reading from net");
	}else{
		if (pfd.revents & POLLIN){
			/* In case of socat -> read until \n */
		#ifndef __EXAMPLE_WITH_SOCAT__
			/* ret = read(fd, &len, sizeof(int32_t)); */
			/* if (ret != sizeof(int32_t))
				return -1; */
			/* we are reading IP packets, the Length is inside the IP header */
			/* we will read 4 bytes, those will be version and flag bytes -> pass directly */
			/* we will read the sie, is a uint16_t */
		#ifndef __MAC_OS_X__
			ret = read(fd, p, MTU_SIZE);
			if (ret <= 0){
				return -1;
			}
			memcpy(&ip_len, p+2, sizeof(uint16_t));
			len = (int) ntohs(ip_len);
		#else
			ret = read(fd, &len, sizeof(int32_t));
			if (ret != sizeof(int32_t))
				return -1;
			ret = read(fd, p, len);
		#endif
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
			log_message(LOG_DEBUG, "Received packet from net -> ");
			//printf_packet(p, len);
			log_message(LOG_INFO, "Read %d bytes from device %s\n", len, "tun0");
			return len;
		}	
		/* packet read! */
	}
	return 0;
}