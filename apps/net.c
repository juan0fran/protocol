#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <util/socket_utils.h>
#include <interfaces/packet.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <string.h>

#define MAX_RANDOM  ((1L << 31L) - 1L)
int HasToBeSent(long PER){
	long n;
	do {
	    n = random();
	} while (n > (MAX_RANDOM - ((MAX_RANDOM % 100) + 1) % 100));
	n %= 100;
	if (n > PER){
		/* nop */
		return 1;
	}else{
		/* yup */
		return 0;
	}
}

int main(int argc, char ** argv){
	int flag;
	int rv;
	int ret;
	struct pollfd ufds[2];
	int counter[2];
	BYTE buffer[MTU_SIZE + MTU_OVERHEAD];
	FILE * fp;
	int net1;
	int net2;
	int net1_send;
	int net2_send;

	if (argc < 3){
		printf("now enough argc\n");
		exit(-1);
	}
	printf("ARGC is: %d\n", argc);
	if (argc == 3){
		net1 = initialise_client_socket(argv[1]);
		net1_send = atoi(argv[2]);
		net2_send = 100;
	}else if (argc == 5){
		net1 = initialise_client_socket(argv[1]);
		net2 = initialise_client_socket(argv[2]);	
		net1_send = atoi(argv[3]);
		net2_send = atoi(argv[4]);
	}else{
		exit(1);
	}
	
	int len;
	printf("Client has been initialised\n");
 	srandom((unsigned)time(NULL));
 	flag = 0;
 	counter[0] = 0;
 	counter[1] = 0;
 	unlink("recv_from_1.txt");
 	unlink("recv_from_0.txt");
	ufds[0].fd = net1;
	ufds[0].events = POLLIN; // check for normal data
	ufds[1].fd = net2;
	ufds[1].events = POLLIN; // check for normal data

	while(1){
		if ((counter[0]) == 5000){
			exit(0);
		}
		rv = poll(ufds, (argc-1), 250);
		if (rv == -1){
			perror("poll");
			/* just return ... */
		}else if(rv == 0){
			/* If there is nothing, put some in a net */
			if (HasToBeSent(net1_send)){
				sprintf((char *)buffer, "%d coutner <-- this is the test sequence to be sent\n", ++counter[0]);
				printf("Sending from 0: %s", buffer);
				len = strlen((const char *)buffer) + 1;
				write(net1, &len, sizeof(int));
				write(net1, buffer, len);
			}
			if(HasToBeSent(net2_send)){
				sprintf((char *)buffer, "%d coutner <-- this is the test sequence to be sent\n", ++counter[1]);
				printf("Sending from 1: %s", buffer);
				len = strlen((const char *)buffer) + 1;
				write(net2, &len, sizeof(int));
				write(net2, buffer, len);
			}
			/* This will also simulate transmission time */
			/* a packet every 500 ms, each side will send a packet every second */
		}else{
			if (HasToBeSent(net1_send)){
				sprintf((char *)buffer, "%d coutner <-- this is the test sequence to be sent\n", ++counter[0]);
				printf("Sending from 0: %s", buffer);
				len = strlen((const char *)buffer) + 1;
				write(net1, &len, sizeof(int));
				write(net1, buffer, len);
			}
			if(HasToBeSent(net2_send)){
				sprintf((char *)buffer, "%d coutner <-- this is the test sequence to be sent\n", ++counter[1]);
				printf("Sending from 1: %s", buffer);
				len = strlen((const char *)buffer) + 1;
				write(net2, &len, sizeof(int));
				write(net2, buffer, len);
			}			
			if (argc == 5){
				if (ufds[1].revents & POLLIN){
					ret = read(ufds[1].fd, &len, sizeof(int));
					if (ret <= 0){
						break;
					}
					read(ufds[1].fd, buffer, len);
					printf("Received from 1: %s", buffer);
					fp = fopen("recv_from_1.txt", "a+");
					if (fp != NULL){
						fprintf(fp, "%s", buffer);
						fclose(fp);
					}
				}
			}
			if (ufds[0].revents & POLLIN){
				ret = read(ufds[0].fd, &len, sizeof(int));
				if (ret <= 0){
					break;
				}
				read(ufds[0].fd, buffer, len);
				printf("Received from 0: %s", buffer);
				fp = fopen("recv_from_0.txt", "a+");
				if (fp != NULL){
					fprintf(fp, "%s", buffer);
					fclose(fp);
				}
			}
			/*usleep(50 * 1000);*/
		}
	}
}


