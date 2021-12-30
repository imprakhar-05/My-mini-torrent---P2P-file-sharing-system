#include<bits/stdc++.h>
#include<iostream>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#define BUFF 2048
#define CHUNK 512*1024
#define clienout cout<<"\nclient: "
#define servrout cout<<"\nserver: "

using namespace std;

char Tracker_IPAdd[20];
char SERVER_IP[20];
int Tracker_Port, PORT;

bool mutex_server_start=false;
char * uid = new char [256];


struct compare_string
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};


struct struct_file_details
{
	int size;
	int no_of_chunks;
	char * bit_vector;
};
std::map<char * , struct struct_file_details *,compare_string> map_file_details;


void *server_request(void *arg)
{

  int sockfd=*((int *)arg);

  char file_name[256];
  int interval,start_point;
  bool ack=true;
  
  recv(sockfd, &start_point, sizeof(start_point),0);
  send(sockfd,&ack,sizeof(ack),0);
  servrout;cout<<"\nstart_point "<<start_point<<"\n";
  
  memset ( file_name , '\0', 256);
  recv(sockfd, &file_name, sizeof(file_name),0);
  send(sockfd,&ack,sizeof(ack),0);
  servrout;cout<<"\nRequest for file "<<file_name<<"\n";


  recv(sockfd, &interval, sizeof(interval),0);
  send(sockfd,&ack,sizeof(ack),0);

  servrout;cout<<"\ninterval "<<interval<<"\n";

  FILE * fp = fopen ( file_name  , "rb" );

  if ( fp == NULL )
  {
      printf( "Could not open file test.c" ) ;
      //return 1;
  } 

  fseek ( fp , 0 , SEEK_END);
  int size = ftell ( fp );
  rewind ( fp );

  auto itr=map_file_details.find(file_name);
  struct struct_file_details * f = itr->second;

  cout<<"\n"<<f->bit_vector;

  send ( sockfd , &size, sizeof(size), 0);
  recv(sockfd,&ack,sizeof(ack),0);

  int number_of_chunks=((size-1)/(CHUNK))+1;

  send(sockfd,f->bit_vector,strlen(f->bit_vector),0);
  recv(sockfd,&ack,sizeof(ack),0);


  
  char bit_vector[number_of_chunks]="";
  recv(sockfd,&bit_vector,sizeof(bit_vector),0);
  cout<<"\nno of chunks "<<number_of_chunks;
  cout<<"\n\tbitvector-";

  for(int i=0;i<number_of_chunks;i++)
    cout<<bit_vector[i];
  cout<<"\n";

  char Buffer[BUFF] ; 
  int n=0;
  //int i = start_point;

  
  for(int i=0;i<number_of_chunks;i++)
  {
    fseek(fp,i*CHUNK,SEEK_SET);
    char big_Buffer[CHUNK];

    if(bit_vector[i]=='1')
    {
      servrout;cout<<"\n sending i chunk-"<<i;
      memset(big_Buffer,'\0',CHUNK);
      int no_of_bytes_sended=0;
      int ack=0;
      while ( CHUNK > no_of_bytes_sended )
      {
        //fseek(fp,i*BUFF,SEEK_SET);

        if( ( n = fread( Buffer , sizeof(char) , BUFF , fp ) ) > 0)
        {
          //n = fread( Buffer , sizeof(char) , BUFF , fp );
          servrout;cout<<n;
          send (sockfd , Buffer, n, 0 );
            servrout;cout<<n<<" "<<i<<" "<<ack<<" "<<n<<" "<<no_of_bytes_sended;

            memset ( Buffer , '\0', BUFF);

            recv( sockfd , &ack ,sizeof(ack), 0);
          //size = size - i ;
            //i+=interval;
            no_of_bytes_sended+=BUFF;
        }
        else
        { servrout;cout<<"breaking while loop";
          break;
        } //printf("\nSending File  %d \n ",size);
      }
    }
  }
  cout<<"\n Sending file complete";
  // fclose ( fp ); 


  close( sockfd);
}


void * server(void * argv)
{
  cout<<"ram";

  //init();

  int server_fd = socket (AF_INET, SOCK_STREAM, 0);
  
  pthread_t tid;

  // cout<<"Enter port for process";
  // cin>>PORT;

  struct sockaddr_in   server_addr,client_addr;

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons( PORT );
  server_addr.sin_addr.s_addr=inet_addr(SERVER_IP);


  cout<<"\nServer port no - "<<ntohs(server_addr.sin_port)<<"\n";

  int addrlen = sizeof(sockaddr);

  if(bind (server_fd  , (struct sockaddr *)&server_addr , sizeof ( server_addr ) ) !=0)
  {
    servrout;cout<<"\nError in Binding Socket\n";
    exit(1);
  }
  mutex_server_start=true;
  listen (server_fd, 3);
  cout<<"\nListening";
  
  while(1)
  {

    int sockfd = accept ( server_fd , (struct sockaddr *)&client_addr , (socklen_t*)&addrlen);
    cout<<"\nport no - "<<ntohs(client_addr.sin_port)<<"\n";
    cout<<"\nIp add  - "<<inet_ntoa(client_addr.sin_addr);
    cout<<"\nConnection Established";
    
    if( pthread_create(&tid,NULL,&Server_request,&sockfd) !=0 )
      printf("Failed to create thread\n");

  }


  close( server_fd);  
}
struct client_addr
{	
	char *file_name;
	char *IP;
	char *bit_vector;
	int PORT;
	int start_point;
	int interval;
	int file_size; 
	bool first_client=false;

};

std::map<char *, vector<struct client_addr *> *,compare_string > map_downloading;

struct mutex_struct
{
	int no_of_client;
	bool mutex;
};

std::map<char*, struct mutex_struct *,compare_string> map_mutex;

void handler_upload_command()
{
	//cout<<"\nEnter filename to upload\n";
    char *file_name = new char [256];
    cin>>file_name;
    int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( Tracker_Port );
	serv_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  { 
        printf("\nConnection Failed with Tracker\n"); 
        pthread_exit(NULL);
    } 

    send(sockfd,"upload_file",strlen("upload_file"),0);
    bool ack=false;
    recv(sockfd , &ack ,sizeof(ack),0 );

    FILE *fp = fopen ( file_name  , "rb" );

    fseek ( fp , 0 , SEEK_END);
	int size = ftell ( fp );
	rewind ( fp );

	int number_of_chunks=((size-1)/(CHUNK))+1;
	char * bit_vector= new char[number_of_chunks];

	for(int j=0;j<number_of_chunks;j++)
		bit_vector[j]='1';

	struct struct_file_details * f = new struct_file_details();
	f->size=size;
	f->bit_vector=bit_vector;
	f->no_of_chunks=number_of_chunks;

	map_file_details.insert({file_name,f});	

	send(sockfd,file_name,strlen(file_name),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,&size,sizeof(size),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,&PORT,sizeof(PORT),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,uid,strlen(uid),0);
	recv(sockfd,&ack,sizeof(ack),0);

	fclose(fp);
	close(sockfd);

    for (auto itre = map_file_details.begin(); itre != map_file_details.end(); ++itre) { 
        
        cout <<"\n" << itre->first ;
        struct struct_file_details * v=itre->second;
        cout<<"\n"<<v->size<<"\t"<<v->bit_vector<<"\t"<<v->no_of_chunks;
        
    } 

}

void piece_selection(char *file_name)
{	
	auto itr = map_downloading.find(file_name);
	std::vector<struct client_addr *> * v=itr->second;

	for(int i=0;i<5;i++)
		cout<<"\n"<<file_name<<" size - "<<v->size();

	for(int i=0;i<v->size();i++)
   	{
   		cout<<"\n"<<v->at(i)->IP<<" "
   		 <<v->at(i)->PORT<<" "
   		  <<v->at(i)->start_point<<" "
   		  <<v->at(i)->interval<<" "
   		  <<v->at(i)->file_size<<" "
   		  <<v->at(i)->bit_vector;
   	}

   	int no_bits_one[v->size()]={0};
   	int no_of_ones_used[v->size()]={0};

   	// int no_of_chunks=( (v->at(0)->file_size-1)/CHUNK)+1;
   	int no_of_chunks = strlen(v->at(0)->bit_vector)-1;
   	cout<<"\nno_of_chunks "<<no_of_chunks;


   	for (int i = 0; i < v->size(); ++i)
   	{
   		cout<<"check";
   		for(int j=0 ;j<no_of_chunks;j++)
   		{
   			if(v->at(i)->bit_vector[j]=='1')
   				no_bits_one[i]++;
   		}
   		cout<<"\n"<<no_bits_one[i]<<" "<<v->at(i)->bit_vector;
   	}

   	for (int i = 0; i < no_of_chunks; ++i)
   	{
   		int rslt_client=0;
   		int temp_no_of_ones=INT_MAX;
   		int temp_no_of_ones_used=INT_MAX;

   		for(int j =0 ;j<v->size();j++)
   		{
   			if(v->at(j)->bit_vector[i]=='1')
   			{
   				if(temp_no_of_ones>no_bits_one[j])
   				{
   					rslt_client=j;
   					temp_no_of_ones=no_bits_one[j];
   					temp_no_of_ones_used= no_of_ones_used[j];
   				}

   				if(temp_no_of_ones==no_bits_one[j] && temp_no_of_ones_used > no_of_ones_used[j])
   				{
   					rslt_client=j;
					temp_no_of_ones=no_bits_one[j];
   					temp_no_of_ones_used= no_of_ones_used[j];   					
   				}

   			}
   		}

   		for(int j=0;j<v->size();j++)
   		{
   			if(rslt_client==j)
   			{
   				v->at(j)->bit_vector[i]='1';
   				no_bits_one[j]--;
   				no_of_ones_used[j]++;
   			}
   			else
   			{
   				if(v->at(j)->bit_vector[i]='1')
   				{
   					no_bits_one[j]--;
   					v->at(j)->bit_vector[i]='0';
   				}
   			}

   		}
   	}
   	cout<<"\t check2";
   	char * bit_vector =new char[no_of_chunks];
	struct struct_file_details * f = new struct_file_details();
	f->size=v->at(0)->file_size;
	f->bit_vector=bit_vector;
	f->no_of_chunks=no_of_chunks;

	for (int i = 0; i < no_of_chunks; ++i)
	{
		bit_vector[i]='0';
	}

	map_file_details.insert({file_name,f});
	cout<<"\nuploading file detais";
   	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( Tracker_Port );
	serv_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed with Tracker\n"); 
        pthread_exit(NULL);
    } 		

    send(sockfd,"upload_file",strlen("upload_file"),0);
    bool ack=false;
    recv(sockfd , &ack ,sizeof(ack),0 );

	send(sockfd,file_name,strlen(file_name),0);
	recv(sockfd,&ack,sizeof(ack),0);

	cout<<"\nfile_size "<<v->at(0)->file_size;
	send(sockfd,&(v->at(0)->file_size),sizeof(v->at(0)->file_size),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,&PORT,sizeof(PORT),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,uid,strlen(uid),0);
	recv(sockfd,&ack,sizeof(ack),0);
	close(sockfd);

}

void * download_file(void * argv)
{

	struct client_addr * client =( (struct client_addr *) argv );

	
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	bool ack=true;
	struct sockaddr_in    serv_addr;

	char *file_name=client->file_name;
	int start_point=client->start_point;
	int interval = client->interval;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( client->PORT );
	serv_addr.sin_addr.s_addr=inet_addr(client->IP);

	clienout;cout<<"\n Thread created for download";

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        pthread_exit(NULL);
    } 

    auto itr=map_mutex.find(file_name);
    struct mutex_struct * _mutex_struct = itr->second;

    clienout;
    // send ( sockfd , &size, sizeof(size), 0);
    
    send(sockfd,&start_point ,sizeof(start_point),0);
    recv(sockfd,&ack,sizeof(ack),0);

    send(sockfd,file_name,strlen(file_name),0);
    recv(sockfd,&ack,sizeof(ack),0);
    cout<<"\n start_point 00 "<<client->start_point<<" interval"<<client->interval;

    sleep(1);
    send ( sockfd , &interval, sizeof(interval), 0);
    recv(sockfd,&ack,sizeof(ack),0);
    
    char create_file_name[256];
   //sprintf(create_file_name,"copy_%s",file_name);
   sprintf(create_file_name,"%s",file_name);


	FILE *fp = fopen ( create_file_name  , "wb+" );

	char Buffer [ BUFF]; 
	int file_size=0;
	sleep(1);
	recv(sockfd, &file_size, sizeof(file_size), 0);
	send(sockfd,&ack,sizeof(ack),0);

	cout<<"\nfile_size - "<<file_size;

	int number_of_chunks=((file_size-1)/(CHUNK))+1;

	char bit_vector[number_of_chunks]="";
	int temp=client->start_point;
	cout<<"\nno of chunks "<<number_of_chunks;

	int to_make_zero =recv(sockfd,&bit_vector,sizeof(bit_vector),0);
	send(sockfd,&ack,sizeof(ack),0);
	cout<<"\n\tbitvector-"<<bit_vector<<"\n";
	
	for (int i = to_make_zero-1; i < number_of_chunks; ++i)
	{
		bit_vector[i]='0';
	}

	// sleep(3);

	client->bit_vector=bit_vector;

	_mutex_struct->no_of_client=(_mutex_struct->no_of_client-1);
	cout<<"\nno of clients remain "<<_mutex_struct->no_of_client<<"\n";

	while(_mutex_struct->no_of_client)
		cout<<"\tw1 ";

	if(client->first_client==true)
	{
		
		piece_selection(file_name);
		_mutex_struct->mutex=false;
	}

	while(_mutex_struct->mutex)
		cout<<"\tw2";

	cout<<"\n\tbitvector-"<<bit_vector<<"\n";

	send(sockfd,bit_vector,number_of_chunks,0);


	int n=0;

	for(int i=0;i<number_of_chunks;i++)
	{
		fseek(fp,i*CHUNK,SEEK_SET);
		char big_Buffer[CHUNK];
		if(bit_vector[i]=='1')
		{
			memset(big_Buffer,'\0',CHUNK);
			clienout;cout<<"\n recieving i chunk-"<<i;			
			int no_of_bytes_recieved=0;
			int k=0;
			int ack=0;
			int no_of_bytes_to_recieve=0;
			if(file_size>((i*CHUNK)+CHUNK) ){
				no_of_bytes_to_recieve=CHUNK;
			}
			else{
				no_of_bytes_to_recieve=file_size-(i*CHUNK);
			}

			while ( no_of_bytes_to_recieve > no_of_bytes_recieved  ){
				//fseek(fp,i*BUFF,SEEK_SET);
				//if(( n = recv( sockfd , Buffer , BUFF, 0) ) > 0)
				{	
					n = recv( sockfd , Buffer , BUFF, 0);
					//clienout;cout<<n<<" "<<i<<" "<<ack;					
					send (sockfd ,&ack,sizeof(ack), 0 );
					ack++;
					for(int j=0;j<n;j++){
						big_Buffer[k]=Buffer[j];
						k++;
					}
					memset ( Buffer , '\0', BUFF);
					//file_size = file_size - n;
					//i+=client->interval;
					no_of_bytes_recieved+=n;
				}
			}
			fwrite(big_Buffer,sizeof(char),no_of_bytes_recieved,fp);
			// sleep(4);

			auto itr=map_file_details.find(file_name);
			struct struct_file_details * f = itr->second;

			f->bit_vector[i]='1';
			cout<<" updated bit_vector "<<f->bit_vector;
		}
	}	
	fclose ( fp );
	printf("\nThread ended" );

	// free(client);

	close( sockfd);
	pthread_exit(NULL);
}


void create_null_file(struct client_addr * client)
{

    char create_file_name[256];
    // sprintf(create_file_name,"copy_%s",client->file_name);
   sprintf(create_file_name,"%s",client->file_name);

    int size;
    size=client->file_size;

	FILE *fp = fopen ( create_file_name  , "wb" );

	char Buffer [ BUFF] ;

	memset ( Buffer , 'a', BUFF);

	int i=0;
	while(size>BUFF){
		if(!fwrite (Buffer , sizeof (char), BUFF, fp)){
			cout<<"\nCant write into file";
		}	
		size-=BUFF;
		//cout<<i<<" "<<size<<"\n";//<<Buffer<<"\n";
		i++;
		fflush(fp);
	}
	char a='a';
	while(size--){
		fwrite(&a,sizeof(char),1,fp);
	//cout<<size<<"\n";
	}
	//close(sockfd);
	fclose(fp);
	cout<<"\nEmpty File created\n";
	// sleep(1);
}

void handler_download_command()
{		int n;
		//cout<<"\nEnter filename to download\n";
	    char *file_name = new char [256];
	    cin>>file_name;

	    int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	    struct sockaddr_in serv_addr;

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons( Tracker_Port );
		serv_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);

		if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  { 
	        printf("\nConnection Failed with Tracker\n"); 
	        pthread_exit(NULL);
	    } 

	    send(sockfd,"download_file",strlen("download_file"),0);
	    bool ack=false;
	    recv(sockfd , &ack ,sizeof(ack),0 );

	    send(sockfd,file_name,strlen(file_name),0);
	    ack=false;
	    recv(sockfd,&ack,sizeof(ack),0 );

	    recv(sockfd,&n,sizeof(n),0);
	    ack=true;
	    send(sockfd,&ack,sizeof(ack),0);

	    cout<<"\nack recieved "<<ack;

	    std::vector<struct client_addr *> *v=new vector<struct client_addr *>();
		for(int i=0;i<n;i++)
	    {
		    char  IP_to_connect[20];
		    memset ( IP_to_connect , '\0', 20);
		    int port;
		   
		    int file_size;

		    recv(sockfd, &IP_to_connect,sizeof(IP_to_connect),0);
		   	ack=true;
	    	send(sockfd,&ack,sizeof(ack),0);

		    recv(sockfd,&port,sizeof(port),0);
		   	ack=true;
	    	send(sockfd,&ack,sizeof(ack),0);

		    recv(sockfd,&file_size,sizeof(file_size),0);
		   	ack=true;
	    	send(sockfd,&ack,sizeof(ack),0);
		    struct client_addr *client1= new client_addr();

		    char * IP = new char[20];
		    strcpy(IP,IP_to_connect);

		    client1->IP=IP;
		    client1->PORT=port;
		    client1->file_name=file_name;
		    client1->start_point=i;
		    client1->interval=n;
		    client1->file_size=file_size;
	
			v->push_back(client1);   
			cout<<"\n"<<client1->IP<<" " <<client1->PORT<<" " <<client1->start_point<<" "<<client1->interval<<" "<<client1->file_size;	
	    }

	   	for(int i=0;i<v->size();i++)
	   	{
	   		cout<<"\n"<<v->at(i)->IP<<" " <<v->at(i)->PORT<<" " <<v->at(i)->start_point<<" "<<v->at(i)->interval;
	   	}
	   	v->at(0)->first_client=true;

	   	struct mutex_struct * _mutex_struct=new mutex_struct();
	   	_mutex_struct->no_of_client=n;
	   	_mutex_struct->mutex=true;
		cout<<"\nno of clients remain "<<_mutex_struct->no_of_client<<"\n";


	   	map_mutex.insert({file_name,_mutex_struct});
	   	map_downloading.insert({file_name,v});
		 create_null_file(v->at(0));

		pthread_t tid[n];

		for(int i=0;i<n;i++)
		{
			cout<<"\n flow will be passed to thread";
			if( pthread_create(&tid[i],NULL,&download_file,v->at(i)) !=0 ){
				printf("Failed to create server thread\n");
				//pthread_exit(1);		
			}
			// pthread_join(tid[i], NULL);
		}	
		for(int i=0;i<n;i++)
		{
			pthread_detach(tid[i]);
			// cout<<"\n thread Ended "<<i;
		}		
		close(sockfd);
}


void handler_create_user()
{
	//cout<<"\nEnter uid and password\n";
    // char * uid = new char [256];
	char * password = new char[256];
	cin>>uid;
	cin>>password;

    int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( Tracker_Port );
	serv_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  { 
        printf("\nConnection Failed with Tracker\n"); 
        pthread_exit(NULL);
    } 
    send(sockfd,"create_user",strlen("create_user"),0);
    bool ack=false;
    recv(sockfd , &ack ,sizeof(ack),0 );
	
	send(sockfd,uid,strlen(uid),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,password ,strlen(password),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,&PORT,sizeof(PORT),0);
	recv(sockfd,&ack,sizeof(ack),0);

	cout<<"\nAccount created";
	close(sockfd);
}

void handle_login()
{
	//cout<<"\nEnter uid and password to login\n";
    // char * uid = new char [256];
	char * password = new char[256];
	cin>>uid;
	cin>>password;
 	
 	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( Tracker_Port );
	serv_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  { 
        printf("\nConnection Failed with Tracker\n"); 
        pthread_exit(NULL);
    } 
	send(sockfd,"login",strlen("login"),0);
    bool ack=false;
    recv(sockfd , &ack ,sizeof(ack),0 );    

	send(sockfd,uid,strlen(uid),0);
	recv(sockfd,&ack,sizeof(ack),0);

	send(sockfd,password ,strlen(password),0);
	recv(sockfd,&ack,sizeof(ack),0);    

	send(sockfd,&PORT,sizeof(PORT),0);
	recv(sockfd,&ack,sizeof(ack),0);
	recv(sockfd,&ack,sizeof(ack),0);

	if(ack==true){
		cout<<"\nLog in successful";
	}
	else{
		cout<<"\nPassword Incorrect";
	}
	close(sockfd);
}

void handle_logout()
{
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in serv_addr;


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( Tracker_Port );
	serv_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  { 
        printf("\nConnection Failed with Tracker\n"); 
        pthread_exit(NULL);
    } 

    send(sockfd,"logout",strlen("logout"),0);
    bool ack=false;
    recv(sockfd , &ack ,sizeof(ack),0 );

    send(sockfd,uid,strlen(uid),0);
    recv(sockfd , &ack ,sizeof(ack),0 );

    strcpy(uid ,"");

    cout<<"\n Logout successful";
    close(sockfd);
}

void * client(void * argv)
{
	while(!mutex_server_start){
		//cout<<"\nwaiting for server part to start ";
	}

	while(1){
		string s;
		cout<<"\nEnter command\t";
		cin>>s;

		if(s=="login"){
			if(strcmp(uid,"")){
				cout<<"\nAlready Login:-"<<uid;continue;
			}			
			handle_login();
		}
		if(s=="create_user"){
			handler_create_user();
		}
		if(!strcmp(uid,"")){
			cout<<"\nPlease login";continue;
		}
		if(s=="upload_file"){
			handler_upload_command();
		}
		if(s=="download_file"){
			handler_download_command();
		}
		if(s=="logout"){
			handle_logout();
		}
	}
}

int main(int argc, char **argv)
{
	if(argc<4)
	{
		cout<<"\nDetails missing";
		return 0;
	}
	strcpy(SERVER_IP,argv[1]);
	PORT = atoi (argv[2]);

	FILE *fp = fopen ( argv[3]  , "rb+" );	

	char buffer[256];
	memset(buffer,'\0',256);
	fread( buffer,sizeof(char),256,fp);

	char* token = strtok(buffer, "\n"); 
	strcpy(Tracker_IPAdd,token);
	token=strtok(NULL,"\n");
	Tracker_Port=atoi(token);

	cout<<SERVER_IP<<" "<<PORT<<" "<<Tracker_IPAdd<<" "<<Tracker_Port;

	pthread_t tid_client,tid_server;

	// cout<<"\nEnter tracker IP and PORT";
	// cin>>Tracker_IPAdd>>Tracker_Port;

	strcpy(uid,"");

	if( pthread_create(&tid_server,NULL,&server,NULL) !=0 ){
		printf("Failed to create server thread\n");return -1;		
	}

	if( pthread_create(&tid_client,NULL,&client,NULL) !=0 ){
		printf("Failed to create client thread\n");return -1;
	}

	pthread_join(tid_server, NULL);

	return 0;				
}