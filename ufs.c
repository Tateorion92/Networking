#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#define RECV_BUFF_SIZE 100
#define SEND_BUFF_SIZE 1024
#define PORT_NO 8080
int main(int argc, char *argv[])
{

  /* Make sure a file name was passed in */
    if(argc < 2){
	printf("No directory name given\n");
	return 1;
    }
  long int file_size;
  long int bytes_rem;
  int num_chunks = 1;
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in sa; 
  char recvBuff[RECV_BUFF_SIZE];
  ssize_t recsize;
  socklen_t fromlen;
  char sendBuff[SEND_BUFF_SIZE];
  int numrv;
  char *end_msg = "";
  char *not_found ="FNF";
  char file_path[strlen(argv[1]) + strlen(recvBuff) + 1];

  memset(sendBuff, 0, sizeof(sendBuff));

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY); /*understand this line, conversion big-little endian*/
  sa.sin_port = htons(PORT_NO);
  fromlen = sizeof(sa);

  if (-1 == bind(sock, (struct sockaddr *)&sa, sizeof sa)) {
    perror("error bind failed");
    close(sock);
    return 1;
  }

  for (;;) {
      
      recsize = recvfrom(sock, (void*)recvBuff, sizeof(recvBuff), 0, (struct sockaddr*)&sa, &fromlen);
      if (recsize < 0) {
         fprintf(stderr, "%s\n", strerror(errno));
         return 1;
      }
     
      printf("Requested filename: %.*s\n", (int)recsize, recvBuff);
	 
     /* Open the file that we wish to transfer */
      recvBuff[recsize] = '\0';
      strcpy(file_path, argv[1]);
      strcat(file_path, recvBuff);
      FILE *fp = fopen(file_path,"r");
      if(fp==NULL)
        {
            printf("File open error: %s\n", file_path);
	    sendto(sock,not_found, sizeof(not_found), 0,(struct sockaddr*)&sa, sizeof sa);
            return 1;
        }
      /* Determine the requested file size */
      fseek(fp, 0L, SEEK_END);
      file_size = ftell(fp);
      bytes_rem = file_size;
      fseek(fp, 0L, SEEK_SET);
      printf("Size of %.*s: %ld\n", (int)recsize, recvBuff, file_size);

      while(1)
        {
            /* Read file in chunks of 1024 bytes */
    
            int nread = fread(sendBuff,1,SEND_BUFF_SIZE,fp);
            printf("Bytes read %d \n", nread);
            /* If read was success, send data. */
            if(nread > 0)
            {
		bytes_rem -= nread;
                //printf("Sending chunk %d of 1024 bytes. %ld bytes remaining.\n", num_chunks++, bytes_rem);
                int n = sendto(sock,sendBuff, nread, 0,(struct sockaddr*)&sa, sizeof sa);
 		usleep(15);
 
   
                if(n<0)
                     {
                               perror("Problem sendto\n");
			       fclose(fp);
                               exit(1);
                           }
            }

            if (nread < SEND_BUFF_SIZE)
            {
                if (feof(fp))
		    if(sendto(sock,end_msg, sizeof(end_msg), 0,(struct sockaddr*)&sa, sizeof sa) < 0){
			perror("Problem sending end of file message\n");
		    }
		    num_chunks = 0;
                    printf("End of file\n");
		    fclose(fp);
                if (ferror(fp))
                    printf("Error reading the file at server program\n");
                break;
            }


        }


  }

close(sock);
return 0;

}
