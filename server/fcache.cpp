#include <fc/Server.hpp>
#include <fc/Common.hpp>
#include <fc/LogFormatter.hpp>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <latch>
#include <tuple>

static std::latch run{1U};
static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


static void kvSigHandle(int param)
{
  run.count_down();
}


void usage(const char opt = ' ')
{
  PLOGE <<  "Incorrect switch " << opt << '\n' <<
            "--ip <ipv4>:   The IPv4 address for the server (default 127.0.0.1)\n"
            "--port <p>:    Port (default: 1987)";
}


std::tuple<bool, std::string, int> getCmdArgs(int argc, char ** argv)
{
  option opts[] = 
  {
    {"ip",  optional_argument, NULL, 'i'},
    {"port",  optional_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
  };

  bool valid = true;
  std::string ip{"127.0.0.1"};
  int port {1987};

  try
  {
    for (int opt; (opt = getopt_long(argc, argv, "p:", opts, NULL)) != -1; )
    {
      switch (opt)
      {
      case 'i':
        ip = optarg;
        break;
      
      case 'p':
        port = std::stoi(optarg);
        break;

      default:
        valid = false;
        usage(opt);
        break;
      }
    }
  }
  catch(const std::exception& e)
  {
    usage();
    valid = false;
  }

  return {valid, ip, port};
}


int main (int argc, char ** argv)
{
  fc::initLogger(consoleAppender); // always first statement of main()

  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle);
  signal(SIGKILL, kvSigHandle);
  
  const auto [valid, ip, port] = getCmdArgs(argc, argv);
  
  if (!valid)
    return 1;


  fc::Server server;

  if (!server.init())
  {
    PLOGF << "Failed to initialise websocket server";
  }
  else
  {
    if (server.start(ip, port, 8192, 0))
    {
      PLOGI << "fcache started";
      run.wait();
    }
    else
    {
      PLOGF << "Failed to start websocket server";
      run.count_down();
    }
  }

  return 0;
}