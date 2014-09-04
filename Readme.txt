Author: Peiwei Ge		
Date: 02/04/2013						
Project ID: prog1			
Programming Language: C
OS/Hardware dependencies: Linux	
				
Problem Description: A Location Client and Server is designed, which can communicate with each other using TCP and execute several operations issued by the client to the server.
		

How to build the program: Execute the makefile in the tar file.
			

Test Procedures: Login one of WPI Linux machine, and type: %./pge_LServer
This will start the Location Server.
Login another WPI Linux machine, and type: %./pge_LClient LServerMachine (LClient.txt)
where 
      LServerMachine indicates the logical name for the server machine (e.g., CCCWORK4.wpi.edu).
and 
      LClient.txt indicates that data is to be read from this text file. If this field is not specified, the Location Client reads command line input from standard input.
			

Performance Evaluation: The program has achieves achieved every founction the assignment requests. Sometimes it may not work very well because of the unstability network issues. Please try more than once.


References: TCP/IP Sockets in C: Practical Guide for Programmers, Second Edition	
