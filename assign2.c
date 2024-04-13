/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.

//***********************************************************************************
/***********************************************************************************/

/*
  To compile assign2_template-v3.c ensure that gcc is installed and run 
  the following command:

  gcc your_program.c -o your_ass-2 -lpthread -lrt -Wall
*/

#include  <pthread.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/msg.h>
#include  <sys/ipc.h>
#include  <fcntl.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <semaphore.h>
#include  <sys/time.h>

#define MESSAGE_SIZE 512

/* --- Structs --- */
typedef struct ThreadParams {
  int pipeFile[2];
  sem_t sem_A, sem_B, sem_C;
  char message[MESSAGE_SIZE];
  pthread_mutex_t lock;
} ThreadParams;

typedef struct message_buffer{
  long message_type;
  char message[MESSAGE_SIZE];
}message_buffer;

/* Global variables */
int sum = 1;
char data_file[50];
char output_file[50];
pthread_attr_t attr;


/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void* ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void* ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void* ThreadC(void *params);

const char* read_file(ThreadParams* params);

void simple_pipe_writing(ThreadParams* params, const char* string);

void simple_pipe_reading(ThreadParams* params);

void message_sending(ThreadParams* params);

message_buffer message_receiving();

void write_file(message_buffer message);

/* --- Main Code --- */
int main(int argc, char const *argv[]) {
  
  //int err;
  int simple_pipe_result;
  pthread_t tid[3];
  ThreadParams params;
  
  // Checking for errors from arguments
  if(argc<3){
    fprintf(stderr,"usage: ./program_name data.txt output.txt  \n<note: Run the above line of code, which is part of the requirements of Assessment Task 2.>\n");
    return -1;
  }
  if(strcmp(argv[1], "data.txt") != 0){
    fprintf(stderr,"Please use data.txt as the second argument. Note: program is case sensitive.\n");
    return -1;
  }
  
  strcpy(data_file, argv[1]); //copy file name from the command line argument
  strcpy(output_file, argv[2]);

  printf("File to be read from: %s     File to be written into: %s\n", data_file, output_file);

  // Initialization
  initializeData(&params);
  
  printf("Initial value of sum = %d\n", sum);

  simple_pipe_result = pipe(params.pipeFile);
  if (simple_pipe_result < 0){
    perror("Simple pipe failed.");
    exit(1);
  }
  // Create Threads / Error handling
  if (pthread_create(&(tid[0]), &attr, ThreadA, (void*)(&params)) != 0){
  	printf ("\n Thread-A can't be created \n");
    exit (2);
  }
  if (pthread_create(&(tid[1]), &attr, ThreadB, (void*)(&params)) != 0){
  	printf ("\n Thread-B can't be created \n");
    exit (2);
  }
  if (pthread_create(&(tid[2]), &attr, ThreadC, (void*)(&params)) != 0){
  	printf ("\n Thread-C can't be created \n");
    exit (2);
  }
  //TODO: add your code
  sem_post(&(params.sem_A));

  // Wait on threads to finish
  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);

    
  //TODO: add your code




  printf("Final value of sum = %d\n", sum);
  return 0;
}

void initializeData(ThreadParams *params) {
  // Initialize Sempahores
  if (sem_init(&(params->sem_A), 0, 0)!=0){
    printf("\nsem_A has failed\n"); 
    exit(1);
  }

  if (sem_init(&(params->sem_B), 0, 0)!=0){
    printf("\nsem_B has failed\n"); 
    exit(1);
  }

  if (sem_init(&(params->sem_C), 0, 0)!=0){
    printf("\nsem_C has failed\n"); 
    exit(1);
  }

  // Initialize thread attributes 
  if (pthread_attr_init(&attr)!=0){
    printf("\n pthread_attr_init has failed\n");
    exit(1);
  }
  
  //TODO: add your code
}

void* ThreadA(void *params) {

  ThreadParams* param = (ThreadParams*) params; 

  sem_wait(&(param->sem_A));

  printf("Thread A: attempt to read file data.txt...\n");

  //const char* filedata = read_file(param);

  printf("\nThread A has finished reading from data.txt.\n");

  simple_pipe_writing(param, read_file(param));

  printf("Thread A has finished writing pipe package.\n");

  printf("\nIn Thread A, \n");
  
  for (int i = 0; i < 5; i++){
    sum *= 2;
    printf("sum = %d\n", sum);
  }

  sem_post(&(param->sem_B));
  
  return 0;
}

void* ThreadB(void *params) {

  ThreadParams* param = (ThreadParams*) params; 

  sem_wait(&(param->sem_B));

  printf("\nThread B: attempts to read file content from pipe...\n");

  simple_pipe_reading(param);

  message_sending(param);

  printf("\nIn Thread B: \n");

  for (int i = 0; i < 3; i++){
    sum *= 3;
    printf("sum = %d\n", sum);
  }

  sem_post(&(param->sem_C));

  return 0;
}

void* ThreadC(void *params) {

  ThreadParams* param = (ThreadParams*) params; 

  sem_wait(&(param->sem_C));

  printf("\nThread C: attempt to receive message from Thread B...\n");
  
  

  //printf("%s\n", );

  write_file(message_receiving());

  printf("\nIn Thread C, \n");
  
  for (int i = 0; i < 4; i++){
    sum -= 5;
    printf("sum = %d\n", sum);
  }

  return 0;
}


const char* read_file(ThreadParams* params){
  char c[50];
  FILE *fptr;
  static char text[MESSAGE_SIZE];

  if ((fptr = fopen(data_file, "r")) == NULL) {
    printf("Error! opening file");
    // Program exits if file pointer returns NULL.
    exit(3);
  }

  // reads text until newline is encountered
  printf("reading from the file:\n"); 

  while(fgets(c, sizeof(c), fptr) != NULL) {
	  //fputs(c, stdout);
    strcat(text, c);
  }

  fclose(fptr);

  return text;
}

void simple_pipe_writing(ThreadParams* params, const char* string){
  int i=0;
  int result;
  char pipe_package[500];

  strcpy(pipe_package, string);
  
  while (pipe_package[i]!='\0'){
    result = write(params->pipeFile[1], &pipe_package[i], 1);
    
    if (result != 1){
      perror("Failed to write pipe package.");
      exit(4);
    }
    //printf("%c", pipe_package[i]);
    i++;
  }

  result = write(params->pipeFile[1], &pipe_package[i], 1);
  
  if (result != 1){
    perror("Failed to write pipe package.");
    exit(4);
  }
}

void simple_pipe_reading(ThreadParams* params){
  char text;
  int result;
  //static char message[500];
  printf("\nThread B is reading from the pipe...\n");
  
  printf("Reader:\n");

  while(1){
    result = read(params->pipeFile[0], &text, 1);

    if (result != 1){
      perror("Failed to read from the pipe.");
      exit(4);
    }

    if (text != '\0'){
      //printf("%c", text);
      strncat(params->message, &text, 1);
    }

    else {
      printf("Thread B has finished reading from pipe.\n");
      break;
      }
  }
  //printf("%s\n", message);
  //return message;
}

void message_sending(ThreadParams* params){
  int queue_id;
  int message_flag = IPC_CREAT | 0666;
  key_t key = 13252768;
  //size_t buffer_length; = fopen(output_file, 'w')
  message_buffer send_buffer; //might be unnecessary
  
	(void)printf("\nmsgget: Calling function msgget(%#1x,\%#o)\n", key, message_flag);

  if ((queue_id = msgget(key, message_flag)) < 0){
    perror("Failed to run msgget.");
    exit(5);
  }

  else {
		(void)printf("msgget: msgget succeeded: msgqid = %d\n", queue_id);
  }

  send_buffer.message_type = 1;

  (void) strcpy(send_buffer.message, params->message);

  //buffer_length = strlen(send_buffer.message) + 1;

  	// Send a message, you can type "man msgsnd" on your linux terminal
	if((msgsnd(queue_id, &send_buffer.message, MESSAGE_SIZE, IPC_NOWAIT)) < 0){
		printf("%d, %ld, %s\n", queue_id, send_buffer.message_type, send_buffer.message);
		perror("msgsnd");
		exit(5);
	}

	else{
	  printf ("msqid=%d, sbuf.mtype=%ld, sbuf.mtext=\n%s\n", queue_id, send_buffer.message_type, send_buffer.message);
    printf("Message: \n%s--Sent\n", send_buffer.message);
  } 
}

message_buffer message_receiving(){
  int queue_id;
  key_t key = 13252768;
  static message_buffer receiving_buffer;
  static char received_message[MESSAGE_SIZE];
  
  if ((queue_id = msgget(key, 0666)) < 0){
    perror("Failed to run msgget.");
    exit(5);
  }

  printf("\nmsgget: Getting Message ID (%d)\n", queue_id);
  
  if (msgrcv(queue_id, &receiving_buffer.message, MESSAGE_SIZE, 0, IPC_NOWAIT) < 0){
    perror("Failed to run msgrcv.");
    exit(5);
  }

  strcat(received_message, receiving_buffer.message);
  
  //printf("Received message: \n%s\n", received_message);

  return receiving_buffer;
}

void write_file(message_buffer message){
  char* header_end = "end_header";
  FILE* fptr = fopen(output_file, "w");
  int end_header_length = strlen(header_end);
  char* file_content;
  char text[MESSAGE_SIZE];

  printf("Checking content of the message for end of header...\n");


  if (fptr == NULL){
    printf("Failed to open file.");
    exit(3);
  }

  strcpy(text, message.message);

  char* end_header_ptr = strstr(text, header_end);

  if(end_header_ptr){
    printf("Found end header in message.\n");
    file_content = end_header_ptr + end_header_length;
    if(*file_content == '\n'){
      file_content += 1;
    }
    printf("Thread C: attempting to write message into %s...\n", output_file);
    printf("Content to be written into output file is:\n%s\n", file_content);
  }
  else {
    printf("End header was not found.");
    exit(6);
  }
  fprintf(fptr, "%s", file_content);
  fclose(fptr);

  printf("Finished writing into the output file.");
}