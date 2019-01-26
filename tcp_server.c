/*Required Headers*/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SEND_BUFF_SIZE 100
#define RECV_BUFF_SIZE 100
 
int main(int argc, char *argv[])
{
 
    char str[RECV_BUFF_SIZE];
    char filename[RECV_BUFF_SIZE];
    char sendline[SEND_BUFF_SIZE];
    char *SERVER_IP, *SERVER_PORT;
    int listen_fd, comm_fd;
    FILE *zipfp, *uzipfp, *uznamefp;

    SERVER_IP = argv[1];
    SERVER_PORT = argv[2];
    /*if((SERVER_PORT = atoi(argv[2])) == 0){
    	printf("Invalid port number\n");
	exit(1);
    }*/
    printf("ip: %s\tport: %d\n", SERVER_IP, atoi(SERVER_PORT));

    struct sockaddr_in servaddr;
 
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    bzero( &servaddr, sizeof(servaddr));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);	//was htons(INADDR_ANY)
    servaddr.sin_port = htons(atoi(SERVER_PORT));
 
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
 
    listen(listen_fd, 10);
 
    comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
 
    //read the name of the zipped file
    while(1){
    	if((read(comm_fd,str,RECV_BUFF_SIZE)) > 0){
		printf("Zipped filename: %s\n", str);
		strcpy(filename, str);
		break;
	}
	else{
		printf("Error reading filename\n");
		exit(1);
	}
    }

    zipfp = fopen(filename, "w");
    printf("Zipped filename: %s\n", filename);
    //read in and build the zipped file
    int n, i = 0;
    fflush(stdout);
    while(1)
    {
        bzero( str, RECV_BUFF_SIZE);
        if((n = read(comm_fd,str,RECV_BUFF_SIZE)) == 0){
		printf("End of zipped file\n");
		break;
	}
	/*else if(n < 0){
		printf("Error reading zipped file\n");
		fclose(zipfp);
		exit(1);
	}*/
	//printf("Writing, round %d\n", i++);
	fwrite(str, 1, n, zipfp);
    }
    fclose(zipfp);
    //use zipinfo command to get the name of the file after it is unzipped (target)
    char command[100];
    strcpy(command, "zipinfo -1 ");
    strcat(command, filename);
    strcat(command, " | cat > outfilename.txt");
    printf("Command \"%s\"\n", command);
    system(command);

    //prepare unzip command
    char com[100];
    strcpy(com, "unzip -o ");
    strcat(com, filename);
    printf("Command \"%s\"\n", com);

    //extract the target filname from "outfilename.txt"
    uznamefp = fopen("outfilename.txt", "r");
    strcpy(filename, "");
    bzero( str, RECV_BUFF_SIZE);
    printf("filename[]: %s\tstr[]:%s\n", filename, str);
    while((fread(str, 1, 1, uznamefp)) == 1){
	printf("str[]: %s\n", str);
	if(*str != '\n'){
    		strcat(filename, str);
    	}
    }
    fclose(uznamefp);
    system(com);
    if((uzipfp = fopen(filename, "r")) == NULL){
	printf("Did not open the unzipped file; uzipfp == NULL at ln 108. Exiting.\n");
	exit(1);
    }
    printf("filename[]: %s\n", filename);

    //send filename
    printf("Sending filename: %s\n", filename);
    if((write(comm_fd, filename, RECV_BUFF_SIZE)) < 0){
	printf("Error sending FILENAME\n");
    }

    //write back the unzipped file
    //printf("Just before uzipfp is set to point to filename\n");
    /*if(uzipfp==NULL){
	printf("uzipfp == NULL\n");
	exit(1);
    }*/
   // printf("Just after uzipfp is set to point to filename\n");
    int bytes_read = 0;
    while(1){
	
	bzero(sendline, RECV_BUFF_SIZE);
	
	if((bytes_read = fread(sendline, 1, SEND_BUFF_SIZE, uzipfp)) > 0){
		//printf("Sending %d bytes\n%s", bytes_read, sendline);
		if((write(comm_fd, sendline, bytes_read))==0){
			printf("END\n");
			break;
		}

		if(bytes_read < SEND_BUFF_SIZE && feof(uzipfp)){
			printf("End of unzipped file\n");
			fclose(uzipfp);
			break;
		}
	} 
	
    }
    close(comm_fd);
    
}
