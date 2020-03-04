#include  "server.h"

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

struct sockaddr_in get_broadcast_address(int port)
{
    struct sockaddr_in servaddr;
    memset(&servaddr, ZERO, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //INADDR_ANY
    return servaddr;
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

void bind_socket(int sockfd, struct sockaddr_in servaddr)
{
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < ZERO )
    {
        perror("bind failed!\n");
        exit(EXIT_FAILURE);
    }
}

void set_timeout_option(int rcv_sock, int seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = ZERO;
    if (setsockopt(rcv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, 
    	sizeof(tv)) < ZERO) 
    {
        perror("Error");
        close(rcv_sock);
        exit(EXIT_FAILURE);
    }
}

void free_group(int group_index)
{
	group_ports[group_index] = ZERO;
   	strcpy(group_names[group_index], EMPTY);
   	close(group_sockets[group_index]);
}

int remove_inactive_groups(int heartbeat_port)
{
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    sockfd = get_udp_socket();
    servaddr = get_broadcast_address(heartbeat_port);

    set_broadcast_options(sockfd);
    bind_socket(sockfd, servaddr);
    set_timeout_option(sockfd, HEARTBEAT_TIMEOUT);

    socklen_t len = sizeof(servaddr);
    int valread;
    valread = recvfrom(sockfd, (char *)buffer, sizeof(buffer),
                 MSG_WAITALL, (struct sockaddr *) &servaddr,&len);

   	for (int i = ZERO; i < MAX_NUMBER_OF_GROUPS; ++i)
   	{
   		if (strstr(buffer, itoa(group_ports[i])) == NULL)
   		{
   			print("The group on port ");
   			print(itoa(group_ports[i]));
   			print(" deleted!\n");

   			free_group(i);
   		}
   	}
}

int get_port(int sockfd, struct sockaddr_in sin)
{
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) < ZERO)
    {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    return ntohs(sin.sin_port);
}

void initialize_users()
{
	for (int i = ZERO; i < MAX_NUMBER_OF_USERS; i++)
    {
        user_sockets[i] = ZERO;
    }
}

void initialize_groups()
{
	for (int i = ZERO; i < MAX_NUMBER_OF_USERS; i++)
    {
        group_ports[i] = ZERO;
    }
}

int handle_login_user(int sockfd, int user_index)
{
    int value;
    write(sockfd, "Login Accepted!\n", strlen("Login Accepted!\n")) != strlen("Login Accepted!\n");

    char username[MAXLINE];
    bzero(username, sizeof(username));

    if (value = read(sockfd, username, sizeof(username)) < ZERO) 
    {
        return UNSUCCESSFUL;
    }
    write(sockfd, "username Accepted", strlen("username Accepted"));

    print("Request for Login "); print(username); print("\n");
    
    strcpy(users[user_index], username);
    print(username); print(" Logged in!"); print("\n");
    return SUCCESSFUL;
}

int handle_get_group_list(int sockfd)
{
	int value;
    write(sockfd, "Get Group List Accepted!\n", 
    	strlen("Get Group List Accepted!\n"));

    print("Request for Getting Group List\n");

    //remove_inactive_groups(server_port);
    char message[MAXLINE];
    bzero(message, sizeof(message));
    for (int i = ZERO; i < MAX_NUMBER_OF_GROUPS; ++i)
    {
        if (group_ports[i] != ZERO)
        {
        	strcat(message, group_names[i]);
        	strcat(message, "\n");
        }
    }
    print(message);

    if(write(sockfd, message, strlen(message)) != strlen(message))
    	print("sending group list failed!");
    return SUCCESSFUL;
}

int set_group_socket(int group_index)
{
	int port = BASE_GROUP_PORT + group_index;
    group_sockets[group_index] = get_udp_socket();
    set_broadcast_options(group_sockets[group_index]);

    struct sockaddr_in servaddr = get_broadcast_address(port);

    bind_socket(group_sockets[group_index], servaddr);

    print("Listener on port "); print(itoa(port)); print("\n");
    return port;
}

int handle_create_group(int sockfd)
{
    int value;
    write(sockfd, "Create Group Accepted!\n", 
    	strlen("Create Group Accepted!\n"));

    char group_name[MAXLINE];
    bzero(group_name, sizeof(group_name));

    if (value = read(sockfd, group_name, 
    	sizeof(group_name)) < ZERO) 
    {
        return UNSUCCESSFUL;
    }
    write(sockfd, "group_name Accepted", strlen("group_name Accepted"));

    print("Request for creating "); print(group_name); print("\n");

    for (int i = ZERO; i < MAX_NUMBER_OF_GROUPS; ++i)
    {
        if (group_ports[i] == 0)
        {
            group_ports[i] = i + BASE_GROUP_PORT; 
            set_group_socket(i);
            strcpy(group_names[i], group_name);
            print("Someone Created "); print(group_name); print("!\n");
            return SUCCESSFUL;
        }
    }

    print("Group creation failed!\n");
    return UNSUCCESSFUL;
}

int handle_join_group(int sockfd)
{
    int value;
    char buffer[MAXLINE];

    write(sockfd, "Join Group Accepted!\n", strlen("Join Group Accepted!\n"));

    char group_name[MAXLINE];
    bzero(group_name, sizeof(group_name));

    if (value = read(sockfd, group_name, sizeof(group_name)) < ZERO) 
    {
        return UNSUCCESSFUL;
    }

    write(sockfd, "group_name Accepted", strlen("group_name Accepted"));

    print("Request for joining "); print(group_name); print("\n");

    //remove_inactive_groups(server_port);

    for (int i = ZERO; i < MAX_NUMBER_OF_GROUPS; ++i)
    {
        if (strcmp(group_names[i], group_name) == EQUAL)
        {
        	char* message = itoa(group_ports[i]);
            write(sockfd, message, strlen(message));
            print("Someone Joined "); print(group_name); print(" on port "); 
            print(message); print("\n");
            return SUCCESSFUL;
        }
    }
    write(sockfd, "fail", strlen("fail"));
    print("Join Group Failed!\n");
    return UNSUCCESSFUL;
}

int handle_start_secret_chat(int sockfd)
{
	char buffer[MAXLINE];
    int value;
    write(sockfd, "Start Secret Chat Accepted!\n", 
    	strlen("Start Secret Chat Accepted!\n"));

    char username[MAXLINE];
    bzero(username, sizeof(username));

    if (value = read(sockfd, username, sizeof(username)) < ZERO) 
    {
        return UNSUCCESSFUL;
    }

    write(sockfd, "username Accepted", strlen("username Accepted"));

    bzero(buffer, sizeof(buffer));
    if (read(sockfd, buffer, sizeof("give me her port")-1)< ZERO)
    {
    	print("request getting failed");
    	return FALSE;
    }

    print("Request for chatting with "); print(username); print("\n");


    for (int i = ZERO; i < MAX_NUMBER_OF_USERS; ++i)
    {
        if (strcmp(users[i], username) == EQUAL)
        {
            write(sockfd, itoa(user_ports[i]), strlen(itoa(user_ports[i])));
            print("Someone started secret chat with "); print(username); print("!\n");
            return SUCCESSFUL;
        }
    }

    print("Start Secret Chat Failed!\n");
    return UNSUCCESSFUL;
}

void handle_commands(int sockfd, int user_index, char* command)
{
	if (strcmp(command, LOGIN_COMMAND) == EQUAL)
		handle_login_user(sockfd, user_index);

	else if (strcmp(command, GET_GROUP_LIST_COMMAND) == EQUAL) 
		handle_get_group_list(sockfd);

	else if (strcmp(command, CREATE_GROUP_COMMAND) == EQUAL)
		handle_create_group(sockfd);

	else if (strcmp(command, JOIN_GROUP_COMMAND) == EQUAL)
		handle_join_group(sockfd);

	else if (strcmp(command, START_SECRET_CHAT_COMMAND) == EQUAL)
		handle_start_secret_chat(sockfd);
}

int run_server(int server_port)
{
	group_sockets = malloc(sizeof(int)*MAX_NUMBER_OF_GROUPS);
	group_ports = malloc(sizeof(int)*MAX_NUMBER_OF_GROUPS);

	user_sockets = malloc(sizeof(int)*MAX_NUMBER_OF_USERS);
	user_ports = malloc(sizeof(int)*MAX_NUMBER_OF_USERS);

    int opt = TRUE;
    int master_socket , addrlen , new_user, activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "SERVER: WELCOME TO SERVER \r\n";

    //initialise all user_sockets[] to 0 so not checked
    initialize_users();
    initialize_groups();

    //create a master socket
    master_socket = get_tcp_socket();

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < ZERO )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS_IP); //INADDR_ANY
    address.sin_port = htons( server_port );

    //bind the socket to localhost port server_port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<ZERO)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, MAX_PENDING) < ZERO)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    print("Waiting for connections ...");
     //------------------<CREATE BROADCAST SOCKET FOR UDP PEER2PEER----
    // close(globalBroadcastSocketFD);
    int heartbeat_socket = get_udp_socket();
    set_broadcast_options(heartbeat_socket);
    struct sockaddr_in heartbeat_address = get_broadcast_address(server_port);
    if (bind(heartbeat_socket, (struct sockaddr *)&heartbeat_address, sizeof(heartbeat_address))<0) 
    {
    	perror("Broadcast bind failed"); 
    	exit(EXIT_FAILURE);
    }
    //------------------CREATE BROADCAST SOCKET FOR UDP PEER2PEER/>----
    clock_t start, end;
    start = clock();
    while(TRUE)
    {
    	end = clock();
    	int msec = (end - start) * 1000 / CLOCKS_PER_SEC;
    	//print(itoa(msec));
        if(msec > HEARTBEAT_TIMEOUT*100){
            socklen_t len = sizeof(heartbeat_address);
    		int valread;
    		valread = recvfrom(heartbeat_socket, (char *)buffer, sizeof(buffer),
                 	MSG_WAITALL, (struct sockaddr *) &heartbeat_address,&len);
    			for (int i = ZERO; i < MAX_NUMBER_OF_GROUPS; ++i)
   				{
   						if (strstr(buffer, itoa(group_ports[i])) == NULL)
   						{
   							print("The group on port ");
   							print(itoa(group_ports[i]));
   							print(" deleted!\n");

   							free_group(i);
   						}
   				}
    			
            start = clock();
        }
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = ZERO ; i < MAX_NUMBER_OF_USERS ; i++)
        {
            //socket descriptor
            sd = user_sockets[i];

            //if valid socket descriptor then add to read list
            if(sd > ZERO)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            max_sd = max(sd, max_sd);
        }

        struct timeval tv;
    	tv.tv_sec = (long)1;
    	tv.tv_usec = ZERO;
    	activity = select( max_sd + ONE , &readfds , NULL , NULL , &tv);

    	if ((activity < ZERO) && (errno!=EINTR))
    	{
        	print("select error");
    	}

    	while (errno==EINTR && FD_ISSET(master_socket, &readfds)) //Alarm Interrupt
    	{
        	print("ALARM INTERRUPT!");
        	activity = select( max_sd + ONE , &readfds , NULL , NULL , &tv);
   	 	}
        
        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_user = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t*)&addrlen))<ZERO)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            bzero(buffer, sizeof(buffer));
            if ((valread = read( new_user , buffer, 4)) < ZERO)
            	return FALSE;

            if (strcmp(buffer, "Inew") == 0)
            {
            print("New connection\n");

            //send new connection greeting message
            if( send(new_user, message, strlen(message), ZERO) != strlen(message))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets

            for (i = ZERO; i < MAX_NUMBER_OF_USERS; i++)
            {
                //if position is empty
                if( user_sockets[i] == ZERO )
                {
                    user_sockets[i] = new_user;
                    user_ports[i] = i + BASE_PORT;

                    char* port = itoa(user_ports[i]);
                    if(send(new_user, port, strlen(port), ZERO) != strlen(port))
            		{
                		perror("send");
            		}

                    print("Adding to list of users\n");
                    break;
                }
            }
        }
        else{
        		int her_port = atoi(buffer);
        		int user_index = 0;
        		user_index = her_port - BASE_PORT;
        		
        		bzero(buffer, sizeof(buffer));
                if ((valread = read( new_user , buffer, 1024)) > ZERO)
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    print(buffer); print("\n");
                    handle_commands(new_user, user_index, buffer);
                }

        	}
        }

        //else its some IO operation on some other socket
        for (i = ZERO; i < MAX_NUMBER_OF_USERS; i++)
        {
            sd = user_sockets[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                bzero(buffer, sizeof(buffer));
                if ((valread = read( sd , buffer, 1024)) == ZERO)
                {
                    //Somebody disconnected
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);

                    print("Host disconnected\n");

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    user_sockets[i] = ZERO;
                }

                else
                {
                    buffer[valread] = '\0';
                    print(buffer); print("\n");
                    handle_commands(sd, i, buffer);
                }
            }
        }
    }

    return 0;
}

int main(int argc, char const *argv[])
{
	if (argc != NUMBER_OF_ARGUMENTS)
	{
		print("Number of arguments is Invalid!\n");
		exit(EXIT_FAILURE);
	}
	server_port = atoi(argv[ONE]);
	run_server(server_port);

	while (TRUE)
	{
    }

	return 0;
}