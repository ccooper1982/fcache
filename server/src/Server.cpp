#include <fc/Server.hpp>
#include <fc/FlatBuffers.hpp>
#include <latch>


namespace fc
{
  void Server::stop()
  {
    m_run = false;

    try
    {
      // if (m_monitorTimer)
      //   us_timer_close(m_monitorTimer);

      for (auto ws : m_clients)
        ws->end(1000); // calls wsApp.close()

      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);
    }
    catch (const std::exception& ex)
    {
      //ignore, shutting down
    }
    

    m_clients.clear();
    m_sockets.clear();
    //m_monitorTimer = nullptr;
  }


  bool Server::init()
  {
    bool init = true;

    try
    {
      m_kvHandler = std::make_shared<KvHandler>();
      m_listHandler = std::make_shared<ListHandler>();
    }
    catch(const std::exception& e)
    {
      PLOGF << e.what();
      init = false;
    }

    return init;
  }


  bool Server::start(const std::string& ip, const int port, const unsigned int maxPayload, const std::size_t core)
  {
    bool listening{false};
    std::latch startLatch (1);

    auto listen = [this, ip, port, &listening, &startLatch, maxPayload]()
    {
      auto wsApp = uWS::App().ws<WsSession>("/*",
      {
        // settings
        .compression = uWS::DISABLED, // TODO consider uWS::SHARED_COMPRESSOR
        .maxPayloadLength = maxPayload,
        .idleTimeout = 180, // TODO should be configurable?
        .maxBackpressure = 128 * 1024,  // TODO reduce when draining handles properly
        // handlers
        .open = [this](WebSocket * ws)
        {
          m_clients.insert(ws);
        },          
        .message = [this](WebSocket * ws, std::string_view message, uWS::OpCode opCode)
        {
          onMessage(ws, message, opCode);
        },
        .drain = [](WebSocket * ws)
        {
          // TODO
          // https://github.com/uNetworking/uWebSockets/blob/master/misc/READMORE.md
          //
          // Something like:
          //
          //  a) ws->getBufferedAmount() is the bytes that have not been sent (i.e. are buffered)
          //  b) keep sending until ws->getBufferedAmount() >= maxBackpressure (perhaps check current message can be sent within the backpressure)
          //  c) either drop or queue messages
        },
        .close = [this](WebSocket * ws, int /*code*/, std::string_view /*message*/)
        {
          ws->getUserData()->connected = false;

          // when we shutdown, we have to call ws->end() to close each client otherwise uWS loop doesn't return,
          // but when we call ws->end(), this lambda is called, so we need to avoid mutex deadlock with this flag
          if (m_run)
            m_clients.erase(ws);
        }
      })
      .listen(ip, port, [this, &listening, &startLatch](auto * listenSocket)
      {
        if (listenSocket)
        {
          m_sockets.push_back(listenSocket);

          us_socket_t * socket = reinterpret_cast<us_socket_t *>(listenSocket); // this cast is safe
          listening = us_socket_is_closed(0, socket) == 0U;
        }

        startLatch.count_down();
      });
      

      if (!wsApp.constructorFailed())
      {
        wsApp.run();
      }
    };


    bool started = false;

    try
    {
      m_wsThread.reset(new std::jthread(listen));

      startLatch.wait();

      if (!listening)
      {
        PLOGF << "Failed to start WS server. Check IP and port";
      }          
      else if (!setThreadAffinity(m_wsThread->native_handle(), core))
      {
        PLOGE << "Failed to assign server to core " << core;
      }
      else
      {
        started = true;
      }
    }
    catch(const std::exception& e)
    {
      PLOGF << "Failed to start WS thread:\n" << e.what();
    }

    return started;
  }


  void Server::onMessage(WebSocket * ws, std::string_view message, const uWS::OpCode opCode)
  { 
    FlatBuilder fbb;

    if (opCode != uWS::OpCode::BINARY)
      sendFailure(ws, fbb, fc::response::Status::Status_ParseError);
    else
    {
      if (const auto request = fc::request::GetRequest(message.data()); !request)
      {
        PLOGE << "Get request buffer failed";
        sendFailure(ws, fbb, fc::response::Status::Status_ParseError);
      }
      else
      {
        switch (request->ident())
        {
          case fc::common::Ident_KV:
          {
            handleKv(ws, fbb, *request);
          }
          break;

          case fc::common::Ident_List:
          {
            handleList(ws, fbb, *request);
          }
          break;
          
          default:
          {
            PLOGE << "Request ident not recognised";
            sendFailure(ws, fbb, fc::response::Status::Status_CommandUnknown);
            break;
          }
        }
      }
    }
  }


  void Server::handleKv(WebSocket * ws, FlatBuilder& fbb, const fc::request::Request& request)
  {
    switch (request.body_type())
    {
      case RequestBody_KVSet:
        callKvHandler<fc::request::KVSet>(fbb, request);
      break;

      case RequestBody_KVGet:
        callKvHandler<fc::request::KVGet>(fbb, request);
      break;

      case RequestBody_KVRmv:
        callKvHandler<fc::request::KVRmv>(fbb, request);
      break;

      case RequestBody_KVAdd:
        callKvHandler<fc::request::KVAdd>(fbb, request);
      break;

      case RequestBody_KVCount:
        callKvHandler<fc::request::KVCount>(fbb, request);
      break;
    
      case RequestBody_KVContains:
        callKvHandler<fc::request::KVContains>(fbb, request);
      break;

      case RequestBody_KVClear:
        callKvHandler<fc::request::KVClear>(fbb, request);
      break;

      case RequestBody_KVClearSet:
        callKvHandler<fc::request::KVClearSet>(fbb, request);
      break;

      default:
      {
        PLOGE << "KV command unknown";
        createEmptyBodyResponse(fbb, Status_CommandUnknown, ResponseBody_NONE);
      }        
      break;
    }
    
    send(ws, fbb);
  }
  

  void Server::handleList(WebSocket * ws, FlatBuilder& fbb, const fc::request::Request& request)
  {

    switch (request.body_type())
    {
      case RequestBody_ListCreate:
        callListHandler<fc::request::ListCreate>(fbb, request);
      break;

      case RequestBody_ListAdd:
        callListHandler<fc::request::ListAdd>(fbb, request);
      break;

      case RequestBody_ListDelete:
        callListHandler<fc::request::ListDelete>(fbb, request);
      break;

      case RequestBody_ListGetRange:
        callListHandler<fc::request::ListGetRange>(fbb, request);
      break;

      case RequestBody_ListRemove:
        callListHandler<fc::request::ListRemove>(fbb, request);
      break;

      case RequestBody_ListRemoveIf:
        callListHandler<fc::request::ListRemoveIf>(fbb, request);
      break;

      case RequestBody_ListIntersect:
        callListHandler<fc::request::ListIntersect>(fbb, request);
      break;

      case RequestBody_ListSet:
        callListHandler<fc::request::ListSet>(fbb, request);
      break;

      case RequestBody_ListAppend:
        callListHandler<fc::request::ListAppend>(fbb, request);
      break;

      case RequestBody_ListInfo:
        callListHandler<fc::request::ListInfo>(fbb, request);
      break;

      default:
      {
        PLOGE << "List command unknown";
        createEmptyBodyResponse(fbb, Status_CommandUnknown, ResponseBody_NONE);
      }        
      break;
    }

    send(ws, fbb);
  }


  void Server::sendFailure (WebSocket * ws, FlatBuilder& fbb, const fc::response::Status status)
  {
    const auto rsp = fc::response::CreateResponse(fbb, status);
    fbb.Finish(rsp);
    send(ws, fbb);
  }


  void Server::send (WebSocket * ws, const FlatBuilder& builder)
  {
    const auto buffer = builder.GetBufferPointer();

    const std::string_view sv {reinterpret_cast<char *>(buffer), builder.GetSize()};
    ws->send(sv, uWS::OpCode::BINARY);
  }

}
