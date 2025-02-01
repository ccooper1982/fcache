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
    const fc::response::Status status = createList(req.name()->str(), req.type(), req.sorted());
    createEmptyBodyResponse(fbb, status, ResponseBody_ListCreate);
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListAdd& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    try
    {
      const auto& name = req.name()->str();
      const auto& itemsVector = req.items_flexbuffer_root().AsTypedVector();

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListAdd); listOpt && itemsVector.size()) [[likely]]
      {
        const auto& fcList = (*listOpt)->second;

        switch (fcList->type())
        {
          case FlexType::FBT_VECTOR_INT:
          case FlexType::FBT_VECTOR_UINT:
          case FlexType::FBT_VECTOR_FLOAT:
          case FlexType::FBT_VECTOR_KEY:
            if (fcList->isSorted())
              std::visit(Add<true>{itemsVector, req.base(), req.items_sorted()}, fcList->list());
            else
              std::visit(Add<false>{itemsVector, req.base(), std::abs(req.position())}, fcList->list());
          break;

          default:  [[unlikely]]
            PLOGE << "Unknown type for list: " << fcList->type();
            status = Status_Fail;
          break;
        }      
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = Status_Fail;
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
        // if list doesn't exist, it's not an error
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

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListGetRange); listOpt)  [[likely]]
      {
        const auto& fcList = (*listOpt)->second;

        FlexBuilder flxb{4096U}; 

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

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListRemove); listOpt)  [[likely]]
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

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListRemoveIf); listOpt)  [[likely]]
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
  

  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListIntersect& req) noexcept
  {
    try
    {
      const auto list1Opt = haveList(fbb, req.list1_name()->str(), ResponseBody_ListIntersect);
      const auto list2Opt = haveList(fbb, req.list2_name()->str(), ResponseBody_ListIntersect);

      if (list1Opt && list2Opt)  [[likely]]
      {
        const auto& fcList1 = (*list1Opt)->second;
        const auto& fcList2 = (*list2Opt)->second;

        // TODO: allow intersecting of IntList and UIntlist

        // check sorted and types
        if (!fcList1->canIntersectWith(*fcList2))
          createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListIntersect);        
        else
        {
          const auto& range1 = *(req.list1_range());
          const auto& range2 = *(req.list2_range());
          const bool createNewList = req.new_list_name() && !req.new_list_name()->str().empty();

          FlexBuilder flxb{4096U};

          if (fc::common::ListType listType; createNewList && flexTypeToListType(fcList1->type(), listType))
          {
            const auto& newListName = req.new_list_name()->str(); 
            if (createList(newListName, listType, true) == Status_Ok)
            {
              if (const auto newListOpt = getList(newListName) ; newListOpt)
              {
                auto& newList = (*newListOpt)->second->list();
                std::visit([&flxb, &range1, &range2, &l1list = fcList1->list(), &l2list = fcList2->list()](auto& newList) mutable
                {
                  // list1, list2 and newList are same type but newList is not const
                  const auto& l1 = std::get<std::remove_cvref_t<decltype(newList)>>(l1list);
                  const auto& l2 = std::get<std::remove_cvref_t<decltype(newList)>>(l2list);
                  intersect<std::remove_cvref_t<decltype(newList)>>(newList, l1, l2, range1, range2);                  
                },
                newList);
              }
            }
            
            // always return a valid/non-empty flexbuffer
            flxb.TypedVector([]{}); 
            flxb.Finish();
          }
          else
          {
            std::visit([&flxb, &range1, &range2, &other = fcList2->list()](const auto& l1)
            {
              // list1 and list2 are same type, checked above
              const auto& l2 = std::get<std::remove_cvref_t<decltype(l1)>>(other);  
              intersect(flxb, l1, l2, range1, range2);
            },
            fcList1->list());
          }

          // intersect() finishes the FlexBuilder
          const auto vec = fbb.CreateVector(flxb.GetBuffer());
          const auto body = fc::response::CreateListIntersect(fbb, vec);
          
          auto rsp = fc::response::CreateResponse(fbb, Status_Ok, ResponseBody_ListIntersect, body.Union());
          fbb.Finish(rsp);
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListIntersect);
    }
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListSet& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    try
    {
      const auto& name = req.name()->str();
      const auto& itemsVector = req.items_flexbuffer_root().AsTypedVector();

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListSet); listOpt && itemsVector.size())
      {
        if (const auto& fcList = (*listOpt)->second; fcList->isSorted())
          status = Status_NotPermitted;
        else
        {
          switch (fcList->type())
          {
            case FlexType::FBT_VECTOR_INT:
            case FlexType::FBT_VECTOR_UINT:
            case FlexType::FBT_VECTOR_FLOAT:
            case FlexType::FBT_VECTOR_KEY:
            {
              std::visit(Set{itemsVector, req.base(), std::abs(req.position())}, fcList->list());
            }
            break;

            default:  [[unlikely]]
              PLOGE << "Unknown type for list: " << fcList->type();
              status = Status_Fail;
            break;
          }
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = Status_Fail;
    }

    createEmptyBodyResponse(fbb, status, fc::response::ResponseBody_ListSet);
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


  fc::response::Status ListHandler::createList(const std::string& name, const fc::common::ListType type, const bool sorted)
  {
    auto create = [this, sorted, &name](auto&& list) -> fc::response::Status
    {
      const auto [_, created] = m_lists.try_emplace(name, std::make_unique<FcList>(std::forward<decltype(list)>(list), sorted));
      return created ? Status_Ok : Status_Duplicate;      
    };

    fc::response::Status status = Status_Fail; 

    if (type <= fc::common::ListType_MAX)
    {
      PLOGD << "ListCreate: " << fc::common::EnumNamesListType()[type] << " list, sorted: " << std::boolalpha << sorted;

      if (type == ListType_Int)
        status = create(IntList{});
      else if (type == ListType_UInt)
        status = create(UIntList{});
      else if (type == ListType_Float)
        status = create(FloatList{});
      else if (type == ListType_String)
        status = create(StringList{});
    }

    return status;
  }
}

