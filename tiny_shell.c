/**
 *  ECSE 427 Assignment 1
 *  Ammar Rudani
 *  260804420
 *  
 */ 




#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>

//defining buffer, variable and array that will be used in the shell
#define BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"
char *history[100];
int history_count = 0;
#define HISTORY_LENGTH 100;



int shell_cd(char **args);
int shell_history(char * command);
int shell_exit(char **args);


//Shell Command key words
char *shellCommand[] = {
  "cd",
  "exit",
  "chdir",
};

//Shell functions associated with the key words
int (*shellFunctions[]) (char **) = {
  
  &shell_cd,
  &shell_exit,
  &shell_cd,

};

int shellCommandLength() {
  return sizeof(shellCommand) / sizeof(char *);
}

//cd function, called when the first argument in command line is cd or chdir, checks for a valid second input
// for which to change directories to. 
int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "error: must give argument after for \"cd\" \n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("error");
    }
  }
  return 1;
}

//exit shell function, added to as the last command to txt files to exit after last command
int shell_exit(char **args){ return 0; }


//history function used to keep track of all the commands made in the shell.
// array holds 100 command inputs, and if hsitory exceeds 100
// the history array deletes the oldest command in the array and moves every other command
// down by one index to make room for new command 
int shell_history(char *currCommand) {
  if(history_count < 100) {

    history[history_count] = strdup(currCommand);
    history_count++;
  }
  else {

    free(history[0]);

    for(int i = 1; i < 100; i++) {

      history[i-1] = history[i];
    }
    history[100 - 1] = strdup(currCommand);
  }
  return 1;
}

//get function to print the commands stored in history array
int get_Shell_History() {
  int i = 0;
  while(i != 100 && history[i] !=NULL) {
    printf("%s\n", history[i]);
    i++;
  }
  return 1;
}

//Signal function used to interrupt shell and ask for confirmation to terminate shell
// used with the ^C signal. 
void signal_exit(int sig){

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    printf("Are you sure you want to exit the shell?: [y/n] ");
    
    char key; 
    
    key =getchar();

    if (key=='Y' || key == 'y' ){
        
        exit(0);
    }

    else {

        signal(SIGINT, signal_exit);
    }

    getchar();
}


//Signal function used to ignore the ^Z command in the shell 
// used with the ^Z signal.
void signal_ignore(int sig) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTSTP, signal_ignore);
    fflush(stdout);
}


// funcion that is the executable function for commands
// after line has been read and interpretted
int my_system(char **args)
{
signal(SIGINT, signal_ignore);
signal(SIGINT, signal_exit);

  int i;

//no command was entered
  if (args[0] == NULL) {
    return 1;
  }

//if history command entered shell prints all commands in history
  else if (strcmp(args[0], "history") == 0) {
      get_Shell_History();

  } 

  else {
  //if any other internal command is called, the arg[0] (command keyword) 
  //will be searched for in shellCommand, and its associated function
  //from shellFunctions, will execute
  for (i = 0; i < shellCommandLength(); i++) {
    if (strcmp(args[0], shellCommand[i]) == 0) {
      return (*shellFunctions[i])(args);
    }
  }

  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {

    if (execvp(args[0], args) == -1) {
      perror("error");
    }
    exit(EXIT_FAILURE);
  }
   else if (pid < 0) {

    perror("error");

  }

  
  
  else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;


}
}

//read the input line from the shell
char *read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; 
  getline(&line, &bufsize, stdin);
  return line;
}


//tokenizes the input of the line
char **parse(char *line)
{
  int bufsize = 64,
  position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *tok;

  if (!tokens) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  tok = strtok(line, TOKEN_DELIM);

  while (tok != NULL) {
    tokens[position] = tok;
    position++;


    tok = strtok(NULL, TOKEN_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}



//Function to get and tokenize the input line, then enter my_system to do the appropriate execution of the command 
void get_a_line(void)
{
  char *line;
  char **args;
  int status;
  int hist;


  do {
    printf("my_tiny_shell > ");
    line = read_line();
    args = parse(line);
    hist = shell_history(line);
    status = my_system(args);

    free(line);

    free(args);
  } 
  
  while (status);
}

//Main function that loops get a line
int main(int argc, char **argv) {

  get_a_line();

  return EXIT_SUCCESS;
}