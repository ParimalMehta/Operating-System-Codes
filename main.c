/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int pipefds[2];
int pipefds1[2];

int fd;
int flag;


int handleTokens(Cmd c)
{
    if (c->in == Tin)
    {       
        
      if(dup2(open(c->infile, O_RDONLY, 0660), 0)==-1)
      {
        printf("Error in processing input file\n");
        return 0;
      }
      
    }
    if (c->out == ToutErr)
    {     
      fflush(stdout);      
      if(dup2(open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC,0777), 1)==-1)
      {
        printf("Error in processing output file");
        return 0;
      }
      if(dup2(open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC,0777), 2)==-1)
      {
        printf("Error in processing error stream.");
        return 0;
      }
    }

    if(c->out == Tout)
    {
      fflush(stdout);
      dup2(creat(c->outfile, 0660), 1);
    }


    if(c->in == Tpipe)
    {    

      close(pipefds1[1]);
      dup2(pipefds1[0], 0);
      
    }

    if(c->out == Tpipe)
    {   
   
     close(pipefds[0]);
     dup2(pipefds[1],1);
              
    }       

    return 1;
}

static void prCmd(Cmd c)
{
  int i;  

  if ( c ) {
    printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin )
      printf("<(%s) ", c->infile);
    if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
	printf(">(%s) ", c->outfile);
	break;
      case Tapp:
	printf(">>(%s) ", c->outfile);
	break;
      case ToutErr:
	printf(">&(%s) ", c->outfile);
	break;
      case TappErr:
	printf(">>&(%s) ", c->outfile);
	break;
      case Tpipe:
	printf("| ");
	break;
      case TpipeErr:
	printf("|& ");
	break;
      default:
	fprintf(stderr, "Shouldn't get here\n");
	exit(-1);
      }

    if ( c->nargs > 1 ) {
      printf("[");
      for ( i = 1; c->args[i] != NULL; i++ )
	printf("%d:%s,", i, c->args[i]);
      printf("\b]");
    }
    putchar('\n');
    // this driver understands one command
    if ( !strcmp(c->args[0], "end") )
      exit(0);
  }
}

static void prPipe(Pipe p)
{
  int i = 0;
  Cmd c;

  if ( p == NULL )
    return;

  printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    printf("  Cmd #%d: ", ++i);
    prCmd(c);
  }
  printf("End pipe\n");
  prPipe(p->next);
}

void handle_cd_command(Cmd  c)
{

  char buf[1000];
if(c->next == NULL)
{

  if(c->args[1]==NULL)
  {
      chdir(getenv("HOME"));
     // printf("\n %s \n",getcwd(buf, sizeof(buf)));
  }
  else
  {
    int status = chdir(c->args[1]);
  //  printf("\n Working directory is => %s \n",getcwd(buf, sizeof(buf)));
    if(status != 0)
    {
      printf("Error in processing cd %d",status);
    }
  }
}
else
  {


    pid_t pid = fork();
    int status;
    if(pid == 0)
    {
      if(handleTokens(c))
    {
      if(c->args[1]==NULL)
      {
          chdir(getenv("HOME"));
        // printf("\n Working directory is => %s \n",getcwd(buf, sizeof(buf)));
      }
      else
      {
        int status = chdir(c->args[1]);
      //  printf("\n Working directory is => %s \n",getcwd(buf, sizeof(buf)));
        if(status != 0)
        {
          printf("Error in processing cd %d",status);
        }
      }
    }
    else{
      return;
    }
    
    }
    else
    {
      waitpid(pid,&status,0); 
    }
}

  return;

}


void handle_pwd_command(Cmd c)
{
  pid_t pid;
  int status;
	pid = fork();	
  if(handleTokens(c))
    {
	if(pid == 0) // child
	{	
    if(handleTokens(c))
    {
		char buf[1000];	
		
		if(getcwd(buf, sizeof(buf)) == NULL)
		{
			printf("pwd failed\n");
		}
		else
		{
			printf("%s\n", getcwd(buf, sizeof(buf)));
		}
		exit(0);
    }
    else{
      return;
    }
	}
	else
	{
		 waitpid(pid,&status,0); 		
	}
    }
    else{
      return;
    }
}


void handle_echo_command(Cmd c)
{
  if(handleTokens(c))
    {
  int i=0;
  int status;

  if(c->next == NULL)
  {    

    for(i=1;i<c->nargs;i++)
    {
      if(c->args[i][0]=='$')
      {     
        memmove(c->args[i], c->args[i]+1, strlen(c->args[i]));
        
      
        if(getenv(c->args[i])!=NULL)
        {
          printf("\nenv variables are => %s\n", getenv(c->args[i]));
        }
        else
        {
          printf("\n Environment variable not defined\n");
        }
      }
      else
      {
        printf("\n %s\n",c->args[i]);
      }   
    }
  }
  else
  {
    pid_t pid = fork();
    
    if(pid == 0)
    {
    if(handleTokens(c))
        {
      for(i=1;i<c->nargs;i++)
      {
        if(c->args[i][0]=='$')
        {     
          memmove(c->args[i], c->args[i]+1, strlen(c->args[i]));
         
        
          if(getenv(c->args[i])!=NULL)
          {
            printf("\nenv variables are => %s\n", getenv(c->args[i]));
          }
          else
          {
            printf("\n Environment variable not defined\n");
          }
        }
        else
        {
          printf("\n %s\n",c->args[i]);
        }   
      }
      }
      else{
        return;
      }
    }
    else
    {
      waitpid(pid,&status,0);
    }
  }
    }
    else
    {
      return;
    }
}

void handle_other_command(Cmd c)
{    
  int status;
  pid_t pid = fork();
  char *c_path = c->args[0];
  if(pid==0)
  {
    if(handleTokens(c))
    {
      if(execvp(c_path,c->args)==-1) 
      {
        printf("\n Command not found ");
      }      
    }
    else
    {
      return;
    }   
  }
  else
  {    
     waitpid(pid,&status,0);    
  }
}


void handle_setenv_command(Cmd c)
{
  int i=0;
  
  if(c->args[1] == NULL)
	{
  
		extern char **environ;
		while(environ[i] != NULL)
		{
			printf("%s\n",environ[i]);
			i++;
		}
		return;
	}
	else
	{		    
    if(c->args[2]==NULL)
    {
 
      if(setenv(c->args[1],"",1) == -1)
      {
        printf("Error in setting environment variable\n");
        return;
    }
    }

    else
    {
      
    	if(setenv(c->args[1],c->args[2],1) == -1)
      {
        printf("Error in setting environment variable\n");
        return;		
      }
	
		}
  }       
       
}

void handle_unsetenv_command(Cmd c)
{
   if(handleTokens(c))
        {
	if(c->args[1] == NULL)
	{
		printf("No env variable name for unsetenv.\n");
		return;
	}
	else
	{
	    if (unsetenv(c->args[1]) == -1)
	    {
	   	printf("Error in unsetenv command execution.\n");
	   	return;
	    }
 	}
        }
        else{
          return;
        }
}

void handle_where_command(Cmd c)
{
   if(handleTokens(c))
        {
  if(c->args[0]==NULL)
  {
    printf("Not found");
    return;
  }
  else
  {   
    
    if(strcmp(c->args[1],"cd") == 0)
    {
      printf(" %s : Built-in command \n", c->args[1]);
      return;
    }
    else if(strcmp(c->args[1],"echo") == 0)
    {
     printf(" %s : Built-in command \n", c->args[1]);
     return;
    }
    else if(strcmp(c->args[1],"logout") == 0)
    {
     printf(" %s : Built-in command \n", c->args[1]);
     return;
    }
    else if(strcmp(c->args[1],"pwd") == 0)
    {
      printf(" %s : Built-in command \n", c->args[1]);
      return;
    }
    else if(strcmp(c->args[1],"setenv") == 0)
    {
      printf(" %s : Built-in command \n", c->args[1]);
      return;
    }
    else if(strcmp(c->args[1],"unsetenv") == 0)
    {
     printf(" %s : Built-in command \n", c->args[1]);
     return;
    }
    else if(strcmp(c->args[1],"where") == 0)
    {
      printf(" %s : Built-in command \n", c->args[1]);
      return;
    }
    else if(strcmp(c->args[1],"nice") == 0)
    {
      printf(" %s : Built-in command \n", c->args[1]);
      return;
    }
    

    char* env_path = getenv("PATH");
	  char executible_file_path[1000]="";

    char* tokenized_path = strtok(env_path,":");
    while(tokenized_path != NULL)
    {
      strcpy(executible_file_path,tokenized_path);
      strcat(executible_file_path,"/");
      strcat(executible_file_path,c->args[1]);
      if(!access(executible_file_path,F_OK))
      {
          printf("%s\n",executible_file_path);
      }
      tokenized_path = strtok(NULL,":");
	  }
      
  }
        }
        else{
          return;
        }
  
}


void handle_nice_command(Cmd c)
{
	int priority;
	int flag = 0;		
	if(c->args[1]!=NULL)
	{
		priority = atoi(c->args[1]);		
		if(priority< -20 )
		{
			printf("Priority Out of range: ");
      return;
		}
		else if(priority> 19)
		{
			printf("Priority Out of range: ");
      return;
		}

    if(priority == 0 && strcmp(c->args[1],"0")){
			priority = 4;
			flag = 1;		
		}
	}
	else
	{
		printf("No arguments \n");
	}
	pid_t pid = fork();
	if(pid == 0){		

 if(handleTokens(c))
        {

		getpriority(PRIO_PROCESS, 0);
		setpriority(PRIO_PROCESS, 0, priority);		
		
			if(flag)
			{
        Pipe p;
        (p->head)->args[0] = c->args[1];
        (p->head)->args[1] = c->args[2];
        p->next = NULL;
        detectCommand(p); 
			}			
			else
			{
				 Pipe p;
        (p->head)->args[0] = c->args[2];
        (p->head)->args[1] = c->args[3];
        p->next = NULL;
        detectCommand(p); 
			}	
        }
        else{
          return;
        }	
	}
	else{		
    int status;
		waitpid(pid,&status,0);
	}
}

void create_pipe(int* pipefds)
{
  int pipe_object = pipe(pipefds);
  if(pipe_object==-1)
  {
    printf("Error in pipe creation");
  }
}


void detectCommand(Pipe p)
{
  int i = 0;
  Cmd c;
  create_pipe(pipefds);
  create_pipe(pipefds1);
  
  if ( p == NULL )
    return;

  for ( c = p->head; c != NULL; c = c->next ) 
  {         

    if(strcmp(c->args[0],"cd") == 0)
    {
      handle_cd_command(c);
    }
    else if(strcmp(c->args[0],"echo") == 0)
    {
      handle_echo_command(c);
    }
    else if(strcmp(c->args[0],"logout") == 0)
    {
      exit(0);
    }
    else if(strcmp(c->args[0],"pwd") == 0)
    {
      handle_pwd_command(c);
    }
    else if(strcmp(c->args[0],"setenv") == 0)
    {
      handle_setenv_command(c);
    }
    else if(strcmp(c->args[0],"unsetenv") == 0)
    {
      handle_unsetenv_command(c);
    }
    else if(strcmp(c->args[0],"where") == 0)
    {
      handle_where_command(c);
    }
     else if(strcmp(c->args[0],"nice") == 0)
    {
      handle_nice_command(c);
    }   
    else
    {     
      handle_other_command(c);     
    }         
    
    if(c->out == Tpipe)
    {
      create_pipe(pipefds1);
      int i=0;
      char directory[10000]="";      
      int aa = read(pipefds[0],directory,sizeof(directory));       
      write(pipefds1[1],directory,aa);
      close(pipefds1[1]);

    }    
   
   // create_pipe(pipefds);
  }
  detectCommand(p->next);
}


//Signal handling for keyboard interrupts
void signalHandling()
{
	signal (SIGQUIT, SIG_DFL);
	signal (SIGTERM, SIG_DFL);
	signal (SIGINT, SIG_DFL);
	signal (SIGKILL, SIG_DFL);
	signal (SIGHUP, SIG_DFL);
  signal (SIGTSTP, SIG_DFL);
}

int main(int argc, char *argv[])
{

  Pipe p;
   
  signalHandling();

  char hostname[500 + 1];
  gethostname(hostname, 500 + 1);

  while ( 1 ) {
    printf("%s%% ", hostname);
    p = parse();
  //  prPipe(p);
    detectCommand(p);  
    freePipe(p);
 }
}


/*........................ end of main.c ....................................*/
