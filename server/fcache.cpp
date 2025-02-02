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

// 64 bytes seems a reasonable value for a minimum useful flatbuffers message
static unsigned int MinPayload = 64;
// no particular reason for this max value, but it needs one
static unsigned int MaxPayload = 8 * 1024*1024; 
// having default too low will cause confusion
static unsigned int DefaultPayload = 16 * 1024;


static void kvSigHandle(int param)
{
  run.count_down();
}


void usage()
{
  PLOGE <<  "\n--ip <ipv4>        The IPv4 address for the server (default 127.0.0.1)\n"
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
        if (std::string sPayload{optarg} ; std::any_of(sPayload.cbegin(), sPayload.cend(), [](const auto c){ return !std::isdigit(c);}))
        {
          // TODO allow 'M' suffix
          PLOGE << "maxPayload: can only contain numbers";
          valid = false;
        }          
        else
        {
          const auto size = std::stoul(optarg);

          PLOGW_IF(size < MinPayload) << "maxPayload below minimum, setting to " << MinPayload;
          PLOGW_IF(size > MaxPayload) << "maxPayload exceeds maximum, setting to " << MaxPayload;

          maxPayload = std::clamp<unsigned int>(size, MinPayload, MaxPayload);  
        }
      break;

      default:
        valid = false;
      break;
      }
    }
  }
  catch(const std::exception& e)
  {
    valid = false;
  }

  if (!valid)
    usage();

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