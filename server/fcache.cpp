#include <fc/Server.hpp>
#include <fc/Common.hpp>
#include <fc/LogFormatter.hpp>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <iostream>
#include <latch>


static std::latch run{1U};
static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


static void kvSigHandle(int param)
{
  run.count_down();
}


int main (int argc, char ** argv)
{
  fc::initLogger(consoleAppender); // always first statement of main()

  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle);
  signal(SIGKILL, kvSigHandle);

  /*
  option opts[] = 
  {
    {"config",  required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
  };


  for (int opt; (opt = getopt_long(argc, argv, "c", opts, NULL)) != -1; )
  {
    switch (opt)
    {
    case 'c':
      PLOGD << "Config: " << optarg ;
      break;
    
    default:
      break;
    }
  }
  */

  fc::Server server;

  if (!server.init())
  {
    PLOGF << "fcache failed to initialise";
  }
  else
  {
    if (server.start("127.0.0.1", 1987, 1024, 0))
    {
      PLOGI << "fcache started";
      run.wait();
    }
    else
      run.count_down();
  }

  return 0;
}