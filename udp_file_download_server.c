/* udp_file_download_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

struct PDU {
	char type;
	char data[100];
};

#define	BUFSIZE 	sizeof(struct PDU)
#define DATA 		'D'
#define FINAL 		'F'
#define ERROR 		'E'
#define FILENAME 	'C'
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/*------------------------------------------------------------------------
 * main - Iterative UDP server for FILE DOWNLOAD service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int		sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int		alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
	int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
	struct 	PDU		msg;
	int 	socket_read_size;
	char	socket_buf[BUFSIZE];
	char	file_name[100];
	int		file_size;
	//FILE* 	fp;

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
	s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
		fprintf(stderr, "can't creat socket\n");
	}
	
    /* Bind the socket */
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		fprintf(stderr, "can't bind to %d port\n",port);
	}
	listen(s, 5);	
	alen = sizeof(fsin);

	while (1) {
		
		if (recvfrom(s, socket_buf, sizeof(socket_buf), 0, 
				(struct sockaddr *)&fsin, &alen) < 0) {
			printf("recvfrom error\n");
		}
		
		memcpy(&msg, socket_buf, sizeof(socket_buf));
		
		if (msg.type == FILENAME) {
			memcpy(file_name, msg.data, sizeof(socket_buf) - 1);
			printf("File Name %s\n", file_name);
			FILE* fp = fopen(file_name, "r");
			
			if (fp == NULL) {
				msg.type = ERROR;
				printf("File Not Found\n");
				sendto(s, &msg, BUFSIZE, 0, (struct sockaddr *)&fsin, sizeof(fsin));
			}
			else {
				fseek(fp, 0L, SEEK_END);
	
				file_size = ftell(fp);
			
				char *file_buf = (char *) malloc(file_size);
				fseek(fp, 0L, SEEK_SET);
				fread(file_buf, file_size, 1, fp);
				
				printf("File Size %d\n", file_size);
				sendto(s, &msg, BUFSIZE, 0, (struct sockaddr *)&fsin, sizeof(fsin));
				
				
				int bytes_remaining = file_size;
				int head = 0;
				int tail = file_size - 1;
				
				while (bytes_remaining > 0 && head < tail) {
					if (bytes_remaining < 100) {
						msg.type = FINAL;
						
						memcpy(msg.data, file_buf + head, MIN(bytes_remaining, BUFSIZE - 1));
						
						sendto(s, &msg, BUFSIZE, 0, (struct sockaddr *)&fsin, sizeof(fsin));
						
						head = head + MIN(bytes_remaining, BUFSIZE - 1);
						bytes_remaining = bytes_remaining - MIN(bytes_remaining, BUFSIZE - 1);
						
					}
					else {
						msg.type = DATA;
						memcpy(msg.data, file_buf + head, MIN(bytes_remaining, BUFSIZE - 1));
						
						head = head + MIN(bytes_remaining, BUFSIZE - 1);
						bytes_remaining = bytes_remaining - MIN(bytes_remaining, BUFSIZE - 1);
						
						sendto(s, &msg, BUFSIZE, 0, (struct sockaddr *)&fsin, sizeof(fsin));						
					}					
					memset(&msg, 0, sizeof(struct PDU));
				}
				
				printf("Sent %s\n", file_name);
				printf("File Transfer Complete\n");
			}
		}
	}
}
