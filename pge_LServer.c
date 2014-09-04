/*
	Name: prog1
	Author: Peiwei Ge
	Date: 04/02/13 16:07
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

#define SERVERPORT 5000 
#define BUFSIZE 150

#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define INFEASIBLE -1
#define OVERFLOW -2

#define LIST_INIT_SIZE 100  //list initial size

typedef int Status;

typedef struct{     // entry structure
   char id_number[10];		
   char first_name[16];    	
   char last_name[21];    	
   char location[36]; 
}ElemType;

typedef struct{    //location array structure
    ElemType elem[LIST_INIT_SIZE]; 
    int length;
    int listsize;
}SqList;

static const int MAXPENDING = 5; // Maximum outstanding connection requests

//founction declaration
int serverrmv(SqList *pL, char frcvcmd[],int clntSock);
void writedata(SqList *pL,char filename[]);
int serverlist(SqList *pL, char frcvcmd[],int clntSock);
int serveradd(SqList *pL, char frcvcmd[],int clntSock);
void warnstr(char *str, int clntSock);
void serverlogin(char frcvcmd[], int clntSock);
int idcmd (char frcvcmd[]);

Status InitList(SqList *pL);
Status ListInsert(SqList *pL,int i,ElemType e);
Status ListDelete(SqList *pL, int i, ElemType *e);


int main(int argc, char *argv[]) {
  
   
  struct sockaddr_in clntAddr; // Client address
   unsigned int clntAddrLen; 
    
  if (argc != 1) // test for correct number of arguments
    DieWithSystemMessage("No parameter(s) needed");

  in_port_t servPort = SERVERPORT; // server port

  // Create socket for incoming connections
  int servSock; // Socket descriptor for server
  if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithSystemMessage("socket() failed");

  // Construct local address structure
  struct sockaddr_in servAddr;                  // Local address
  memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
  servAddr.sin_family = AF_INET;                // IPv4 address family
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  servAddr.sin_port = htons(servPort);          // Local port

  // Bind to the local address
  if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    DieWithSystemMessage("bind() failed");

  // Mark the socket so it will listen for incoming connections
  if (listen(servSock, MAXPENDING) < 0)
    DieWithSystemMessage("listen() failed");
  
  

  unsigned int cmdnum;   //the value of cmdnum stand for the corresponging command
  unsigned int cmdcnt = 0;    //count of the number of commands
  unsigned int cltcnt = 0;   //count of the number of clients
  char buffer[BUFSIZE];    //buffer for the incoming message
  unsigned int bytesrcvd;   //the number of bytes received
  char cmdstr[bytesrcvd];   //stores the command string
  SqList datalist,*pL;   //database list
  pL = &datalist;   //list pointer
  InitList(pL);    //initiate the list
 


while(1){ // Run forever
    
     // Set length of client address structure (in-out parameter)
    clntAddrLen = sizeof(clntAddr);
    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
      DieWithSystemMessage("accept() failed");

    // clntSock is connected to a client!

	//get client address
    char clntName[INET_ADDRSTRLEN]; // String to contain client address
    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr,clntName,sizeof(clntName)) != NULL)
      printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
    else
      puts("Unable to get client address"); 
	
	
    
    while(1){    // Receive command from one user

	 bytesrcvd = recv(clntSock, buffer, BUFSIZE, 0);  
     if (bytesrcvd < 0)
	   DieWithSystemMessage("recv() failed");
	 buffer[bytesrcvd] = '\0';
 
	 strcpy(cmdstr,buffer); //store the command string

	 cmdnum = idcmd(cmdstr);  //identify the id for the corresponding command
	 
	 switch (cmdnum){   //execute command by different functions
	  case 1 :
	  {
	  	//log in
		serverlogin (cmdstr, clntSock);
		cltcnt += 1;   //count user number
		break;
      }
	  case 2 :	
	  {
	  	//add
		serveradd(pL, cmdstr,clntSock);
		cmdcnt += 1;  //count command number
		break;

	  }
	  case 3 :	//remove
	  {
		serverrmv(pL, cmdstr,clntSock);
		cmdcnt += 1;  //count command number
		break;
      }
  	  case 4 :	//list
	  {
	  	serverlist(pL, cmdstr,clntSock);
		cmdcnt += 1;  //count command number
		break;
	  }	
 
      case 5 : 
		  {     //quit 
		   cmdcnt += 1;
	       char *quitcmd;
		   asprintf(&quitcmd, "User quit, and the number of commands the user issued is: %d", cmdcnt); 
		   warnstr(quitcmd,clntSock);  //send back a count of the number of command a user issued to the server
		   cmdcnt = 0;  //reset the count of the number of commmands
		   close(clntSock);  //close connection
		   break;
          }
      case 6 :  //quit EOF
        {
        char *quiteof;
		asprintf(&quiteof, "The number of clients processed is: %d. Server is closed", cltcnt);  
		warnstr(quiteof,clntSock); //send back a count of the number of users processed

		writedata(pL, "LDatabase.txt");//write out the complete database to the file LDatabase.txt

		DieWithSystemMessage("Quit EOF. Tasks finished. Server is closed.");
        close(clntSock);
		close(servSock);  //close the connection, the server then terminates.
        exit(0);
		break;
        }

	  default:			//command wrong, discard and waiting for next
		break;
	 }//end switch
	 
	 if(cmdnum==5)//one user quit
	   break;
	 
    }
	  
  }
  
  return 0;
}

//write data to file
void writedata(SqList *pL,char filename[]){
	int i=0;
	FILE *wfPtr; //writing file pointer
        wfPtr = fopen (filename, "w");
	if (wfPtr == NULL) {
            DieWithSystemMessage("File could not be opened\n");
	} 
	
		if ((*pL).length == 0) {
		printf("no items in the database.\n" );
	     }//end if
      	else
		while (i< (pL->length)){
		fprintf(wfPtr, "%s %s %s %s\n", (pL->elem)[i].id_number,\
                        pL->elem[i].first_name,pL->elem[i].last_name, \
                        pL->elem[i].location);
		i++;
		}//end while
	
	fclose(wfPtr);
	
}


//list founction
int serverlist(SqList *pL, char frcvcmd[],int clntSock)
{
        char strsep[BUFSIZE];                   //it will be separated
		strcpy (strsep, frcvcmd);			//string got by tcp/ip need copy to it
		char *token;
		char servcmd[5];  				 //server command
		token = strtok(strsep, " ");
		strncpy(servcmd, token,5);   // get server command
		
		char temprecv[BUFSIZE];    //temporary buffer
		unsigned int numbytesrcvd;
		
        
        char backstr[BUFSIZE]; //store the string which will be send back to client
		char start[10]; // start character string
		char end[10];	//end character string
		int i;
		
		
		
		if(pL->length==0){    //determine if the database is empty
			
			warnstr("There are no entries in the database.",clntSock);
			warnstr("\n",clntSock);
			return OK;
		}
		
			
        token = strtok(NULL, " ");	//get the first argument
		if(token==NULL)    //determine if there are arguments
		{
		for(i=0;i<pL->length;i++) //no arguments are specified, returns the complete list of database entries
		{
			strcpy(backstr,pL->elem[i].id_number);
			backstr[9]=' ';
			backstr[10]='\0';
			strcat(backstr,pL->elem[i].first_name);
			backstr[strlen(pL->elem[i].first_name)+10]=' ';
			backstr[strlen(pL->elem[i].first_name)+11]='\0';
			strcat(backstr,pL->elem[i].last_name);
			backstr[strlen(pL->elem[i].first_name)+11+strlen(pL->elem[i].last_name)]=' ';
			backstr[strlen(pL->elem[i].first_name)+12+strlen(pL->elem[i].last_name)]='\0';
			strcat(backstr,pL->elem[i].location);
			
			warnstr(backstr,clntSock);
			
		    numbytesrcvd = recv(clntSock, temprecv, BUFSIZE, 0); //receive the acknowledgment message after one item was sent
            if (numbytesrcvd < 0)
	         DieWithSystemMessage("recv() failed");
			
     	}
     	warnstr("\n",clntSock); //tell the client the list has been sent over
     	return OK;
        }
		else strncpy(start, token,1);	//store the start character
	
				
                token = strtok(NULL, " "); //determine if there is finish character
                if(token==NULL) //no finish character, will be set the same as start character
				{
                	strncpy(end,start,1);
                }
				else
				{
					strncpy(end, token,1); //store finish character
					if(strncasecmp(start,end,1)>0)//if finish character is ahead of start character, return wrong parameters
		            {
		 			 warnstr("Input parameters are wrong",clntSock);
		 			 numbytesrcvd = recv(clntSock, temprecv, BUFSIZE, 0);
		 			 
                     if (numbytesrcvd < 0)
	                   DieWithSystemMessage("recv() failed");
	                   
		 			 warnstr("\n",clntSock);
			    	 return OK;
    				}    
				} 
		
		
		
		
		int j=0,k=0;
		for(i=0;i < pL->length;i++)    //get the location of the first item matching the start character
		if(strncasecmp(pL->elem[i].last_name,start,1)>=0)
        break;
        
        if(pL->length==i){ //no items matching the list request
			warnstr("There are no entries satisfying the list request.",clntSock);
			numbytesrcvd = recv(clntSock, temprecv, BUFSIZE, 0);
            if (numbytesrcvd < 0)
	         DieWithSystemMessage("recv() failed");
	         
			warnstr("\n",clntSock);
			return OK;
		}
		j=i;
		
		for(i=(pL->length)-1;i >= 0;i--)  //get the location of the last item matching the start character
		if(strncasecmp(pL->elem[i].last_name,end,1)<=0)
		break;
		k=i;
		
		if(j>k)  //if the location of the last item is ahead that of the first item, return no items matching the list request
		{
		    warnstr("There are no entries satisfying the list request.",clntSock);
		    
		    numbytesrcvd = recv(clntSock, temprecv, BUFSIZE, 0);
            if (numbytesrcvd < 0)
	         DieWithSystemMessage("recv() failed");
	         
			warnstr("\n",clntSock);
			return OK;	
		}
		
		
		for(i=j;i<=k;i++) //return the list of entries matching the list request
		{	
			strcpy(backstr,pL->elem[i].id_number);
			backstr[9]=' ';
			backstr[10]='\0';
			strcat(backstr,pL->elem[i].first_name);
			backstr[strlen(pL->elem[i].first_name)+10]=' ';
			backstr[strlen(pL->elem[i].first_name)+11]='\0';
			strcat(backstr,pL->elem[i].last_name);
			backstr[strlen(pL->elem[i].first_name)+11+strlen(pL->elem[i].last_name)]=' ';
			backstr[strlen(pL->elem[i].first_name)+12+strlen(pL->elem[i].last_name)]='\0';
			strcat(backstr,pL->elem[i].location);
			
			warnstr(backstr,clntSock);
			
			numbytesrcvd = recv(clntSock, temprecv, BUFSIZE, 0); //receive the acknowledgment message after one item was sent
            if (numbytesrcvd < 0)
	         DieWithSystemMessage("recv() failed");
			
		}
		warnstr("\n",clntSock);	//tell the client the list has been sent over	
	
	return OK;
	
	
}//list end

//remove founction
int serverrmv(SqList *pL, char frcvcmd[],int clntSock)
{
	    char strsep[BUFSIZE];                   //it will be separated
		strcpy (strsep, frcvcmd);			//string got by tcp/ip need copy to it
		char *token;
		char servcmd[7];  				 //server command
		token = strtok(strsep, " ");
		strncpy(servcmd, token,7); 
        
		char tempnum[10];
						
        token = strtok(NULL, " ");		//store id_number
        strncpy(tempnum,token,10);	
                
       	if(pL->length==0){   //if the database is empty, return: Item not found
			warnstr("Item not found",clntSock);
			return OK;
		 }
                
                int i=0;
               
                //looping to find correct location in the list
                while(strncmp(pL->elem[i].id_number,tempnum,9) != 0)
				{

                        i++;
                        
                        if(i == (pL->length))  
						{
						
                        warnstr("Item not found",clntSock);
                        return ERROR;
                        }
                }
                
                ElemType *t;  //get the pointer of the item which will be deleted
                t = &(pL->elem[i]);
                
                if(ListDelete(pL,(i+1),t)!=1) //determine if delete action is successful
                        {
                        warnstr("the delete operation is not successful!", clntSock);
                        return ERROR;}

                
                char *backstr;	//send back the item's information which has been deleted
                asprintf(&backstr, "An item successfully deleted: %s %s", t->first_name,t->last_name);
                warnstr(backstr,clntSock);

	return OK;
}//remove end


//add founction
int serveradd(SqList *pL, char frcvcmd[],int clntSock)
{

	char strsep[BUFSIZE];                   //it will be separated
	strcpy (strsep, frcvcmd);			//string got by tcp/ip need copy to it
	char *token;
	char servcmd[4];  				 //server command
	token = strtok(strsep, " ");
	
	strncpy(servcmd, token,4); //discard the first field

        ElemType newentry; //store the item information which will be added
						
        token = strtok(NULL, " ");//get id_number
        if(token==NULL)      //if id_number is not given, return missing parameters
        {
           warnstr("Missing parameters",clntSock);
           return ERROR;
        }
        strncpy(newentry.id_number,token,10); //store id_number
        
        if(strlen(newentry.id_number)!=9){ //determine if the length of id_number is right
                warnstr("id_number should be a 9-digit identification number",clntSock);
                return ERROR;
        }
        token = strtok(NULL, " ");  //get first_name
        if(token==NULL)   //if first name is not given, return missing parameters
        {
           warnstr("Missing parameters",clntSock);
           return ERROR;
        }
        strncpy(newentry.first_name,token,16); //store first_name
        if(strlen(newentry.first_name)>15){   //determine if the length of first_name is right
                warnstr("first_name should be a non-blank ASCII string with maximum length of 15 characters",clntSock);
                return ERROR;
        }
        token = strtok(NULL, " ");   //get last_name
        if(token==NULL)   //if last name is not given, return missing parameters
        {
           warnstr("Missing parameters",clntSock);
           return ERROR;
        }
        strncpy(newentry.last_name, token,21);  //store first_name
        if(strlen(newentry.last_name)>20){   //determine if the length of last_name is right
                warnstr("last_name should be a non-blank ASCII string with maximum length of 20 characters",clntSock);
                return ERROR;
        }
        token = strtok(NULL, " ");  //get location
        if(token==NULL)  //if location is not given, return missing parameters
        {
           warnstr("Missing parameters",clntSock);
           return ERROR;
        }
        strncpy(newentry.location, token,36); //store location
        if(strlen(newentry.location)>35){  //determine if the length of location is right
                warnstr("Location should be a non-blank character string (35 character max) indicating the person's current location.",clntSock);
                return ERROR;
        }

        
        int i = 0;
        
        
        //looping to find correct location in the list
        while ((strncasecmp(pL->elem[i].last_name,"0",1)!=0)\
                &&(strncasecmp(pL->elem[i].last_name,newentry.last_name,20) <= 0))
        {
                if(strncasecmp(pL->elem[i].id_number,newentry.id_number,9)==0)//determine if there is identical items
                {
                warnstr("Identical id_number!",clntSock);
                return ERROR;
                } 
				 
                i++;
                
                if(i == pL-> listsize)  //determine if the database is full
				{
                warnstr("The database is full, can't add any more!",clntSock);
                return ERROR;
                }
        }
        
        	
        if(ListInsert(pL,(i+1),newentry)==0)  //determine if the add action is successful
                {
                warnstr("The add operation is not successful!", clntSock);
                return ERROR;}

        
        
        char *backstr;	 //send back the item's information which has been added
        asprintf(&backstr, "An item successfully added: %s %s %s %s", \
                pL->elem[i].id_number,pL->elem[i].first_name,pL->elem[i].last_name,pL->elem[i].location);
        warnstr(backstr,clntSock);
				
				
	
	return OK;
}//add end


//founction used to send message back to client
void warnstr(char *str, int clntSock){
    
    char string[BUFSIZE];
    strcpy(string,str);
	unsigned int stringlen;
    stringlen = strlen(string);
	ssize_t numBytesSent = send(clntSock, string, stringlen, 0);
	if (numBytesSent < 0)
	DieWithSystemMessage("send() failed");
	
}


//login founction
void serverlogin(char frcvcmd[], int clntSock)
{
    char strsep[BUFSIZE];                   //it will be separated
    strcpy (strsep, frcvcmd);			//string got by tcp/ip need copy to it
	char *token;
	char *servcmd;  				 //server command
	servcmd = strtok(strsep," ");
	token = strtok(NULL," ");   //get user name
	char *welcome;
	asprintf(&welcome, "Hello %s",token); 
    warnstr(welcome,clntSock);  //return the response message
    

}//login end


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
	int cmdnum = 0;                 //the value of cmdnum stands for the corresponding command 

  if (strncasecmp(strqeof, strsep,8) == 0)
	cmdnum = 6 ;                               //quit EOF
  else if (strncasecmp(strquit, strsep,4) == 0)
	cmdnum = 5 ;                               //quit
    else{
	char *token;
	char *servcmd;  				 //server command
	token = strtok(strsep, " ");
	asprintf(&servcmd, "%s", token);//separate the fist word as command
	if (strncasecmp(strlogin, servcmd,5) == 0)  
	cmdnum = 1 ;   //login
	else if (strncasecmp(stradd, servcmd,3) == 0)
	cmdnum = 2 ;   //add
	else if (strncasecmp(strremove, servcmd,6) == 0)
	cmdnum = 3 ;   //remove
	else if (strncasecmp(strlist, servcmd,4) == 0)
	cmdnum = 4 ;   //list
	else 
	return 10;		//wrong command
        }//end else
        
return cmdnum;

}//identify command finished


//the founction used to add one item in the data list
Status ListInsert(SqList *pL,int i,ElemType e)
{
    //1<=i<=ListLength_Sq(L)+1
    ElemType *p;
    ElemType *q;
    
    if(i<1 || i>( pL->length + 1)) 
        return ERROR;
    
    q=&((*pL).elem[i-1]);
    for(p=&((*pL).elem[(*pL).length-1]);p>=q;--p)
        *(p+1)= *p;
    *q=e;
    ++(*pL).length;  //the length of data list plus one
    return OK;
}//ListInsert end


//the founction used to delete one item in the data list
Status ListDelete(SqList *pL, int i, ElemType *e) 
{
    ElemType *p, *q;
    if (i<1 || i> (pL->length)) 
    return ERROR; 
    p = &((*pL).elem[i-1]); 
    *e = *p; 
    q = (*pL).elem+(*pL).length-1; 
    for (++p; p<=q; ++p) *(p-1) = *p; 
        --(*pL).length;    //the length of data list minus one
    return OK;
} // ListDelete end

//the founction used to initiate the data list
Status InitList(SqList *pL)//init list
{
    
    (*pL).length=0;  //the initial length of the data list is 0
    (*pL).listsize = LIST_INIT_SIZE;  //capacity of the database
    
    int i;
    for(i=0;i < LIST_INIT_SIZE ;i++)  //initiate the items in the data list
    {
    	strcpy(pL->elem[i].id_number,"0");
    	strcpy(pL->elem[i].first_name,"0");
    	strcpy(pL->elem[i].last_name,"0");
    	strcpy(pL->elem[i].location,"0");
    }
    
    return OK;
}//InitList end

