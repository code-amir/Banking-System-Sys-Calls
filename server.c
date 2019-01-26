#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define MAX 20

char cur_user[20];

struct database
{
	char uname[MAX];
	char pass[MAX];
	int acc_id;
	int balance;
};

struct jointdatabase
{
	char uname1[MAX];
	char pass1[MAX];
	char uname2[MAX];
	char pass2[MAX];
	int acc_id;
	int balance;
};


void mainMenu(int );
void displayMenu(int, struct database);
void admin_functions(int);
void deleteAccount(int);
void deleteJointAccount(int);
void jointDisplayMenu(int, struct jointdatabase,struct flock,int );

void findDetails(struct database db_user)
{
	struct database db;
	int fd1 = open("Accounts.dat", O_CREAT | O_RDWR , 0666);

	while(read(fd1, (char *)&db, sizeof (struct database)))
	{
		
		if(!strcmp(db_user.uname, db.uname))
		{
			db.balance = db_user.balance;
			strcpy(db.pass,db_user.pass);
			lseek(fd1,-sizeof(struct database),SEEK_CUR);
			write(fd1, (char *)&db,sizeof(struct database));
			break;
		}
	}
}

void jointFindDetails(struct jointdatabase db_user)
{
	struct jointdatabase db;
	
	int fd1 = open("Joint_Account.dat", O_CREAT | O_RDWR , 0666);

	while(read(fd1, (char *)&db, sizeof (struct jointdatabase)))
	{
		
		if(!strcmp(db_user.uname1, db.uname1) && !strcmp(db_user.uname2, db.uname2))
		{
			

			db.balance = db_user.balance;
			strcpy(db.pass1,db_user.pass1);
			strcpy(db.pass2,db_user.pass2);
			lseek(fd1,-sizeof(struct jointdatabase),SEEK_CUR);
			write(fd1, (char *)&db,sizeof(struct jointdatabase));
			
			break;
		}
	}
}

int jointAccountExists(char uname1[],char uname2[])
{
	struct jointdatabase db;

	int fd1 = open("Joint_Account.dat", O_CREAT | O_APPEND | O_RDWR, 0666);
    
	while(read(fd1, (char *)&db, sizeof(struct jointdatabase)))
	{

		if((!strcmp(db.uname1, uname1)) && (!strcmp(db.uname2,uname2)) || (!strcmp(db.uname1, uname2)) && (!strcmp(db.uname2,uname1)))
			{
				close(fd1);
				return 1;
			}
	}
	close(fd1);
	return 0;	
}

int checkAccountExists(char uname[])
{
	struct database db;

	int fd1 = open("Accounts.dat", O_CREAT | O_APPEND | O_RDWR , 0666);
	
	while(read(fd1, (char *)&db, sizeof(struct database)))
	{

		if(!strcmp(db.uname, uname))
			{
				close(fd1);
				return 1;
			}
	}
	close(fd1);
	return 0;	
	
}

void addjointAccount(char uname1[], char pass1[], char uname2[], char pass2[], int sd)
{
	srand(time(0));

	int fd1 = open("Joint_Account.dat", O_CREAT | O_APPEND | O_RDWR , 0666);

	struct jointdatabase db;

	strcpy(db.uname1, uname1);
	strcpy(db.pass1, pass1);
	strcpy(db.uname2, uname2);
	strcpy(db.pass2, pass2);

	db.balance=0;

	db.acc_id=(rand()%1001)+5000;

	write(fd1, (char *)&db, sizeof(struct jointdatabase));

	write(sd, "Account Created", 15);
	
	close(fd1);
}

void addAccount(char uname[], char pass[],int sd)
{
	srand(time(0));

	int fd1 = open("Accounts.dat", O_CREAT | O_APPEND | O_RDWR , 0666);

	struct database db;

	strcpy(db.uname, uname);
	strcpy(db.pass, pass);

	db.balance=0;

	db.acc_id=(rand()%1001)+3000;

	write(fd1, (char *)&db, sizeof(struct database));

	write(sd, "Account Created", 15);
	
	close(fd1);
}

void newJointAccount(int sd, int flag)
{
	char uname1[20], pass1[20], uname2[20], pass2[20];

	read(sd, uname1, sizeof(uname1));
	read(sd, pass1, sizeof(pass1));
	read(sd, uname2, sizeof(uname2));
	read(sd, pass2, sizeof(pass2));

	if(!jointAccountExists(uname1,uname2))
	{
		addjointAccount(uname1, pass1, uname2, pass2, sd);
		if(flag==1)
			mainMenu(sd);
		else
			admin_functions(sd);
	}
	else
	{
		write(sd, "Username already exist\n", 23);
		if(flag ==1)
			mainMenu(sd);
		else
			admin_functions(sd);
	}


}

void signUp(int sd,int flag)
{
	char uname[20], pass[20];

	read(sd, uname, sizeof(uname));
	read(sd, pass, sizeof(pass));

	if(!checkAccountExists(uname))
	{
		addAccount(uname, pass, sd);
		if(flag ==1)
			mainMenu(sd);
		else
			admin_functions(sd);
	}
	else
	{
		write(sd, "Username already exist\n", 23);
		if(flag ==1)
			mainMenu(sd);
		else
			admin_functions(sd);
	}	

}


void login(int sd)
{
	
	char uname[20], pass[20];

	read(sd, uname, sizeof(uname));
	read(sd, pass, sizeof(pass));

	struct database db;
	int flag=1;

	int fd1 = open("Accounts.dat", O_CREAT | O_APPEND | O_RDWR , 0666);
	
	while(read(fd1, (char *)&db, sizeof(struct database)))
	{

		if(!strcmp(db.uname, uname) && !strcmp(db.pass, pass))
			{
				write(sd, &flag, sizeof(flag));
				write(sd, "Login Successful", 16);
				displayMenu(sd,db);
				close(fd1);
			
				return;		
			}
	}

	close(fd1);
	flag=0;
	write(sd, &flag, sizeof(flag));
	write(sd, "Invalid Credentials", 19);
	mainMenu(sd);	
}

void jointAccountLogin(int sd,int fd)
{
	char uname1[20], uname2[20], pass[20];
	struct flock lock;

	read(sd, uname1, sizeof(uname1));
	read(sd, uname2, sizeof(uname2));
	read(sd, pass, sizeof(pass));
	strcpy(cur_user,uname1);

	struct jointdatabase db;
	int flag=1;
	

	int fd1 = open("Joint_Account.dat", O_CREAT | O_APPEND | O_RDWR , 0666);
	
	while(read(fd1, (char *)&db, sizeof(struct jointdatabase)))
	{

		if((!strcmp(db.uname1, uname1) && !strcmp(db.pass1,pass) && !strcmp(db.uname2, uname2)) || (!strcmp(db.uname1, uname2) && !strcmp(db.uname2, uname1) && !strcmp(db.pass2,pass)))
			{
				lseek(fd1,-sizeof(struct jointdatabase),SEEK_CUR);
				lock.l_type=F_WRLCK;
				lock.l_whence=SEEK_CUR;
				lock.l_start=0;
				lock.l_len=sizeof(struct jointdatabase);

			/*Locking*/	
				fcntl(fd1,F_SETLKW,&lock);
				
				write(sd, &flag, sizeof(flag));
				write(sd, "Login Successful", 16);
				jointDisplayMenu(sd,db,lock,fd1);
				
				

				close(fd1);
			
				return;		
			}
	}

	close(fd1);
	flag=0;
	write(sd, &flag, sizeof(flag));
	write(sd, "Invalid Credentials", 19);
	mainMenu(sd);

}

void viewDetails(int sd, struct database db)
{
	write(sd,db.uname, sizeof(db.uname));
	write(sd,&db.acc_id, sizeof(db.acc_id));
	write(sd,&db.balance, sizeof(db.balance));
	displayMenu(sd,db);
}

void viewJointDetails(int sd, struct jointdatabase db,struct flock lock,int fd)
{
	write(sd,db.uname1, sizeof(db.uname1));
	write(sd,db.uname2, sizeof(db.uname2));
	write(sd,&db.acc_id, sizeof(db.acc_id));
	write(sd,&db.balance, sizeof(db.balance));
	jointDisplayMenu(sd,db,lock,fd);
}

void deposit(int sd, struct database db)
{
	int amount;
	
	read(sd, &amount, sizeof(amount));

	db.balance = db.balance + amount;
    findDetails(db);
	write(sd, &db.balance, sizeof(db.balance));
	
	displayMenu(sd,db);
}

void jointDeposit(int sd, struct jointdatabase db,struct flock lock,int fd)
{
	int amount;
	
	
	read(sd, &amount, sizeof(amount));

    
	db.balance = db.balance + amount;
	
    jointFindDetails(db);
	write(sd, &db.balance, sizeof(db.balance));
	
	jointDisplayMenu(sd,db,lock,fd);
}


void jointWithdraw(int sd, struct jointdatabase db,struct flock lock, int fd)
{
	int amount, flag=0;
	
	read(sd, &amount, sizeof(amount));

	if(amount>=db.balance)
	{
		write(sd,&flag,sizeof(flag));
		write(sd, &db.balance, sizeof(db.balance));
	}
	else
	{
		flag=1;
		db.balance = db.balance - amount;
		jointFindDetails(db);
		write(sd,&flag,sizeof(flag));
		write(sd, &db.balance, sizeof(db.balance));
	}


	jointDisplayMenu(sd,db,lock,fd);
}

void withdraw(int sd, struct database db)
{
	int amount, flag=0;
	
	read(sd, &amount, sizeof(amount));

	if(amount>=db.balance)
	{
		write(sd,&flag,sizeof(flag));
		write(sd, &db.balance, sizeof(db.balance));
	}
	else
	{
		flag=1;
		db.balance = db.balance - amount;
		findDetails(db);
		write(sd,&flag,sizeof(flag));
		write(sd, &db.balance, sizeof(db.balance));
	}


	displayMenu(sd,db);
}
void jointPassChange(int sd, struct jointdatabase db,struct flock lock)
{
	char pass[MAX];
	read(sd,pass,sizeof(pass));
    if(!strcmp(cur_user,db.uname1))
	   strcpy(db.pass1,pass);
	else
		strcpy(db.pass2,pass);

	jointFindDetails(db);
	mainMenu(sd);

}

void passChange(int sd, struct database db)
{
	char pass[MAX];
	read(sd,pass,sizeof(pass));
	strcpy(db.pass,pass);
	findDetails(db);
	mainMenu(sd);

}

void displayMenu(int sd, struct database db)
{
	int choice;
	read(sd, &choice, sizeof(choice));
	switch(choice)
	{
		case 1: viewDetails(sd,db);
				break;
		case 2: deposit(sd,db);
				break;
		case 3: withdraw(sd,db);
				break;
		case 4: passChange(sd,db);
				break;
		case 5: mainMenu(sd);
				break;

	}

}

void jointDisplayMenu(int sd,struct jointdatabase db,struct flock lock,int fd)
{
	int choice;
	read(sd, &choice, sizeof(choice));
	switch(choice)
	{
		case 1: viewJointDetails(sd,db,lock,fd);
				break;
		case 2: jointDeposit(sd,db,lock,fd);
				break;
		case 3: jointWithdraw(sd,db,lock,fd);
				break;
		case 4: jointPassChange(sd,db,lock);
				break;
		case 5: lock.l_type=F_UNLCK;
				fcntl(fd,F_SETLK,&lock); 
				mainMenu(sd);
				break;

	}
}

void admin_functions(int sd)
{
	int choice;
	char uname[20], pass[20], uname1[20], uname2[20];
	read(sd, &choice, sizeof(choice));
	switch(choice)
	{
		case 1:signUp(sd,0);
			break;
		case 2:deleteAccount(sd);
			break;
		case 3:read(sd, uname, sizeof(uname));
				if(checkAccountExists(uname))
			{
				write(sd, "Account Found\0", 14);
			}
			else
			{
				write(sd, "Account does not exist with the given Username", 46);
			}
			admin_functions(sd);
			break;
		case 4:newJointAccount(sd,0);
			break;
		case 5:deleteJointAccount(sd);
		     	break;
		case 6:read(sd, uname1, sizeof(uname1));
				read(sd, uname2, sizeof(uname2));
				if(jointAccountExists(uname1,uname2))
			{
				write(sd, "Account Found\0", 14);
			}
			else
			{
				write(sd, "Account does not exist with the given Username", 46);
			}
			admin_functions(sd);
			break;
		default:mainMenu(sd);
	}
}

void deleteAccount(int sd)
{
	char uname[20];
	int fd1, fd2;

	fd1 = open("Accounts.dat", O_CREAT | O_RDWR , 0666);
	fd2 = open("Accounts2.dat", O_CREAT | O_RDWR, 0666);

	read(sd, uname, sizeof(uname));

	if(!checkAccountExists(uname))
	{
		write(sd, "Account does not exist", 22);
	}
	else
	{	

		struct database db[1000], db1;

		int i=0;
		lseek(fd1, 0, SEEK_SET);
		while(read(fd1, (char *)&db[i++], sizeof(struct database)));
		

		for(int j=0;j<i-1;j++)
		{
			if(!strcmp(db[j].uname, uname))
			{
				continue;				
			}
			
			else
			{
				write(fd2,(char *)&db[j],sizeof(struct database));
				
			}
		}

		lseek(fd2, 0, SEEK_SET);
		fd1=open("Accounts.dat",O_TRUNC|O_RDWR,0666);
		lseek(fd1, 0, SEEK_SET);
		
		while(read(fd2, (char *)&db1, sizeof(struct database)))
		{
			write(fd1,(char *)&db1,sizeof(struct database));
		}


		lseek(fd1,0, SEEK_SET);

		write(sd,"Account deleted Successfully", 28);
		open("Accounts2.dat",O_TRUNC|O_RDWR,0666);
	}

	close(fd1);
	close(fd2);
	admin_functions(sd);
}

void deleteJointAccount(int sd)
{
	char uname1[20], uname2[20];
	int fd1, fd2;

	fd1 = open("Joint_Account.dat", O_CREAT | O_RDWR , 0666);
	fd2 = open("Joint_Account2.dat", O_CREAT | O_RDWR, 0666);

	read(sd, uname1, sizeof(uname1));
	read(sd, uname2, sizeof(uname2));
    
	if(!jointAccountExists(uname1,uname2))
	{
		
		write(sd, "Account does not exist", 22);
	}
	else
	{	
		
		struct jointdatabase db[1000], db1;

		int i=0;
		lseek(fd1, 0, SEEK_SET);
		while(read(fd1, (char *)&db[i++], sizeof(struct jointdatabase)));
		

		for(int j=0;j<i-1;j++)
		{
			
			if((!strcmp(db[j].uname1, uname1)) && (!strcmp(db[j].uname2,uname2)) || (!strcmp(db[j].uname1, uname2)) && (!strcmp(db[j].uname2,uname1)))
			{
				continue;				
			}
			
			else
			{
				write(fd2,(char *)&db[j],sizeof(struct jointdatabase));
				
			}
		}

		lseek(fd2, 0, SEEK_SET);
		fd1=open("Joint_Account.dat",O_TRUNC|O_RDWR,0666);
		lseek(fd1, 0, SEEK_SET);
		
		while(read(fd2, (char *)&db1, sizeof(struct jointdatabase)))
		{
			write(fd1,(char *)&db1,sizeof(struct jointdatabase));
		}


		lseek(fd1,0, SEEK_SET);
		

	
		write(sd,"Account deleted Successfully", 28);
		open("Joint_Account2.dat",O_TRUNC|O_RDWR,0666);
	}

	close(fd1);
	close(fd2);
	admin_functions(sd);

}

void adminLogin(int sd)
{
	
	char uname[20], pass[20];
	int flag=1;
	read(sd, uname, sizeof(uname));
	read(sd, pass, sizeof(pass));
	if(!strcmp(uname,"admin") && !strcmp(pass,"password"))
	{
			
		write(sd, &flag, sizeof(flag));
		write(sd,"Welcome Admin", 13);
		admin_functions(sd);
	}
	else
	{	
		flag=0;
		write(sd, &flag, sizeof(flag));
		write(sd,"Invalid Credentials", 20);
		
		
		mainMenu(sd);
	}

}

void mainMenu(int sd)
{
	int choice,fd;
	read(sd, &choice, sizeof(choice));
	switch(choice)
	{
		case 1: signUp(sd,1);
				break;
		case 2: login(sd);
				break;
		case 3: adminLogin(sd);
				break;
		case 4: newJointAccount(sd,1);
				break;
		case 5: jointAccountLogin(sd,fd);
				break;
		case 6: exit(0);
				break;

	}
}

int main(int argc,char *argv[])
{

	struct sockaddr_in server, client;
	int sd, client_addr_len, nsd;
	
	sd = socket(AF_INET, SOCK_STREAM,0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));

	if(bind(sd,(struct sockaddr *)(&server), sizeof(server))<0){
		printf("Error at bind\n");
	}

	listen(sd,10);
	client_addr_len = sizeof(client);

	while(1){
		nsd = accept(sd, (struct sockaddr *)&client, &client_addr_len);
		if(!fork()){

			close(sd);
			mainMenu(nsd);
			exit(0);

		}
		else
		{
			close(nsd);
		}

	}
		
	close(nsd);
	close(sd);
	return 0;
}


