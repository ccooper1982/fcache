#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <fstream>
#include <filesystem>

// #pragma GCC diagnostic push 
// #pragma GCC diagnostic ignored "-Wnrvo"
//   #include <uWebSockets/App.h>
// #pragma GCC diagnostic push

#include <fc/Common.hpp>
#include <fc/FlatBuffers.hpp>
#include <fc/KvHandler.hpp>
#include <fc/ListHandler.hpp>


namespace fc
{
  class Server
  {
  public:
    Server() : m_run(true)
    {
      
    }


    ~Server()
    {
      stop();
    }

    bool init();
    bool start(const std::string& ip, const int port, const unsigned int maxPayload, const std::size_t core);
    void stop() ;

  
  private:
    void onMessage(WebSocket * ws, std::string_view message, const uWS::OpCode opCode);
    void sendFailure (WebSocket * ws, FlatBuilder& fbb, const fc::response::Status status);
    void send (WebSocket * ws, const FlatBuilder& builder);

    void handleKv(WebSocket * ws, FlatBuilder& fbb, const fc::request::Request& request);
    void handleList(WebSocket * ws, FlatBuilder& fbb, const fc::request::Request& request);

  private:
    struct TimerData
    {
      // TODO use when KV expiry is implemented
    };

    std::unique_ptr<std::jthread> m_wsThread;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<WebSocket *> m_clients;
    std::atomic_bool m_run;
    //us_timer_t * m_monitorTimer{};

    // could be a unique_ptr but when the Timer is used for key expiry,
    // this needs to be in the TimerData struct for the timer callback.
    std::shared_ptr<KvHandler> m_kvHandler;
    std::shared_ptr<ListHandler> m_listHandler;
  };
}
