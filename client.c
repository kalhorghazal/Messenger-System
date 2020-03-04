#include "client.h"

char* itoa(int val)
{
	int base = 10;
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}

void print(char* buf) 
{
    write(ONE, buf, strlen(buf));
}

struct sockaddr_in get_server_address(int port)
{
    struct sockaddr_in servaddr;
    memset(&servaddr, ZERO, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS_IP); //INADDR_ANY
    return servaddr;
}

struct sockaddr_in get_broadcast_address(int port)
{
    struct sockaddr_in servaddr;
    memset(&servaddr, ZERO, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //INADDR_ANY
    return servaddr;
}

void bind_socket(int sockfd, struct sockaddr_in servaddr)
{
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < ZERO )
    {
        perror("bind failed!\n");
        exit(EXIT_FAILURE);
    }
}

int get_udp_socket()
{
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, ZERO)) < ZERO ) 
    {
        perror("socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int get_tcp_socket()
{
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, ZERO)) < ZERO ) 
    {
        perror("socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void set_broadcast_options(int sockfd)
{
    int opt = ONE;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, 
    	sizeof(opt)) < ZERO)
    {
        perror("set broadcast option failed!\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, 
    	sizeof(opt)) < ZERO)
    {
        perror("set broadcast option failed!\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, 
    	sizeof(opt)) < ZERO)
    {
        perror("set broadcast option failed!\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void set_reusable_option(int sockfd)
{
    int opt = TRUE;
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < ZERO )
    {
        perror("set reusable option failed!\n");
        exit(EXIT_FAILURE);
    }
}

void handle_heartbeat_signal()
{
	char message[MAXLINE]; 
	strcat(message, itoa(group_port)); 
	strcat(message, DELIM);
    if (sendto(globalHeartbeatSocketFD, message, strlen(message),
           MSG_CONFIRM, (const struct sockaddr *) &globalHeartbeatAddress,
           sizeof(globalHeartbeatAddress)) != strlen(message))
    {

        perror("sending heartbeat failed"); 
        exit(EXIT_FAILURE);
    }

    alarm(HEARTBEAT_RATE);
}

void start_heartbeat(int heartbeat_port)
{
    int sockfd = get_udp_socket();

    struct sockaddr_in servaddr = get_broadcast_address(heartbeat_port);

    set_broadcast_options(sockfd);

    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < ZERO )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    globalHeartbeatSocketFD = sockfd;
    globalHeartbeatAddress = servaddr;
    signal(SIGALRM, handle_heartbeat_signal);
    //alarm(HEARTBEAT_RATE);
}

int login_user(char* username, int server_port)
{
    char buffer[MAXLINE];
    strcpy(my_username, username);


    int sockfd = get_tcp_socket();

    struct sockaddr_in servaddr = get_server_address(server_port);

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(sockfd, itoa(my_port), strlen(itoa(my_port))) < ZERO) 
    {
    	print("port sending failed");
    	return FALSE;
    }

    if (write(sockfd, LOGIN_COMMAND, strlen(LOGIN_COMMAND)) < ZERO) 
    {
    	print("login command sending failed"); 
    	return FALSE;
    }   
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("Login Accepted!\n")-1) < ZERO) 
    {
    	print("login command Accept failed");
    	return FALSE;
    }

    if (write(sockfd, username, strlen(username)) < ZERO) 
    {
    	print("username sending failed");
    	return FALSE;
    }
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("username Accepted")-1) < ZERO) 
    {
    	print("username accept failed");
    	return FALSE;
    }

    close(sockfd);
    return TRUE;
}

int create_group(char* group_name, int server_port)
{
    char buffer[MAXLINE];

    int sockfd = get_tcp_socket();

    struct sockaddr_in servaddr = get_server_address(server_port);

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(sockfd, itoa(my_port), strlen(itoa(my_port))) < ZERO) 
    {
    	print("port sending failed");
    	return FALSE;
    }

    if (write(sockfd, CREATE_GROUP_COMMAND, strlen(CREATE_GROUP_COMMAND)) < ZERO) 
    {
    	print("create_group command sending failed"); 
    	return FALSE;
    }   
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof(buffer)) < ZERO) 
    {
    	print("create_group command Accept failed");
    	return FALSE;
    }

    if (write(sockfd, group_name, strlen(group_name)) < ZERO) 
    {
    	print("group_name sending failed");
    	return FALSE;
    }
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof(buffer)) < ZERO) 
    {
    	print("group_name accept failed");
    	return FALSE;
    }

    close(sockfd);
    return TRUE;
}

int join_group(char* group_name, int server_port)
{
    char buffer[MAXLINE];
    int sockfd = get_tcp_socket();

    struct sockaddr_in servaddr = get_server_address(server_port);

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(sockfd, itoa(my_port), strlen(itoa(my_port))) < ZERO) 
    {
    	print("port sending failed");
    	return FALSE;
    }

    if (write(sockfd, JOIN_GROUP_COMMAND, strlen(JOIN_GROUP_COMMAND)) < ZERO) 
    {
    	print("join_group command sending failed"); 
    	return FALSE;
    }   
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof(buffer)) < ZERO) 
    {
    	print("join_group command Accept failed");
    	return FALSE;
    }

    if (write(sockfd, group_name, strlen(group_name)) < ZERO) 
    {
    	print("group_name sending failed");
    	return FALSE;
    }
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("group_name Accepted")-1) < ZERO) 
    {
    	print("group_name accept failed");
    	return FALSE;
    }

    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, 4) < ZERO) 
    {
    	print("group_port receiving failed");
    	return FALSE;
    }

    if (strcmp(buffer, "fail") == EQUAL) 
    {
    	return FALSE;
    }

    group_port = atoi(buffer);

    close(sockfd);
    start_group_chatting();
    return TRUE;
}

int start_secret_chat(char* username, int server_port)
{
    char buffer[MAXLINE];

    int sockfd = get_tcp_socket();

    struct sockaddr_in servaddr = get_server_address(server_port);

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(sockfd, itoa(my_port), strlen(itoa(my_port))) < ZERO) 
    {
    	print("port sending failed");
    	return FALSE;
    }

    if (write(sockfd, START_SECRET_CHAT_COMMAND, strlen(START_SECRET_CHAT_COMMAND)) 
    	!= strlen(START_SECRET_CHAT_COMMAND)) 
    {
    	print("start_secret_chat command sending failed"); 
    	return FALSE;
    }   
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("Start Secret Chat Accepted!\n")-1) < ZERO) 
    {
    	print("start_secret_chat command Accept failed");
    	return FALSE;
    }
    if (write(sockfd, username, strlen(username)) != strlen(username)) 
    {
    	print("username sending failed");
    	return FALSE;
    }
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("username Accepted")-1) < ZERO) 
    {
    	print("username accept failed");
    	return FALSE;
    }
    if (write(sockfd, "give me her port", strlen("give me her port")) != 
    	strlen("give me her port"))
    {
    	print("request sending failed");
    	return FALSE;
    }

    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, 4) < ZERO) 
    {
    	print("user_port receiving failed");
    	return FALSE;
    }

    friend_port = atoi(buffer);

    close(sockfd);
    start_secret_chatting();
    return TRUE;
}

int start_secret_chatting()
{
	friend_socket = get_tcp_socket();
	char buffer[MAXLINE];
	struct sockaddr_in servaddr = get_server_address(friend_port);

	if (connect(friend_socket, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(friend_socket, itoa(my_port), strlen(itoa(my_port))) < ZERO) 
    {
    	print("port sending failed");
    	return FALSE;
    }
    bzero(buffer, sizeof(buffer));
    if (read(friend_socket, buffer, sizeof("PEER SERVER: WELCOME TO SERVER \r\n")-1) < ZERO)
    {
    	print("reading welcome failed"); 
    	return FALSE;
    }
    print(buffer);
    return TRUE;
}

int get_group_list(int server_port)
{
    char buffer[MAXLINE];
    int sockfd = get_tcp_socket();

    struct sockaddr_in servaddr = get_server_address(server_port);

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(sockfd, itoa(my_port), strlen(itoa(my_port))) < ZERO) 
    {
    	print("port sending failed");
    	return FALSE;
    }

    if (write(sockfd, GET_GROUP_LIST_COMMAND, strlen(GET_GROUP_LIST_COMMAND)) < ZERO) 
    {
    	print("get_group_list command sending failed"); 
    	return FALSE;
    }   
    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("Get Group List Accepted!\n")-1) < ZERO) 
    {
    	print("get_group_list command Accept failed");
    	return FALSE;
    }

    bzero(buffer, sizeof(buffer));
    int valread;
    if ((valread =read(sockfd, buffer, sizeof(buffer))) <= ZERO) 
    {
    print("group_list receiving failed");
    return FALSE;
    }

    print(buffer);

    close(sockfd);
    return TRUE;
}

void broadcast(char* buffer)
{
    if (sendto(group_socket, buffer, strlen(buffer),
           /*MSG_CONFIRM*/ ZERO, (const struct sockaddr *) &group_address,
           sizeof(group_address)) != strlen(buffer)){

        perror("broadcasting message failed");
        exit(EXIT_FAILURE);
    }

    print("sent to group: |");
    print(buffer);
    print("|\n");
}

void start_group_chatting()
{
	group_socket = get_udp_socket();
    set_broadcast_options(group_socket);
    group_address = get_broadcast_address(group_port);
    if (bind(group_socket, (struct sockaddr *)&group_address, sizeof(group_address))<ZERO) 
    {
    	perror("Broadcast bind failed"); 
    	exit(EXIT_FAILURE);
    }
}

void handle_leave_group()
{
	close(group_socket);
	group_port = ZERO;
}

int handle_leave_secret_chat()
{
	friend_socket = get_tcp_socket();
	struct sockaddr_in servaddr = get_server_address(friend_port);
	if (connect(friend_socket, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

	if (write(friend_socket, "bye", strlen("bye")) < ZERO) 
    {
    	print("bye sending failed");
    	return FALSE;
    }

	friend_port = ZERO;
	close(friend_socket);
}

int handle_friend_chat(char* message)
{
	friend_socket = get_tcp_socket();
	struct sockaddr_in servaddr = get_server_address(friend_port);

	char messages[MAXLINE];
    bzero(messages, sizeof(messages));
    strcat(messages, my_username);
    strcat(messages, ": ");
    strcat(messages, message);
    strcat(messages, "\n");

	if (connect(friend_socket, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

	if (write(friend_socket, messages, strlen(messages)) < ZERO) 
    {
    	print("message sending failed");
    	return FALSE;
    }
}

int handle_group_chat(char* message)
{
	group_socket = get_udp_socket();
	set_broadcast_options(group_socket);
	group_address = get_broadcast_address(group_port);

	char messages[MAXLINE];
    bzero(messages, sizeof(messages));
    strcat(messages, my_username);
    strcat(messages, ": ");
    strcat(messages, message);
    strcat(messages, "\n");

	bind_socket(group_socket, group_address);

	broadcast(messages);
}

int run_client(int server_port)
{
	int opt = TRUE;
    int addrlen, new_socket, client_socket[30], max_clients = 30, 
    	activity, i , valread , sd;
    int max_sd;
	char buffer[MAXLINE];

	fd_set readfds;

    //a message
    char *message = "PEER SERVER: WELCOME TO SERVER \r\n";

	for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

	//<........create tcp server socket for connecting to server.........
	 //create a master socket
    int server_socket = get_tcp_socket();
    struct sockaddr_in servaddr = get_server_address(server_port);    
    if (connect(server_socket, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < ZERO) 
    {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    if (write(server_socket, "Inew", strlen("Inew")) < ZERO) 
    {
    	print("I am new sending failed");
    	return FALSE;
    }

    bzero(buffer, sizeof(buffer));
    if (read(server_socket, buffer, sizeof("SERVER: WELCOME TO SERVER \r\n")-1) < ZERO)
    {
    	print("reading welcome failed"); 
    	return FALSE;
    }

    print(buffer);
    bzero(buffer, sizeof(buffer));
    if(read(server_socket, buffer, sizeof(buffer)) < ZERO)
    {
    	print("port receiving failed");
    	return FALSE;
    }

    my_port = atoi(buffer);
    //........create tcp server socket for connecting to server.........>

    //------------------<CREATE CLIENT SOCKET FOR SECRET CHATTING----
    //create a master socket
    int masterSocket = get_tcp_socket();
    //set master socket to allow multiple connections ,
    set_reusable_option(masterSocket);
    //type of socket created
    struct sockaddr_in address = get_server_address(my_port);
    //bind the socket to localhost port serverPort
    if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address))<ZERO) 
    {
    	perror("Client bind failed"); 
    	exit(EXIT_FAILURE);
    }
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(masterSocket, MAX_PENDING) < ZERO) 
    {
    	perror("listen"); 
    	exit(EXIT_FAILURE);
    }
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections on clientPort ...");
    //------------------CREATE CLIENT SOCKET FOR SECRET CHATTING/>----

    //------------------<CREATE BROADCAST SOCKET FOR GROUP CHATTING----
    // close(globalBroadcastSocketFD);
    group_socket = get_udp_socket();
    set_broadcast_options(group_socket);
    group_address = get_broadcast_address(group_port);
    if (bind(group_socket, (struct sockaddr *)&group_address, sizeof(group_address))<0) 
    {
    	perror("Broadcast bind failed"); 
    	exit(EXIT_FAILURE);
    }
    //------------------CREATE BROADCAST SOCKET FOR GROUP CHATTING/>----

    FD_ZERO(&readfds);

    while(TRUE)
    {
    	if (group_port != 0){
    		FD_SET(group_socket, &readfds); 
    		FD_SET(STDIN_FILENO, &readfds); /* Add keyboard to descriptor vector */
    		FD_SET(masterSocket, &readfds); /* Add CLIENT socket to descriptor vector */
    		max_sd = max(masterSocket, group_socket);
    	}
    	else
    	{
    		FD_SET(STDIN_FILENO, &readfds); /* Add keyboard to descriptor vector */
    		FD_SET(masterSocket, &readfds); /* Add CLIENT socket to descriptor vector */
    		max_sd = masterSocket;
    	}

    	//add child sockets to set
    	for ( i = ZERO ; i < max_clients ; i++)
    	{
        	//socket descriptor
        	sd = client_socket[i];

        	//if valid socket descriptor then add to read list
        	if(sd > ZERO)
            	FD_SET( sd , &readfds);

        	//highest file descriptor number, need it for the select function
        	if(sd > max_sd)
            	max_sd = sd;
    	}

    	//wait for an activity on one of the sockets , timeout is NULL ,
    	//so wait indefinitely
    	struct timeval tv;
    	tv.tv_sec = (long)1;
    	tv.tv_usec = ZERO;
    	activity = select( max_sd + ONE , &readfds , NULL , NULL , &tv);
    	if ((activity < ZERO) && (errno!=EINTR))
        {
            print("select error");
        }
    	
    	while (errno==EINTR && FD_ISSET(masterSocket, &readfds)) //Alarm Interrupt
    	{
        	//print("ALARM INTERRUPT!");
        	activity = select( max_sd + ONE , &readfds , NULL , NULL , &tv);
   	 	}
        
    	if (FD_ISSET(masterSocket, &readfds))
    	{
        	print("MasterSocket Interrupt\n");
        	if ((new_socket = accept(masterSocket,
                                 (struct sockaddr *)&address, (socklen_t*)&addrlen))<ZERO)
        	{
            	perror("accept");
            	exit(EXIT_FAILURE);
        	}

        	bzero(buffer, sizeof(buffer));
            if ((valread = read( new_socket , buffer, sizeof(buffer))) < ZERO)
            	return FALSE;
            if (strlen(buffer) == 4)
            {
        		print("New connection\n");
        		//send new connection greeting message
        		if( send(new_socket, message, strlen(message), ZERO) != strlen(message) )
        		{
            		perror("send");
        		}

        		puts("Welcome message sent successfully");
                friend_socket = new_socket;
                print("friend found\n");
                friend_port = atoi(buffer);
            }
            else if (strlen(buffer) == 3)
            {
            	friend_port = ZERO;
				close(friend_socket);
            	print("chat finished!\n");
            }
            else 
            {
            	print(buffer);
            }
        	
    	}
    

    	if (FD_ISSET(STDIN_FILENO, &readfds))
    	{
        	print("INPUT Interrupt\n");
        	char *command, *argument;
        	char line[COMMAND_MAX_LENGTH];
        	valread = read(STDIN_FILENO, line, COMMAND_MAX_LENGTH);
        	line[valread-1] = '\0'; 
        	parse_input(&command, &argument, line);

            if(strcmp(command, LOGIN_COMMAND) == 0)
            {
                print(command); print(" "); print(argument); print("...\n");

                if(login_user(argument, server_port) == TRUE)
                {
                    print("User logged in successfully \n");
                }
                else
                {
                    print("login failed \n");
                }

            }

            else if(strcmp(command, JOIN_GROUP_COMMAND) == 0)
            {
                print(command); print(" "); print(argument); print("...\n");

                if(join_group(argument, server_port) == TRUE)
                {
                  print("User joined group successfully \n");
                }
                else
                {
                    print("Joining group failed \n");
                }
            }

            else if(strcmp(command, CREATE_GROUP_COMMAND) == 0)
            {
                print(command); print(" "); print(argument); print("...\n");

                if(create_group(argument, server_port) == TRUE)
                {
                  print("Group created successfully \n");
                }
                else
                {
                    print("Group creation failed \n");
                }
            }

            else if(strcmp(command, START_SECRET_CHAT_COMMAND) == 0)
            {
                print(command); print(" "); print(argument); print("...\n");

                if(start_secret_chat(argument, server_port) == TRUE)
                {
                  print("User started secret chat successfully \n");
                }
                else
                {
                    print("Starting secret chat failed \n");
                }
            }

            else if(strcmp(command, GET_GROUP_LIST_COMMAND) == 0)
            {
                print(command); print("...\n");

                if(get_group_list(server_port) == TRUE)
                {
                  print("User got group_list successfully \n");
                }
                else
                {
                    print("Getting group list failed \n");
                }
            }

            else if(strcmp(command, LEAVE_GROUP) == 0)
            {
                print(command); print("...\n");

                handle_leave_group();
                print("User left group successfully \n");
            }

            else if(strcmp(command, LEAVE_SECRET_CHAT) == 0)
            {
                print(command); print("...\n");

                handle_leave_secret_chat();
                print("User left chat successfully \n");
            }

            else if(strcmp(command, CHAT_SIGN) == 0)
            {
                print("sending"); print("...\n");

                handle_friend_chat(argument);
                print("User sent message successfully \n");
            }

            else if(strcmp(command, CHAT_SIGN_GROUP) == 0)
            {
                print("sending"); print("...\n");

                handle_group_chat(argument);
                print("User sent message successfully \n");
            }

            else
            {
                print("Please use valid commands!\n");
            }
            print("-----------------------------\n");
            print("-> Please write your command:\n");
    }
    	if (FD_ISSET(group_socket, &readfds))
    	{
    		print("Broadcast Interrupt: \n");
    		char buffer[MAXLINE];
            int valread;
            bzero(buffer, sizeof(buffer));

            socklen_t len = sizeof(group_address);
            if(valread = recvfrom(group_socket, (char *)buffer, MAXLINE,
                        MSG_WAITALL, (struct sockaddr *) &group_address,
                        &len) < 0){
                perror("Failed to read Broadcast");
                return -1;
            }
            
            print(buffer);
    	}

        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if(sd == STDIN_FILENO) continue;
            if (FD_ISSET( sd , &readfds))
            {
                print("Client Interrupt\n");
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);

                    print("Host disconnected\n");

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
            }
        }
     }
    return 0;
}

void parse_input(char** command, char**argument, char* line)
{
    (*command) = (char*) malloc(COMMAND_MAX_LENGTH * sizeof(char));
    (*argument) = (char*) malloc(COMMAND_MAX_LENGTH * sizeof(char));
    int is_argument = 0;
    int space_pos;
    for(int i=0; i<strlen(line); i++){
        if(line[i] == ' '){
            is_argument = 1;
            space_pos = i+1;
            (*command)[i] = '\0';
        }else{
            if(is_argument == 0){
                (*command)[i] = line[i];
            }else{
                (*argument)[i - space_pos] = line[i];
            }
        }
    }
    (*argument)[strlen(line) - space_pos] = '\0';
}

int main(int argc, char const *argv[])
{
	if (argc != NUMBER_OF_ARGUMENTS)
	{
		print("Number of arguments is Invalid!\n");
		exit(EXIT_FAILURE);
	}
	int server_port = atoi(argv[ONE]);

	start_heartbeat(server_port);
	run_client(server_port);

	while (1){

    }

	close(globalHeartbeatSocketFD);

	return 0;
}