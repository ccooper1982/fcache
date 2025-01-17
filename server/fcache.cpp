#include <fc/Server.hpp>
#include <fc/Common.hpp>
#include <fc/LogFormatter.hpp>
#include <fc/Memory.hpp>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <latch>
#include <tuple>


static std::latch run{1U};
static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


static unsigned int MinPayload = 64;
static unsigned int MaxPayload = 4 * 1024*1024;
#ifdef FC_DEBUG
static unsigned int DefaultPayload = 16384;
#else
static unsigned int DefaultPayload = 2048;
#endif



static void kvSigHandle(int param)
{
  run.count_down();
}


void usage(const char opt = ' ')
{
  PLOGE <<  "Incorrect switch " << opt << '\n' <<
            "--ip <ipv4>        The IPv4 address for the server (default 127.0.0.1)\n"
            "--port <p>         Port (default: 1987)\n"
            "--maxPayload <n>   Max bytes accepted by the WebSocket server";
}


std::tuple<bool, std::string, int, unsigned long> getCmdArgs(int argc, char ** argv)
{
  option opts[] = 
  {
    {"ip",  optional_argument, NULL, 0},
    {"port",  optional_argument, NULL, 1},
    {"maxPayload", optional_argument, NULL, 2},
    {NULL, 0, NULL, 0}
  };

  bool valid = true;
  std::string ip{"127.0.0.1"};
  int port {1987};
  unsigned int maxPayload{DefaultPayload};

  try
  {
    for (int opt; (opt = getopt_long(argc, argv, "p:", opts, NULL)) != -1; )
    {
      switch (opt)
      {
      case 0:
        ip = optarg;
      break;
      
      case 1:
        port = std::stoi(optarg);
      break;
      
      case 2:
        // low: 64B, high: 2MB
        maxPayload = std::clamp<unsigned int>(std::stoul(optarg), MinPayload, MaxPayload);  
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

  return {valid, ip, port, maxPayload};
}


int main (int argc, char ** argv)
{
  fc::initLogger(consoleAppender); // always first statement of main()

  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle);
  signal(SIGKILL, kvSigHandle);
  
  // TODO warm up memory pools
    
  const auto [valid, ip, port, maxPayload] = getCmdArgs(argc, argv);
  
  if (!valid)
    return 1;


  fc::Server server;

  if (!server.init())
  {
    PLOGF << "Failed to initialise websocket server";
  }
  else
  {
    if (server.start(ip, port, maxPayload, 0))
    {
      PLOGI << "Listening on " << ip << ":" << port;
      PLOGI << "Max payload: " << maxPayload << " bytes";
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