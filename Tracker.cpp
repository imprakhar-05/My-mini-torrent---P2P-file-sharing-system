#include<bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#define servrout cout<<"\nserver: "
#define BUFF_SIZE 2048
using namespace std;


char SERVER_IP[20];
char Tracker_IPAdd[20];
int Tracker_Port, PORT;
struct compare_string{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};

bool mutex_server_start=false;


struct client_addr{
	char *IP;
	int *PORT; 
	int *size;
	char *uid;
};

struct struct_uid {
	char uid[256];
	char password[256];
	char IP[32];
	int port;
	bool active;
};

map<char *,struct struct_uid *,compare_string> map_uid;

struct struct_upload
{
	struct sockaddr_in client_addr;
	int sockfd;
};

map<char *,vector<client_addr* >*, compare_string > map_shared_file;



struct struct_group{
	char * gid;
	vector<char *> v;
};

void * client(void * argv)
{
	while(!mutex_server_start){
		//cout<<"\nwaiting for server part to start ";
	}

	while(1)
	{

		cout<<"\nEnter file name to share\n";
	    char *file_name = new char [256];
	   	cin>>file_name;
	   	cout<<"\nEnter file size\n";
	    int *size=new int[1];
	    cin>>*size;

	   	cout<<"\nnumber of peer have this\n";
	    int n;
	    cin>>n;

	    vector<struct client_addr*> * v=new vector<struct client_addr*>();

  		for(int i=0;i<n;i++)
	    {
		    char *IP_to_connect=new char[20];
		    //char *IP_to_connect2=new char[20];
		    //cin>>file_name;
		    int *port=new int[1];
		    //int *port2=new int[1];
		    cout<<"\nEnter IP and PORT\n";
		    cin>>IP_to_connect;
		    cin>>*port;

		    struct client_addr *client1= new client_addr();

		    client1->IP=IP_to_connect;
		    client1->PORT=port;
		    client1->size=size;
			v->push_back(client1);   	
	    }	    
	    map_shared_file.insert({file_name,v});

	    auto itr=map_shared_file.find(file_name);

		if(map_shared_file.find(file_name)!=map_shared_file.end()){
		vector<struct client_addr *>  *v=itr->second;
		sleep(2);
		cout<< "\n size  "<< v->size()<<" ";
		//sleep(2);
		int n=v->size();
		}

    map<char *, vector<client_addr* >* >::iterator itre;

    for (itre = map_shared_file.begin(); itre != map_shared_file.end(); ++itre) { 
        cout <<"\n" << itre->first ;
        vector<struct client_addr*> * v=itre->second;
        for(int i=0;i<v->size();i++) {
        	cout<<"\n"<<v->at(i)->IP<<"\t"<<*(v->at(i)->PORT)<<"\t"<<*(v->at(i)->size);
        }
    } 
	}
}


void serve_download_command(int sockfd)
{
	cout<<"\nin the serve_download_command function";
	bool ack=true;
	send(sockfd,&ack,sizeof(ack),0);

	char file_name[256];
	memset ( file_name , '\0', 256);
	recv(sockfd,&file_name,sizeof(file_name),0);

	servrout;cout<<"\n"<<file_name;
	ack=true;
	send(sockfd,&ack,sizeof(ack),0);

	auto itr=map_shared_file.find(file_name);

	if(map_shared_file.find(file_name)!=map_shared_file.end())
	{
		vector<struct client_addr *>  *v=itr->second;
		sleep(2);
		cout<< "\n size  "<< v->size()<<" ";
		//sleep(2);

		int n=v->size();

		int count=0;
		for(int i=0;i<n;i++){
			auto itr = map_uid.find(v->at(i)->uid);
			struct struct_uid * _struct_uid = itr->second;
			if(_struct_uid->active==true)
				count++;
		}

		send(sockfd,&count,sizeof(count),0);
		ack=false;
		recv(sockfd,&ack,sizeof(ack),0);

		for(int i=0;i<n;i++){	
			auto itr = map_uid.find(v->at(i)->uid);
			struct struct_uid * _struct_uid = itr->second;			
			if(_struct_uid->active==true){
				sleep(1);
				cout<<"\n";

				char * ip=_struct_uid->IP;
				int port = _struct_uid->port;
				int file_size=*(v->at(i)->size);

				send(sockfd,ip,strlen(ip),0);
				ack=false;
				recv(sockfd,&ack,sizeof(ack),0);			

				send(sockfd,&port,sizeof(port),0);
				ack=false;
				recv(sockfd,&ack,sizeof(ack),0);				

				send(sockfd,&file_size,sizeof(file_size),0);
				ack=false;
				recv(sockfd,&ack,sizeof(ack),0);	

				cout<< ip <<" " <<port<<" " <<*(v->at(i)->size)<<" "<<strlen(ip)<<" "<<v->at(i)->uid;
			}
		}	
	}
	else{
		exit(0);
	}
}

void serve_upload_command(void * argv)
{
	struct struct_upload * temp=(struct struct_upload * )argv;
	bool ack=false;
	send(temp->sockfd,&ack,sizeof(ack),0);

	char recieve_file_name[256];
	memset ( recieve_file_name , '\0', 256);
	int file_size;

    char *IP_to_connect=new char[20];
    int *port=new int[1];
    int *size=new int[1];
	
	recv(temp->sockfd,&recieve_file_name,sizeof(recieve_file_name),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

	recv(temp->sockfd,&file_size,sizeof(file_size),0);	
	send(temp->sockfd,&ack,sizeof(ack),0);

	recv(temp->sockfd,port,sizeof(int),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

	char uid[256];
	memset(uid,'\0',256);
	recv(temp->sockfd,&uid,sizeof(uid),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

	char * UID = new char[256];
	strcpy(UID,uid);

    strcpy(IP_to_connect,inet_ntoa(temp->client_addr.sin_addr));
    //*port=ntohs(temp->client_addr.sin_port);
    *size=file_size;

    struct client_addr *client1= new client_addr();

    client1->IP=IP_to_connect;
    client1->PORT=port;
    client1->size=size;
    client1->uid=UID;

    char *file_name = new char [256];
    strcpy(file_name,recieve_file_name);

    cout<<"\nUpload File "<< file_name;


    if(map_shared_file.find(file_name)!=map_shared_file.end()){
		auto itr=map_shared_file.find(file_name);
    	vector<struct client_addr*> * v=itr->second;
    	v->push_back(client1);
    }
    else{
		vector<struct client_addr*> * v=new vector<struct client_addr*>();
    	cout<<"\n"<<"new Vecot created of size "<<v->size();
    	v->push_back(client1);
    	map_shared_file.insert({file_name,v});
    }

    map<char *, vector<client_addr* >* >::iterator itr;

    for (itr = map_shared_file.begin(); itr != map_shared_file.end(); ++itr) { 
        cout <<"\n" << itr->first ;
        vector<struct client_addr*> * v=itr->second;

        for(int i=0;i<v->size();i++) {
        	cout<<"\n"<<v->at(i)->IP<<"\t"<<*(v->at(i)->PORT)<<"\t"<<*(v->at(i)->size);
        }
    } 

}

void serve_create_user(void * argv)
{
	struct struct_upload * temp=(struct struct_upload * )argv;
	bool ack=false;
	send(temp->sockfd,&ack,sizeof(ack),0);

	char uid[256];
	char password[256];
	memset(uid,'\0',256);
	memset(password,'\0',256);

	recv(temp->sockfd,&uid,sizeof(uid),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

	recv(temp->sockfd,&password,sizeof(password),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

	struct struct_uid * UID = new struct_uid();

	strcpy(UID->uid,uid);
	strcpy(UID->password,password);
	strcpy(UID->IP,inet_ntoa(temp->client_addr.sin_addr));

	int port;
	recv(temp->sockfd,&port,sizeof(port),0);
	send(temp->sockfd,&ack,sizeof(ack),0);
	UID->port=port;
	UID->active=true;

	char * uid_new = new char[256];
	strcpy(uid_new,uid);

	map_uid.insert({uid_new,UID});

    for (auto itr = map_uid.begin(); itr != map_uid.end(); ++itr) { 
        cout <<"\n" << itr->first ;
    } 	

	cout<<"\nclient account created";
}

void serve_login(void * argv)
{
	struct struct_upload * temp=(struct struct_upload * )argv;
	bool ack=false;
	send(temp->sockfd,&ack,sizeof(ack),0);

	char uid[256];
	char password[256];
	memset(uid,'\0',256);
	memset(password,'\0',256);

	recv(temp->sockfd,&uid,sizeof(uid),0);
	send(temp->sockfd,&ack,sizeof(ack),0);
	cout<<"\nuid "<<uid;
	recv(temp->sockfd,&password,sizeof(password),0);
	send(temp->sockfd,&ack,sizeof(ack),0);
	cout<<"\npassword "<<password;
	int port;
	recv(temp->sockfd,&port,sizeof(port),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

    for (auto itr = map_uid.begin(); itr != map_uid.end(); ++itr) { 
        cout<<"\n"<< itr->first ;
        struct struct_uid * UID= itr->second;
        cout<<"\n"<<UID->uid<<" "<<UID->password<<" "<<UID->password;
    } 

	auto itr = map_uid.find(uid);
	struct struct_uid * UID= itr->second;
	cout<<"\n"<<UID->uid<<" "<<UID->password<<" "<<UID->IP;

	if(!strcmp(password,UID->password))
	{
		ack=true;
		send(temp->sockfd,&ack,sizeof(ack),0);
		strcpy(UID->IP,inet_ntoa(temp->client_addr.sin_addr));
		UID->port=port;
		UID->active=true;
	}
	else{
		ack=false;
		send(temp->sockfd,&ack,sizeof(ack),0);
	}
}

void serve_logout(void * argv)
{
	struct struct_upload * temp=(struct struct_upload * )argv;
	bool ack=false;
	send(temp->sockfd,&ack,sizeof(ack),0);

	char uid[256];
	memset(uid,'\0',256);

	recv(temp->sockfd,&uid,sizeof(uid),0);
	send(temp->sockfd,&ack,sizeof(ack),0);

	auto itr = map_uid.find(uid);
	struct struct_uid * UID= itr->second;

	UID->active=false;
}


void *serveRequest(void *arg)
{
	struct struct_upload * client_addr=(struct struct_upload * )arg;

	char command_type[128];
	memset ( command_type , '\0', 128);


	recv(client_addr->sockfd,&command_type,sizeof(command_type),0);
	//sleep(2);
	servrout;cout<<"command:-"<<command_type;

	if(!strcmp(command_type,"download_file"))
	{
		serve_download_command(client_addr->sockfd);
	}
	else if(!strcmp(command_type,"upload_file"))
	{
		serve_upload_command(client_addr);
	}
	else if (!strcmp(command_type,"create_user"))
	{
		serve_create_user(client_addr);
	}
	else if(!strcmp(command_type,"login"))
	{
		serve_login(client_addr);
	}
	else if(!strcmp(command_type,"logout"))
	{
		serve_logout(client_addr);
	}
	else
	{
		cout<<"\ncommand not found";
	}


}


void * server(void * argv)
{
	cout<<"ram";

	//init();

	int server_fd = socket (AF_INET, SOCK_STREAM, 0);
	
	pthread_t tid;

	// cout<<"\nEnter port for process\n";
	// cin>>PORT;

	struct sockaddr_in   server_addr,client_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( Tracker_Port);
	server_addr.sin_addr.s_addr=inet_addr(Tracker_IPAdd);


	cout<<"\nport no - "<<ntohs(server_addr.sin_port)<<"\n";

	int addrlen = sizeof(sockaddr);

	if(bind (server_fd  , (struct sockaddr *)&server_addr , sizeof ( server_addr ) ) !=0)
	{
		cout<<"\nError in Binding Socket\n";
		exit(1);
	}
	mutex_server_start=true;
	listen (server_fd, 3);
	cout<<"\nListening";
	
	while(1)
	{

		int sockfd = accept ( server_fd , (struct sockaddr *)&client_addr , (socklen_t*)&addrlen);
		cout<<"\nport no - "<<client_addr.sin_port<<"\n";
		cout<<"\nConnection Established";
		struct struct_upload * temp = new struct_upload();

		temp->sockfd=sockfd;
		temp->client_addr=client_addr;

		if( pthread_create(&tid,NULL,&serveRequest,temp) !=0 )
			printf("Failed to create thread\n");
		pthread_join(tid,NULL);
		cout<<"\nthread ended";
	}


	close( server_fd);	
}

int main(int argc, char **argv)
{
	pthread_t tid_client,tid_server;

	if(argc<2)
	{
		cout<<"\nDetails missing";
		return 0;
	}


	FILE *fp = fopen ( argv[1]  , "rb+" );	

	char buffer[256];
	memset(buffer,'\0',256);
	fread( buffer,sizeof(char),256,fp);

	char* token = strtok(buffer, "\n"); 
	strcpy(Tracker_IPAdd,token);
	token=strtok(NULL,"\n");
	Tracker_Port=atoi(token);



	if( pthread_create(&tid_server,NULL,&server,NULL) !=0 ){
		printf("Failed to create server thread\n");return -1;		
	}

	if( pthread_create(&tid_client,NULL,&client,NULL) !=0 ){
		printf("Failed to create client thread\n");return -1;
	}

	pthread_join(tid_server, NULL);

	return 0;				
}