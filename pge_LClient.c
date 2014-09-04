/*
	Name: prog1
	Author: Peiwei Ge
	Date: 04/02/13 13:17
	Description: A Location Client and Server
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Practical.h"
#include "Practical.h"
#include <netdb.h>
#include <arpa/inet.h>
#include "Practical.h"

#define SERVERPORT 5000   //server port
#define BUFSIZE 100        //buffer size



int idcmd (char frcvcmd[]);   //declaration


int main(int argc, char *argv[]) {

  if (argc < 2 || argc > 3) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)", "<Server Address> <file name>");
        
  char rcvdbuffer[BUFSIZE]; // I/O buffer store received strings
  
  int flag = 0;   //flag for command input manner: from standard input or from an input file
  
  size_t cmdstrlen;  //input command string length
  ssize_t numBytes;   //bytes the send() founction sends
  
  unsigned int cmdnum; //count of the number of commands
  unsigned int bytesrcvd; //bytes the recv() founction receives
  char cmdstr[BUFSIZE];  //buffer for the input command
  FILE *fp;    //file pointer of LClient.txt
  FILE *wfPtr; //writing file pointer
  
  if ((wfPtr = fopen ("LClient.log", "w")) == NULL) {  
	    printf("File could not be opened\n" );
	} 
	
  if (argc == 3) //decide to read commands from standard input or from an input file
  {
  	flag =1;  //read commands from an input file
  	char *fname = argv[2];
	fp = fopen (fname, "r"); /*open the file to read*/
    if (fp == NULL) {
        DieWithSystemMessage("File could not be opened\n");
	} 
  }
  else //read commands from standard input
  {
	printf("File could not be opened, please input from command line\n" );
  }
  
  //Construct the server address structure
	struct sockaddr_in servAddr;            // Server address
	memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
	servAddr.sin_family = AF_INET;          // IPv4 address family

	//get sever IP address
	struct hostent *hptr;  //struct for return of gethostbyname
        hptr = gethostbyname(argv[1]);
	if(hptr == NULL) {    //get the host info
	herror("gethostbyname");
	return 2;
	}
	bcopy( hptr->h_addr, &servAddr.sin_addr, hptr->h_length);
	
	in_port_t servPort = SERVERPORT;       //get server port
        servAddr.sin_port = htons(servPort);   
 
 



   //read commands
while(1)  
{    
 
  if (flag == 0) {  //read command from standard input
	 	  	
	fp = stdin;
	   fgets(cmdstr, BUFSIZE, fp);
	   
        while(idcmd (cmdstr)!=1){
		   printf("Please log in!\n");
		   fgets(cmdstr, BUFSIZE, fp);
		   
	    }    
       } 
	else {//file can be opened, read command from file
	 	  		
		 fgets(cmdstr, BUFSIZE, fp); //get a line from the file
		 
		 while(idcmd (cmdstr)!=1){   //first command must be login
	        fprintf(stdout,"Please log in!\n");
	        fprintf(wfPtr,"Please log in!\n");
	        fgets(cmdstr, BUFSIZE, fp); 
	        
	     }		   
	}
	

   cmdnum = idcmd (cmdstr);  //cmdnum stands for the corresponding command
   
  
    // Create a reliable, stream socket using TCP
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0)
    DieWithSystemMessage("socket() failed");
    
    
  // Establish the connection to the echo server
  if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    DieWithSystemMessage("connect() failed");
  
    
    
  // Send login command to the server
    cmdstrlen = strlen(cmdstr); // Determine input length
    numBytes = send(sock, cmdstr, cmdstrlen, 0);
    if (numBytes < 0)
        DieWithSystemMessage("send() failed");
    else if (numBytes != cmdstrlen)
        DieWithUserMessage("send()", "sent unexpected number of bytes");
            
        
  //receive respond to the login command
    bytesrcvd = recv(sock, rcvdbuffer, (BUFSIZE-1), 0);//Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender
    if (bytesrcvd < 0){
            fputs("recv() failed, skip to the next command\n", stdout);
            fprintf(wfPtr, "recv() failed, skip to the next command\n"); //store the respond to file 
    } 
    else if (bytesrcvd == 0){
    	DieWithUserMessage("recv()", "connection closed prematurely");	
    }
        
    else {   //receive successfully, print the response to the login command
     rcvdbuffer[bytesrcvd] = '\0';   // Terminate the string
     fputs(rcvdbuffer, stdout); // Print buffer
     fputc('\n', stdout); // Print a blank line
	 fprintf(wfPtr, "%s\n", rcvdbuffer);   //store the respond to file 
    }
    
  

      //commands issued by a user
    while(1) {
             
            if (flag == 0) {   //read command from standard input

            fp=stdin;
            fgets(cmdstr, BUFSIZE, fp);
            

            }
            else   //file can be opened, read command from file	
            {
            fgets(cmdstr, BUFSIZE, fp); //get a line from the file	
              
            }
            cmdnum = idcmd (cmdstr);
            
  	        
            switch (cmdnum){     //execute corresponging command 
                case 1 :fputs("A user has logged in!\n", stdout);   
                        fprintf(wfPtr, "A user has logged in!\n"); 
                        break;
                case 2 :   //add action
                case 3 :   //remove action
                {       // Send the command to the server
                        cmdstrlen = strlen(cmdstr); // Determine input length
                        numBytes = send(sock, cmdstr, cmdstrlen, 0);
                        if (numBytes < 0)
                            DieWithSystemMessage("send() failed");
                        else if (numBytes != cmdstrlen)
                                DieWithUserMessage("send()", "sent unexpected number of bytes");
						
						//receive the respond
                        bytesrcvd = recv(sock, rcvdbuffer, BUFSIZE - 1, 0);
                        if (bytesrcvd < 0){
                                fputs("recv() failed, skip to the next command\n", stdout);
                                fprintf(wfPtr, "recv() failed, skip to the next command\n"); 
                        } 
                        else if (bytesrcvd == 0)
                                DieWithUserMessage("recv()", "connection closed prematurely");
                        else {
                         rcvdbuffer[bytesrcvd] = '\0';   // Terminate the string
                         fputs(rcvdbuffer, stdout); // Print buffer
                         fputc('\n', stdout); // Print a blank line
                         fprintf(wfPtr, "%s\n", rcvdbuffer);     //store the respond to file 
                        }
                        break;
                }
                case 4 :	//list action	
                {		// Send the command to the server
                        cmdstrlen = strlen(cmdstr); // Determine input length
                        numBytes = send(sock, cmdstr, cmdstrlen, 0);
                        if (numBytes < 0)
                            DieWithSystemMessage("send() failed");
                        else if (numBytes != cmdstrlen)
                            DieWithUserMessage("send()", "sent unexpected number of bytes");
                        
						//list the results
                        do
						{
                        bytesrcvd = recv(sock, rcvdbuffer, BUFSIZE - 1, 0);
                        if (bytesrcvd < 0){
                            fputs("recv() failed, skip to the next command\n", stdout);
                            fprintf(wfPtr, "recv() failed, skip to the next command\n"); //store the respond to file 
                        } 
                        else if (bytesrcvd == 0)
                                DieWithUserMessage("recv()", "connection closed prematurely");
                        else 
						{
                         rcvdbuffer[bytesrcvd] = '\0';   // Terminate the string
                        
						 if(strncmp(rcvdbuffer,"\n",1)!=0) //if receive "\n", not print out,
                         {
                         
						 fputs(rcvdbuffer, stdout); // Print buffer
                         fputc('\n', stdout); // Print a blank line
                         fprintf(wfPtr, "%s\n", rcvdbuffer);  
                         
						 numBytes = send(sock, cmdstr, cmdstrlen, 0);//when an entry is received, send one acknowledgment message
                         if (numBytes < 0)
                            DieWithSystemMessage("send() failed");   
                         }
                          
                        }
                        }while(strncmp(rcvdbuffer,"\n",1)!=0);//if receive "\n", list results have been all received
                        
                        break;
                }
				case 5: //quit action
				{		//send the command
					    cmdstrlen = strlen(cmdstr); // determine input length
                        numBytes = send(sock, cmdstr, cmdstrlen, 0);
                        if (numBytes < 0)
                            DieWithSystemMessage("send() failed");
                        else if (numBytes != cmdstrlen)
                                DieWithUserMessage("send()", "sent unexpected number of bytes");
						
						//receive the respond
                        bytesrcvd = recv(sock, rcvdbuffer, BUFSIZE - 1, 0);
                        if (bytesrcvd < 0){
                                fputs("recv() failed, connection closed\n", stdout);
                                fprintf(wfPtr, "recv() failed, connection closed\n"); 
                        } 
                        else if (bytesrcvd == 0)
                            DieWithUserMessage("recv()", "connection closed prematurely");
                        else {
                         rcvdbuffer[bytesrcvd] = '\0';    // Terminate the string
                         fputs(rcvdbuffer, stdout);      // Print buffer
                         fputc('\n', stdout); // Print a blank line
                         fprintf(wfPtr, "%s\n", rcvdbuffer);   //store the respond to file 
                        }
       				 
       				 close(sock);    
					
					break;
				}
                case 6 :
                {//quit EOF	action	
                        //send the command
						cmdstrlen = strlen(cmdstr); // Determine input length
                        numBytes = send(sock, cmdstr, cmdstrlen, 0);
                        if (numBytes < 0)
                            DieWithSystemMessage("send() failed");
                        else if (numBytes != cmdstrlen)
                                DieWithUserMessage("send()", "sent unexpected number of bytes");

						//receive the respond
                        bytesrcvd = recv(sock, rcvdbuffer, BUFSIZE - 1, 0);
                        if (bytesrcvd < 0){
                                fputs("recv() failed, connection closed\n", stdout);
                                fprintf(wfPtr, "recv() failed, connection closed\n"); //store the respond to file 
                        } 
                        else if (bytesrcvd == 0)
                               DieWithUserMessage("recv()", "connection closed prematurely");
                        else {
                         rcvdbuffer[bytesrcvd] = '\0';    // Terminate the string
                         fputs(rcvdbuffer, stdout);      // Print buffer
                         fputc('\n', stdout); // Print a blank line
                         fprintf(wfPtr, "%s\nEOF", rcvdbuffer);    //store the respond to file 
                        }
                        fclose(wfPtr); //closes the log file and terminates
                        close(sock);
                        exit(0);
                        break;

                }
                default:			//command wrong, go on to receive
                        fputs("Wrong command or missing parameter!\n", stdout);
                        break;

                }//end switch
                
                
        if(cmdnum==5)//one user quit
        break;
    } //if quit, another user will follows either as a continuation of the input file or from standard input
  		
  		
       
 }

  
  
  exit(0);
  
}


//identify command and return the number corresponding to the command
int idcmd (char frcvcmd[])
{
    char strsep[BUFSIZE];                   //it will be separated
    strcpy (strsep, frcvcmd);			//string got by tcp/ip need copy to it
	char strlogin[]="login";		//strings to be compared with command
	char stradd[]="add";			
	char strremove[]="remove";	
	char strlist[]="list";			
	char strquit[]="quit";			
	char strqeof[]="quit EOF";		
	int cmdnum = 0;                 //value standing for command 

  if (strncasecmp(strqeof, strsep,8) == 0)
	 cmdnum = 6 ;                               //for quit EOF
  else if (strncasecmp(strquit, strsep,4) == 0)
	cmdnum = 5 ;                               //for quit
    else{
	char *token;
	char *servcmd;  				 //server command
	token = strtok(strsep, " ");
	asprintf(&servcmd, "%s", token); //separate the fist word as command
	if (strcasecmp(strlogin, servcmd) == 0)
	cmdnum = 1 ;
	else if (strcasecmp(stradd, servcmd) == 0)
	cmdnum = 2 ;
	else if (strcasecmp(strremove, servcmd) == 0)
	cmdnum = 3 ;
	else if (strncasecmp(strlist, servcmd,4) == 0)
	cmdnum = 4;
	else {		
        return 10;//---command is wrong
        } 
    }//end else

   return cmdnum;

}//identify command finished 
