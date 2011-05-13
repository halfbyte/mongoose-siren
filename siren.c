#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <usb.h>
#include <sys/time.h>
#include <pthread.h>
#include <k8055.h>

#include "mongoose.h"

static const char *start_command = "start";
static const char *stop_command = "stop";
static const char *red_siren = "red";
static const char *blue_siren = "blue";
static const char *post_method = "POST";

static int exit_flag;



static const char *reply_start =
  "HTTP/1.1 200 OK\r\n"
  "Cache: no-cache\r\n"
  "Content-Type: text/plain\r\n"
  "\r\n";
  
static const char *reply_ok = "%s %s OK";

static void signal_handler(int sig_num) {
  exit_flag = sig_num;
}

int channel_for_siren(char *siren_name) {
  if (strcmp(siren_name, red_siren) == 0) {
    return 1;
  } else if (strcmp(siren_name, blue_siren) == 0) {
    return 8;
  } else {
    return -1;
  }  
  
}

static void *event_handler(enum mg_event event,
                           struct mg_connection *conn,
                           const struct mg_request_info *request_info) {
  void *processed = "yes";
  char *command;
  char *color;
  int channel;
  printf("URL: %s\n", request_info->uri);
  
  
  if (event == MG_NEW_REQUEST) {
    if (strcmp(request_info->request_method, post_method) == 0) {
      command = strtok(request_info->uri, "/");
      printf("CMD: %s\n", command);
      color = strtok(NULL, "/");
      if (color == NULL) return NULL;
      printf("COLOR: %s\n", color);
      
      channel = channel_for_siren(color);
      if (channel == -1) {
        processed = NULL;
      } else {
        if (strcmp(command, start_command) == 0) {
          SetDigitalChannel(channel);
        } else if (strcmp(command, stop_command) == 0) {
          ClearDigitalChannel(channel);
        } else {
          processed = NULL;
        }
      }
      if (processed != NULL) {
        mg_printf(conn, reply_start);
        mg_printf(conn, reply_ok, color, command);
      }
    } else {
      processed = NULL;
    }
    
  } else {
    processed = NULL;
  }

  return processed;
}

static const char *options[] = {
  "document_root", "public",
  "listening_ports", "8080",
  "num_threads", "5",
  NULL
};

int main(void) {
  struct mg_context *ctx;

  OpenDevice(0);
  // Initialize random number generator. It will be used later on for
  // the session identifier creation.
  srand((unsigned) time(0));

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);


  // Setup and start Mongoose
  ctx = mg_start(&event_handler, NULL, options);
  assert(ctx != NULL);

  // Wait until enter is pressed, then exit
  printf("Siren server started on port %s, press CTRL-C to quit.\n",
         mg_get_option(ctx, "listening_ports"));
  while (exit_flag == 0) {
    sleep(1);
  }
  printf("Exiting on signal %d, waiting for all threads to finish...",
        exit_flag);
  fflush(stdout);
  mg_stop(ctx);

  return EXIT_SUCCESS;
}