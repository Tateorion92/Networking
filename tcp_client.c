#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define SEND_BUFF_SIZE 100
#define RECV_BUFF_SIZE 100

int main(int argc,char *argv[])
{
    if(argc < 4){
	printf("Missing or incorrect argument format. Please enter ./client <target IP> <target port> <file name>\n");
	exit(1);
    }
    int sockfd,n;
    char sendline[SEND_BUFF_SIZE];
    char recvline[RECV_BUFF_SIZE];
    char filename[RECV_BUFF_SIZE];
    char *SERVER_IP, *SERVER_PORT;
    FILE *fp, *outfp;

    SERVER_IP = argv[1];
    SERVER_PORT = argv[2];
    /*if((SERVER_PORT = atoi(argv[2])) == 0){
    	printf("Invalid port number\n");
	exit(1);
    }*/
    /*if((argv[3] = argv[3]) == NULL){
	printf("Missing file name. Please enter ./client <target IP> <target port> <file name>\n");
	exit(1);
    }*/
    if((fp = fopen(argv[3], "rb")) == NULL){
	printf("File \"%s\" not found\n", argv[3]);
	exit(1);
    }

    //Get the size of the file
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);
    int bytes_remaining = file_size;
    fseek(fp, 0L, SEEK_SET);

    printf("ip:%s\tport:%d\tfile:%s\tsize:%d\n", SERVER_IP, atoi(SERVER_PORT), argv[3], file_size);
    struct sockaddr_in servaddr;
 
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof servaddr);
 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(SERVER_PORT));
 
    inet_pton(AF_INET,SERVER_IP,&(servaddr.sin_addr));
 
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
 
    //send .zip filename
    if((write(sockfd, argv[3], RECV_BUFF_SIZE)) < 0){
	printf("Error sending argv[3]\n");
    }
    printf("Sending zipped filename: \"%s\"\n", argv[3]);

    int bytes_read = 0, nr = 0;
    clock_t begin = clock();
    fflush(stdout);
    while(1)
    {
        bzero( sendline, SEND_BUFF_SIZE);

        if((bytes_read = fread(sendline, 1, SEND_BUFF_SIZE, fp)) > 0 ){
		write(sockfd,sendline, bytes_read);
		if(bytes_read < SEND_BUFF_SIZE && feof(fp)){
			printf("End of zipped file.\n");
			shutdown(sockfd, SHUT_WR);
			fclose(fp);
			break;
		}
	}
    }

    //bzero(recvline, RECV_BUFF_SIZE);
    if((read(sockfd,filename,RECV_BUFF_SIZE)) < 0){
	printf("ERROR\n");
    }

    //read unzipped file from server
    if((outfp = fopen(filename, "w")) == NULL){
	printf("Did not create an output file; outfp == NULL at ln 89. Exiting.\n");
	exit(1);
    }
    printf("Ready to receive unzipped file %s\n", filename);
    while(1){
	
        bzero(recvline, RECV_BUFF_SIZE);

	if((nr = read(sockfd,recvline,RECV_BUFF_SIZE)) <= 0){
		printf("End of unzipped file\n");
		clock_t end = clock();
		printf("Time of transfer: %0.2f\n", (double)(end-begin)*1000/CLOCKS_PER_SEC);
		break;
	}
	//printf("Received %d bytes\n%s", nr, recvline);	
	fwrite(recvline, 1, nr, outfp);
    }
    fclose(outfp);
}
