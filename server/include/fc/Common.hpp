#pragma once

#define PLOG_OMIT_LOG_DEFINES // must be prior to including plog

#include <uWebSockets/App.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


namespace fc
{
  #define fc_always_inline inline __attribute__((always_inline))

  #ifdef NDEBUG
    #define FC_RELEASE
  #else
    #define FC_DEBUG
  #endif

  using fcint = std::int64_t;
  using fcuint = std::uint64_t;
  using fcfloat = float;
  using fcdouble = double;
  using fcbool = bool;
  using fcblobsize = std::uint32_t;
  using fcstring = std::string;


  // TODO remind myself why we have connected flag, it is never used after being set. 
  //      Should it be used before each call to send()?
  struct WsSession
  {
    WsSession () : connected(true)
    {
    }

    // need this because uWebSockets moves the userdata after upgrade to websocket
    WsSession (WsSession&& other) : connected(other.connected)
    {
      other.connected = false;
    }
    
    ~WsSession()
    {
    }

    bool connected;
  };

  
  using WebSocket = uWS::WebSocket<false, true, WsSession>; // disable SSL, IsServer


  template<class Formatter>
  inline void initLogger (plog::ConsoleAppender<Formatter>& appender)
  {
    static bool init = false;

    if (!init)
    {
      #ifdef FC_DEBUG
        plog::init(plog::verbose, &appender);
      #else
        plog::init(plog::info, &appender);
      #endif
    }
    
    init = true;
  }
  

  bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core);
  
}