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
    auto create = [this](auto&& list, const std::string& name) -> fc::response::Status
    {
      const auto [_, created] = m_lists.try_emplace(name, std::make_unique<FcList>(std::forward<decltype(list)>(list)));
      return created ? Status_Ok : Status_Duplicate;      
    };

    fc::response::Status status = Status_Fail;

    if (req.type() == ListType_Int)
    {
      PLOGD << "ListCreate: Int list";
      status = create(IntList{}, req.name()->str());
    }
    else if (req.type() == ListType_UInt)
    {
      PLOGD << "ListCreate: UInt list";
      status = create(UIntList{}, req.name()->str());
    }
    else if (req.type() == ListType_Float)
    {
      PLOGD << "ListCreate: float list";
      status = create(FloatList{}, req.name()->str());
    }
    else if (req.type() == ListType_String)
    {
      PLOGD << "ListCreate: string list";
      status = create(StringList{}, req.name()->str());
    }    
    else
    {
      PLOGE << "ListCreate: unknown list type";
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

      switch (fcList->type())
      {
        case FlexType::FBT_VECTOR_INT:
        case FlexType::FBT_VECTOR_UINT:
        case FlexType::FBT_VECTOR_FLOAT:
        case FlexType::FBT_VECTOR_KEY:
          std::visit(Add{itemsVector, req.base(), std::abs(req.position())}, fcList->list());
        break;

        default:
          PLOGE << "Unknown type for list: " << fcList->type();
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


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListGetRange& req) noexcept
  {
    try
    {
      const auto& name = req.name()->str();
      const int32_t start = req.range()->start();
      const int32_t stop = req.range()->stop();
      const bool hasStop = req.range()->has_stop();
      const auto base = req.base();

      if (const auto listOpt = getList(name); !listOpt)
      {
        // TODO Status_NotExist for when a List doesn't exist? Or an empty vector? A FAIL seems a bit unnecessary
        createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListGetRange); 
      }
      else
      {
        const auto& fcList = (*listOpt)->second;

        FlexBuilder flxb{2048U}; 

        const bool createdBuffer = hasStop ?  std::visit(GetByRange{flxb, start, stop, base}, fcList->list()) :
                                              std::visit(GetByRange{flxb, start, base}, fcList->list());
        
        if (!createdBuffer)
          flxb.TypedVector([]{}); // return empty vector

        flxb.Finish();
        const auto vec = fbb.CreateVector(flxb.GetBuffer());
        const auto body = fc::response::CreateListGetRange(fbb, vec);
        
        auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_ListGetRange, body.Union());
        fbb.Finish(rsp);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListGetRange);
    }
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListRemove& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    try
    {
      const auto& name = req.name()->str();
      const int32_t start = req.range()->start();
      const int32_t stop = req.range()->stop();
      const bool hasStop = req.range()->has_stop();

      if (const auto listOpt = getList(name); !listOpt)
      {
        // TODO Status_NotExist for when a List doesn't exist? Or an empty vector? A FAIL seems a bit unnecessary
        status = Status_Fail;
      }
      else
      {
        const auto& fcList = (*listOpt)->second;

        if (hasStop)
          std::visit(Remove{start, stop}, fcList->list());
        else
          std::visit(Remove{start}, fcList->list());
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = Status_Fail;
    }

    createEmptyBodyResponse(fbb, status, ResponseBody_ListRemove);    
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListRemoveIf& req) noexcept
  {
    using enum fc::request::Condition;
    using enum fc::request::Value;

    fc::response::Status status = Status_Ok;

    try
    {
      const auto& name = req.name()->str();
      const int32_t start = req.range()->start();
      const int32_t stop = req.range()->stop();
      const bool hasStop = req.range()->has_stop();
      // const auto condition = req.condition();  // not used, only one condition at the moment

      if (const auto listOpt = getList(name); !listOpt)
      {
        // TODO Status_NotExist for when a List doesn't exist? Or an empty vector? A FAIL seems a bit unnecessary
        status = Status_Fail;
      }
      else
      {
        const auto& fcList = (*listOpt)->second;

        switch (req.value_type())
        {
        case Value_IntValue:
          doRemoveIf<IsEqual<fcint>>(start, stop, hasStop, req.value_as_IntValue()->v(), fcList->list());
        break;

        case Value_UIntValue:
          doRemoveIf<IsEqual<fcuint>>(start, stop, hasStop, req.value_as_UIntValue()->v(), fcList->list());
        break;

        case Value_StringValue:
          doRemoveIf<IsEqual<std::string>>(start, stop, hasStop, req.value_as_StringValue()->v()->str(), fcList->list());
        break;

        case Value_FloatValue:
          doRemoveIf<IsEqual<fcfloat>>(start, stop, hasStop, req.value_as_FloatValue()->v(), fcList->list());
        break;

        default:
        {
          PLOGE << "List::remove_if value type not supported";
        }
        break;
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = Status_Fail;
    }

    createEmptyBodyResponse(fbb, status, ResponseBody_ListRemoveIf);
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
