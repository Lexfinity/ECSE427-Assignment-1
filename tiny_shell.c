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


#define BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"
char *history[100];
int history_count = 0;
char * lastCommand;
#define HISTORY_LENGTH 100;


// int shell_history(char *command )
// {
//    if (history_count < 100) {
//         history[history_count++] = strdup( command );
//    } else {
//         free( history[0] );
//         for (unsigned index = 1; index < 100; index++) {
//             history[index - 1] = history[index];
//         }
//         history[100 - 1] = strdup( command );
//     }
// }


int shell_cd(char **args);
int shell_exit(char **args);
int shell_history();


char *shellCommand[] = {
  "cd",
  "exit",
  "chdir",
  "history"
};


int (*shellFunctions[]) (char **) = {
  &shell_cd,
  &shell_exit,
  &shell_cd,
  &shell_history
};

int shellCommandLength() {
  return sizeof(shellCommand) / sizeof(char *);
}


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


int shell_exit(char **args){ return 0; }

int shell_history() {

    int count = history_count;
    //  % 100;
    while(count < 100) {
        if(history[count] !=NULL) {
            printf("%s\n", history[count]);
        }
        count++;
    }
    int i;
    // while(count < (history_count%100)) {
    for(i = 0; i < history_count; i++) {
        if(history[i] != NULL) {
        printf("%s\n", history[i]);
    }
        // count++;
    }
    return 1;
}


// int lsh_launch(char **args)
// {
  // pid_t pid, wpid;
  // int status;

  // pid = fork();
  // if (pid == 0) {
  //   // Child process
  //   if (execvp(args[0], args) == -1) {
  //     perror("lsh");
  //   }
  //   exit(EXIT_FAILURE);
  // } else if (pid < 0) {
  //   // Error forking
  //   perror("lsh");
  // } else {
  //   // Parent process
  //   do {
  //     wpid = waitpid(pid, &status, WUNTRACED);
  //   } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  // }

  // return 1;
// }

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

void signal_ignore(int sig) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTSTP, signal_ignore);
    fflush(stdout);
}

int my_system(char **args)
{
signal(SIGINT, signal_ignore);
signal(SIGINT, signal_exit);

lastCommand = history[history_count -1];
history[history_count] = args[0];
history_count = history_count + 1;



  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

//   else if (strcmp(args[0], "history") == 0) {
//       shell_history();
//   } 

  else {

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

char *read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; 
  getline(&line, &bufsize, stdin);
  return line;
}



char **parse(char *line)
{
  int bufsize = 64,
  position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOKEN_DELIM);

  while (token != NULL) {
    tokens[position] = token;
    position++;


    token = strtok(NULL, TOKEN_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}


void get_a_line(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("my_tiny_shell > ");
    line = read_line();
    args = parse(line);
    status = my_system(args);

    free(line);
    free(args);
  } while (status);
}


int main(int argc, char **argv) {

  get_a_line();

  return EXIT_SUCCESS;
}