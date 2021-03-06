#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/tcp.h>



/*
 * ask host or ip
 * input host or ip
 * convert host to ip
 * ask user 3 options 
 * 
 *
 *
 * */

const int MAX_IP_STR_LEN = 17;
const int MAX_PORT_STR_LEN = 7;
const int MAX_MSG_OUT_LEN = 128;
const int MAX_HOSTNAME_LEN=100;
const int NUMBER_OF_SERVICES = 10;
const int MIN_PORT_NUMBER= 1;
const int MAX_WELLKNOWN_PORT_NUMBER= 1023;
const int MAX_PORT_NUMBER= 65353;
const int MAX_NUMBER_OF_THREADS = 7;

int number_of_threads;
int timeout_in_sec;

struct  thread_scan_port_range_args {
	char *server_addr_str;
	in_port_t server_port_start;
	in_port_t server_port_end;
	int sock;
};
// (char *server_addr_str,in_port_t server_port_start,in_port_t server_port_end,      int sock)

int ask_what_to_do(char *server_addr_str, char *port_str);
int ask_what_to_do6(char *server_addr_str, char *port_str);

void remove_cr(char *str){
	for(int i=0;i<strlen(str); i++){
		if(str[i] == '\n'){
			str[i] = '\0';
			return;
		}
	}
}

in_port_t input_port(char *port_str){
	puts("SERVER PORT: [unit 16] ");
	fgets(port_str,MAX_PORT_STR_LEN,stdin);
	remove_cr(port_str);

	in_port_t server_port = atoi(port_str);

	return server_port;
}

struct addrinfo * input_host(char *server_addr_str,char *port_str ){

	struct addrinfo hints, *results, *p;
	struct sockaddr_in *ip_access;
	char *hostname = malloc(sizeof(char)*MAX_HOSTNAME_LEN);
	int addinfo_status;

	printf("Enter a Domain Name: \n");
	fgets(hostname,MAX_HOSTNAME_LEN,stdin);
	remove_cr(hostname);

	puts(hostname);
	// zero out structure
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((addinfo_status = getaddrinfo( hostname , "domain" , &hints ,  &results)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addinfo_status));
		return NULL;
	}

	//loops through structure using ai_next
	for(p = results; p != NULL; p = p->ai_next)
	{
		//pass data into sockaddr_in struct
		ip_access = (struct sockaddr_in *) p->ai_addr;
		printf("IP address is %s: \n",inet_ntoa( ip_access->sin_addr ) );
		server_addr_str = inet_ntoa(ip_access->sin_addr );
		ask_what_to_do(server_addr_str, port_str);

	}

	freeaddrinfo(results);


	printf("\n");


	return results;
}
int input_ipv6(char *server_addr_str ){
	puts("SERVER IPv6: [x::x:x:x:x (ipv6)] ");
	puts("i.e:fe80::250:56ff:fea7:864 ");
	fgets(server_addr_str,MAX_IP_STR_LEN,stdin);
	remove_cr(server_addr_str);
	return 0;
}

int input_ip(char *server_addr_str ){
	puts("SERVER IP: [x.x.x.x (ipv4)] ");
	fgets(server_addr_str,MAX_IP_STR_LEN,stdin);
	remove_cr(server_addr_str);
	return 0;
}
int create_socket6(){
	//socket() creation
	int sock = socket(AF_INET6, SOCK_STREAM, 0);

	struct timeval tv_out;
	tv_out.tv_sec = timeout_in_sec;
	tv_out.tv_usec = 0;

	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));
	if(sock < 0){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	return sock;
}

int create_socket(){
	//socket() creation
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct timeval tv_out;
	tv_out.tv_sec = timeout_in_sec;
	tv_out.tv_usec = 0;

	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));
	if(sock < 0){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	return sock;
}
int connect_socket6(int sock ,struct  sockaddr_in6 server_address){
        if(connect(sock, (const struct sockaddr *)&server_address, sizeof(server_address))<0){
		printf("port %d is not open\n", ntohs(server_address.sin6_port));
             //   perror("connection failed!");
             //   exit(EXIT_FAILURE);
        }else{
		printf("port %d is open\n", ntohs(server_address.sin6_port));
	}
	return 0;
}

int connect_socket(int sock ,struct  sockaddr_in server_address){
        if(connect(sock, (const struct sockaddr *)&server_address, sizeof(server_address))<0){
		printf("port %d is not open\n", ntohs(server_address.sin_port));
             //   perror("connection failed!");
             //   exit(EXIT_FAILURE);
        }else{
		printf("port %d is open\n", ntohs(server_address.sin_port));
	}
	return 0;
}


struct sockaddr_in create_struct_sockaddr(struct sockaddr_in server_address, char *server_addr_str, in_port_t server_port){
	//create struct sockaddr
	memset(&server_address,0,sizeof(server_address));
	server_address.sin_family = AF_INET;
        int numerical_address = inet_pton(AF_INET,server_addr_str, &server_address.sin_addr.s_addr);
//	printf("^^^^^^^^^^ ip is :  %d \n", ntohs(server_address.sin_addr.s_addr));
//	server_address.sin_addr.s_addr++;
//	printf("^^^^^^^^^^ ip is :  %d \n", ntohs(server_address.sin_addr.s_addr));
//	server_address.sin_addr.s_addr++;
//      inet_ntop(AF_INET, &server_address.sin_addr.s_addr ,server_addr_str, sizeof(char) * MAX_IP_STR_LEN );
//	printf("^^^^^^^^^^ ip is :  %s \n", server_addr_str);

	if(numerical_address == 0){
                fputs("invalid IPv4", stderr);
                exit(EXIT_FAILURE);
        }
        if(numerical_address < 0){
                fputs("p to n failed",stderr);
                exit(EXIT_FAILURE);
        }
        server_address.sin_port = htons(server_port);

	return server_address;
}

struct sockaddr_in6 create_struct_sockaddr6(struct sockaddr_in6 server_address, char *server_addr_str, in_port_t server_port){
	//create struct sockaddr
	memset(&server_address,0,sizeof(server_address));
	server_address.sin6_family = AF_INET6;
	puts("server_addr_str");
	puts(server_addr_str);
        int numerical_address = inet_pton(AF_INET6,server_addr_str, &server_address.sin6_addr);

	if(numerical_address == 0){
                fputs("invalid IPv6", stderr);
                exit(EXIT_FAILURE);
        }
        if(numerical_address < 0){
                fputs("p to n failed",stderr);
                exit(EXIT_FAILURE);
        }
        server_address.sin6_port = htons(server_port);

	return server_address;
}

int scan_port(char *server_addr_str,in_port_t server_port,int sock){
	struct sockaddr_in server_address;
	server_address = create_struct_sockaddr(server_address,server_addr_str,server_port);

//	int sock = create_socket();
	connect_socket(sock,server_address);


	return 0;
}

int scan_port6(char *server_addr_str,in_port_t server_port,int sock){
	struct sockaddr_in6 server_address;
	server_address = create_struct_sockaddr6(server_address,server_addr_str,server_port);

//	int sock = create_socket();
	connect_socket6(sock,server_address);


	return 0;
}

void *thread_scan_port_range(void *_args){
	struct thread_scan_port_range_args *args = (struct thread_scan_port_range_args *) _args;	

	char *server_addr_str = malloc(sizeof(char) * MAX_IP_STR_LEN);
	server_addr_str = args->server_addr_str;
	in_port_t server_port_start =  args->server_port_start;
	in_port_t server_port_end =  args->server_port_end;
	int sock = args->sock;

	for(int i = server_port_start ; i < server_port_end ; i++ ){
	//	printf("scan port %d started\n",i);
		scan_port(server_addr_str, i, sock);
	}

	free (args);
	pthread_exit (NULL);


	//scan_port(server_addr_str , i, sock);
}
void *thread_scan_port_range6(void *_args){
	struct thread_scan_port_range_args *args = (struct thread_scan_port_range_args *) _args;	

	char *server_addr_str = malloc(sizeof(char) * MAX_IP_STR_LEN);
	server_addr_str = args->server_addr_str;
	in_port_t server_port_start =  args->server_port_start;
	in_port_t server_port_end =  args->server_port_end;
	int sock = args->sock;

	for(int i = server_port_start ; i < server_port_end ; i++ ){
	//	printf("scan port %d started\n",i);
		scan_port6(server_addr_str, i, sock);
	}

	free (args);
	pthread_exit (NULL);


	//scan_port(server_addr_str , i, sock);
}


int scan_port_range(char *server_addr_str,in_port_t server_port_start, in_port_t server_port_end){

	int sock = create_socket();
	int number_of_ports = server_port_end - server_port_start + 1 ;
	int ports_of_each_thread = number_of_ports / number_of_threads ;
	int remained_ports = number_of_ports % number_of_threads ;


	int thread_counter = 0;
	int i;
	int result_code;


	pthread_t threads[number_of_threads];

	for(i = server_port_start; thread_counter < number_of_threads-1 ; i+=ports_of_each_thread){

		struct thread_scan_port_range_args *args = malloc (sizeof (struct thread_scan_port_range_args));
		args-> server_addr_str= server_addr_str;
		args-> server_port_start = i;
		args-> server_port_end = (i+ports_of_each_thread);
		args-> sock = sock;


		printf("thread %d started from %d to %d\n", (thread_counter+1), args->server_port_start, args->server_port_end);

		pthread_create(&threads[thread_counter], NULL, thread_scan_port_range, args);

		thread_counter++;


	}

	struct thread_scan_port_range_args *args = malloc (sizeof (struct thread_scan_port_range_args));
	args-> server_addr_str= server_addr_str;
	args-> server_port_start = i;
	args-> server_port_end = server_port_end;
	args-> sock = sock;

	printf("thread %d started from %d to %d\n", (thread_counter+1),args-> server_port_start, args->server_port_end);

	pthread_t thread_id;
	pthread_create(&threads[thread_counter], NULL, thread_scan_port_range, args);

	for(i = 0; i<number_of_threads;i++){
//		printf("IN MAIN: wait for Thread %d has ended.\n", i+1);
		result_code = pthread_join(threads[i], NULL);
		assert(!result_code);
		printf("IN MAIN: Thread %d has ended.\n", i+1);
	}

	return 0;
}


int scan_port_range6(char *server_addr_str,in_port_t server_port_start, in_port_t server_port_end){

	int sock = create_socket();
	int number_of_ports = server_port_end - server_port_start + 1 ;
	int ports_of_each_thread = number_of_ports / number_of_threads ;
	int remained_ports = number_of_ports % number_of_threads ;


	int thread_counter = 0;
	int i;
	int result_code;


	pthread_t threads[number_of_threads];

	for(i = server_port_start; thread_counter < number_of_threads-1 ; i+=ports_of_each_thread){

		struct thread_scan_port_range_args *args = malloc (sizeof (struct thread_scan_port_range_args));
		args-> server_addr_str= server_addr_str;
		args-> server_port_start = i;
		args-> server_port_end = (i+ports_of_each_thread);
		args-> sock = sock;


		printf("thread %d started from %d to %d\n", (thread_counter+1), args->server_port_start, args->server_port_end);

		pthread_create(&threads[thread_counter], NULL, thread_scan_port_range, args);

		thread_counter++;


	}

	struct thread_scan_port_range_args *args = malloc (sizeof (struct thread_scan_port_range_args));
	args-> server_addr_str= server_addr_str;
	args-> server_port_start = i;
	args-> server_port_end = server_port_end;
	args-> sock = sock;

	printf("thread %d started from %d to %d\n", (thread_counter+1),args-> server_port_start, args->server_port_end);

	pthread_t thread_id;
	pthread_create(&threads[thread_counter], NULL, thread_scan_port_range6, args);

	for(i = 0; i<number_of_threads;i++){
//		printf("IN MAIN: wait for Thread %d has ended.\n", i+1);
		result_code = pthread_join(threads[i], NULL);
		assert(!result_code);
		printf("IN MAIN: Thread %d has ended.\n", i+1);
	}

	return 0;
}




int input_port_range(char *server_addr_str){
	in_port_t server_port_start;
	in_port_t server_port_end;
	char *port_str = malloc(sizeof(char) * MAX_PORT_STR_LEN);

	puts("FROM PORT: [unit 16] ");
	fgets(port_str,MAX_PORT_STR_LEN,stdin);
	remove_cr(port_str);
	server_port_start = atoi(port_str);

	puts("TO PORT: [unit 16] ");
	fgets(port_str,MAX_PORT_STR_LEN,stdin);
	remove_cr(port_str);
	server_port_end = atoi(port_str);

	scan_port_range(server_addr_str,server_port_start,server_port_end);

	return 0;
}
int input_port_range6(char *server_addr_str){
	in_port_t server_port_start;
	in_port_t server_port_end;
	char *port_str = malloc(sizeof(char) * MAX_PORT_STR_LEN);

	puts("FROM PORT: [unit 16] ");
	fgets(port_str,MAX_PORT_STR_LEN,stdin);
	remove_cr(port_str);
	server_port_start = atoi(port_str);

	puts("TO PORT: [unit 16] ");
	fgets(port_str,MAX_PORT_STR_LEN,stdin);
	remove_cr(port_str);
	server_port_end = atoi(port_str);

	scan_port_range6(server_addr_str,server_port_start,server_port_end);

	return 0;
}



int input_and_scan_port(char *server_addr_str,char *port_str){
	in_port_t server_port = input_port(port_str);

	int sock = create_socket();
	scan_port(server_addr_str,server_port, sock);
	return 0;
}
int input_and_scan_port6(char *server_addr_str,char *port_str){
	in_port_t server_port = input_port(port_str);

	int sock = create_socket6();
	scan_port6(server_addr_str,server_port, sock);
	return 0;
}


int create_socket_and_scan_port(char *server_addr_str,char *port_str){
	in_port_t server_port = atoi(port_str);
	int sock = create_socket();
	scan_port(server_addr_str,server_port,sock);
	return 0;

}
int create_socket_and_scan_port6(char *server_addr_str,char *port_str){
	in_port_t server_port = atoi(port_str);
	int sock = create_socket6();
	scan_port6(server_addr_str,server_port,sock);
	return 0;

}


char* ask_port_service(){

	int choice;
	char*  port_of_service[NUMBER_OF_SERVICES];
	for(int i=0;i<NUMBER_OF_SERVICES;i++){
		port_of_service[i] = malloc(sizeof(char) * MAX_PORT_STR_LEN);
	}

	int i = 1;
	port_of_service[i]="80";
	i++;
	port_of_service[i]="443";
	i++;
	port_of_service[i]="25";
	i++;
	port_of_service[i]="21";
	i++;
	port_of_service[i]="23";
	i++;
	port_of_service[i]="22";
	i++;
	

	puts("select service you want to scan:\n");
	puts("1-HTTP(80)\n");
	puts("2-TLS(443)\n");
	puts("3-SMTP(25)\n");
	puts("4-FTP(21)\n");
	puts("5-TELNET(23)\n");
	puts("6-SSH(22)\n");
	

	choice = getchar();
	getc(stdin);
	choice -= '0';


	printf("you chose service %d \n",choice);

	return port_of_service[choice];
}

int port_service(char *server_addr_str,char *port_str){
	port_str = ask_port_service();
	create_socket_and_scan_port(server_addr_str,port_str);
	return 0;
}
int port_service6(char *server_addr_str,char *port_str){
	port_str = ask_port_service();
	create_socket_and_scan_port6(server_addr_str,port_str);
	return 0;
}

int ask_what_to_do6(char *server_addr_str, char *port_str){

	int choice;
	puts("select what you want:\n");
	puts("1-scan all ports\n");
	puts("2-just for well-known ports(0-1023)\n");
	puts("3-request for specific port \n");
	puts("4-request for specific services\n");
	puts("5-request for specific range\n");

	choice = getchar();
	getc(stdin);

	choice -= '0';

	printf("you chose %d \n",choice);

	switch(choice)
	{
		case 1:
			scan_port_range6(server_addr_str,MIN_PORT_NUMBER,MAX_PORT_NUMBER);
			break;
		case 2:
			scan_port_range6(server_addr_str,MIN_PORT_NUMBER,MAX_WELLKNOWN_PORT_NUMBER);
			break;
		case 3:

			input_and_scan_port6(server_addr_str,port_str);
			break;
		case 4:
			port_service6(server_addr_str,port_str);
			break;
		case 5:
			input_port_range6(server_addr_str);
			break;


			// operator doesn't match any case constant +, -, *, /
		default:
			printf("Error! operator is not correct");
	}
	return 0;
}

int ask_what_to_do(char *server_addr_str, char *port_str){

	int choice;
	puts("select what you want:\n");
	puts("1-scan all ports\n");
	puts("2-just for well-known ports(0-1023)\n");
	puts("3-request for specific port \n");
	puts("4-request for specific services\n");
	puts("5-request for specific range\n");

	choice = getchar();
	getc(stdin);

	choice -= '0';

	printf("you chose %d \n",choice);

	switch(choice)
	{
		case 1:
			scan_port_range(server_addr_str,MIN_PORT_NUMBER,MAX_PORT_NUMBER);
			break;

		case 2:
			scan_port_range(server_addr_str,MIN_PORT_NUMBER,MAX_WELLKNOWN_PORT_NUMBER);
			break;

		case 3:

			input_and_scan_port(server_addr_str,port_str);
			break;
		case 4:
			port_service(server_addr_str,port_str);
			break;
		case 5:

			input_port_range(server_addr_str);
			break;


			// operator doesn't match any case constant +, -, *, /
		default:
			printf("Error! operator is not correct");
	}
	return 0;
}


int ask_ip_or_host(char *server_addr_str, char *port_str){

	 int choice;
         puts("choose your input method:\n");
         puts("1-ipv4\n");
         puts("2-hostname(domain)\n");
         puts("3-ipv6\n");

	 choice = getchar();
	 getc(stdin);

         puts("choice done\n");
	 choice -= '0';

         puts("2-hostname(domain)\n");

	switch(choice)
	{
		case 1:
			printf("you chose ipv4\n");
			input_ip(server_addr_str );
			ask_what_to_do(server_addr_str, port_str);
			break;

		case 2:
			printf("you chose hostname\n");
			input_host(server_addr_str,port_str);
			//input host function
			break;
			// operator doesn't match any case constant +, -, *, /

		case 3:
			printf("you chose ipv6\n");
			input_ipv6(server_addr_str );
			ask_what_to_do6(server_addr_str, port_str);
			break;


		default:
			printf("Error! operator is not correct");
	}
       	return 0;
}

int main(int argc, char *argv[]){


	// get ip and port
	char *server_addr_str = malloc(sizeof(char) * MAX_IP_STR_LEN);
	char *port_str = malloc(sizeof(char) * MAX_PORT_STR_LEN);


	if(server_addr_str == NULL){
		fputs("memory allocation failed (ip)",stderr);
		exit(EXIT_FAILURE);
	}
	if(port_str == NULL){
		fputs("memory allocation failed (port)",stderr);
		exit(EXIT_FAILURE);
	}
	memset(server_addr_str, '\0' , MAX_IP_STR_LEN);
        memset(port_str, '\0', MAX_PORT_STR_LEN);


	//threads are specified in argc 
	if(argc > 1){
		//in_port_t number_of_threads = atoi(argv[1]);
		if(atoi(argv[1])>=MAX_NUMBER_OF_THREADS){
			number_of_threads = MAX_NUMBER_OF_THREADS;
		}else{
			number_of_threads = atoi(argv[1]);
		}

		if(argc > 2){//timeout also specified
			timeout_in_sec = atoi(argv[2]);
		}else{
			timeout_in_sec = 4;
		}
		printf("timeout=%d\n",timeout_in_sec);
	}else{//default threads
		number_of_threads = 1;
		printf("argc less than 1 \n");
	}


	ask_ip_or_host(server_addr_str, port_str);

	return 0; 
}
