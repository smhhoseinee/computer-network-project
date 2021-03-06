#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void main(void) { 
	int res; 
	struct sockaddr_in addr; 
	long arg; 
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon; 
	int soc;

	// Create socket 
	soc = socket(AF_INET, SOCK_STREAM, 0); 
	if (soc < 0) { 
		fprintf(stderr, "Error creating socket (%d %s)\n", errno, strerror(errno)); 
		exit(0); 
	} 

	addr.sin_family = AF_INET; 
	addr.sin_port = htons(443); 
	addr.sin_addr.s_addr = inet_addr("212.33.204.20"); 

	// Set non-blocking 
	if( (arg = fcntl(soc, F_GETFL, NULL)) < 0) { 
		fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		exit(0); 
	} 
	arg |= O_NONBLOCK; 
	if( fcntl(soc, F_SETFL, arg) < 0) { 
		fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		exit(0); 
	} 
	// Trying to connect with timeout 
	res = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); 
	if (res < 0) { 
		if (errno == EINPROGRESS) { 
			fprintf(stderr, "EINPROGRESS in connect() - selecting\n"); 
			do { 
				tv.tv_sec = 15; 
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(soc, &myset); 
				res = select(soc+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
					exit(0); 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						exit(0); 
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt) 
						       ); 
						exit(0); 
					} 
					break; 
				} 
				else { 
					fprintf(stderr, "Timeout in select() - Cancelling!\n"); 
					exit(0); 
				} 
			} while (1); 
		} 
		else { 
			fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
			exit(0); 
		} 
	} 
	// Set to blocking mode again... 
	if( (arg = fcntl(soc, F_GETFL, NULL)) < 0) { 
		fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		exit(0); 
	} 
	arg &= (~O_NONBLOCK); 
	if( fcntl(soc, F_SETFL, arg) < 0) { 
		fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		exit(0); 
	} 
	// I hope that is all 
}
