#include <fc/ListHandler.hpp>
#include <fc/List.hpp>
#include <fc/Common.hpp>


namespace fc
{
  using enum fc::common::ListType;
  using enum fc::response::Status;


  
  template<typename T>
  static void dumpList(List& list)
  {
    #ifdef FC_DEBUG
      const auto& l = std::get<T>(list);

      for (const auto n : l)
        PLOGD << n;
    #endif
  }
  


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListCreate& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    if (req.type() == ListType_Int)
    {
      PLOGD << "ListCreate: creating Int list";
      if (const auto [_, created] = m_lists.try_emplace(req.name()->str(), std::make_unique<FcList>(FlexType::FBT_VECTOR_INT, IntList{})); !created)
        status = Status_Duplicate;
    }
    else if (req.type() == ListType_UInt)
    {
      PLOGD << "ListCreate: creating UInt list";
      if (const auto [_, created] = m_lists.try_emplace(req.name()->str(), std::make_unique<FcList>(FlexType::FBT_VECTOR_UINT, UIntList{})); !created)
        status = Status_Duplicate;
    }
    else
    {
      PLOGE << "ListCreate: unknown list type";
      status = Status_Fail;
    }

    createEmptyBodyResponse(fbb, status, ResponseBody_ListCreate);
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListAdd& req) noexcept
  {
    fc::response::Status status = Status_Ok;
    const auto& name = req.name()->str();

    if (const auto listOpt = getList(name); !listOpt) [[unlikely]]
    {
      status = Status_Fail;
    }
    else
    {
      const auto& itemsVector = req.items_flexbuffer_root().AsTypedVector();
      const auto& fcList = (*listOpt)->second;

      auto add = [pos = req.position(), base=req.base(), &itemsVector, &list = fcList->list()]<typename T>()
      {
        std::visit(Add<T>{itemsVector, base, std::abs(pos)}, list);
      };

      switch (fcList->type())
      {
        case FlexType::FBT_VECTOR_INT:
          add.template operator()<std::int64_t>();
          dumpList<IntList>(fcList->list());
        break;

        case FlexType::FBT_VECTOR_UINT:
          add.template operator()<std::uint64_t>();
          dumpList<UIntList>(fcList->list());
        break;

        default:
          PLOGE << "Unknown list type";
          status = Status_Fail;
        break;
      }

      
    }

    createEmptyBodyResponse(fbb, status, fc::response::ResponseBody_ListAdd);
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListDelete& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    try
    {
      if (req.name()->empty())
      {
        m_lists.clear();
      }
      else
      {
        for (const auto name : *req.name())
          m_lists.erase(name->str());
      }    
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = Status_Fail;
    }
    
    createEmptyBodyResponse(fbb, status, fc::response::ResponseBody_ListDelete);
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListGetN& req) noexcept
  {
    try
    {
      const auto& name = req.name()->str();
      const auto start = req.start();
      const auto count = req.count();
      
      if (!((start >= 0 && count >= 0) || (start < 0 && count < 0)))  [[unlikely]]
      {
        PLOGE << "ListGet: start/stop must be both positive or both negative";
        createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListGetN);
      }
      else if (const auto listOpt = getList(name); !listOpt)
      {
        createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListGetN);
      }
      else
      {
        // TODO estimate this more accurately by using (count * sizeof(int64_t)) + padding?
        //      Padding is for FlexBuffer stuff. perhaps add a typeSize() to FcList
        FlexBuilder flxb{2048U}; 
        const auto& fcList = (*listOpt)->second;

        auto get = [start, count, &flxb, &list = fcList->list()]<typename T>()
        {
          std::visit(GetByCount<int64_t>{start, count, flxb}, list);
        };
        
        if (fcList->type() == FlexType::FBT_VECTOR_INT)
        {
          get.template operator()<std::int64_t>();
        }
        else if (fcList->type() == FlexType::FBT_VECTOR_UINT)
        {
          get.template operator()<std::uint64_t>();
        }

        flxb.Finish();

        const auto vec = fbb.CreateVector(flxb.GetBuffer());
        const auto body = fc::response::CreateListGetN(fbb, vec);
        
        auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_ListGetN, body.Union());
        fbb.Finish(rsp);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListGetN);
    }
  }


  void ListHandler::createEmptyBodyResponse (FlatBuilder& fbb, const fc::response::Status status, const fc::response::ResponseBody bodyType) noexcept
  {
    // TODO this function is also in KvHandler. Move to Common.hpp or create Handler base class
    try
    {
      const auto rsp = fc::response::CreateResponse (fbb, status, bodyType);
      fbb.Finish(rsp);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
    }
  }
}
