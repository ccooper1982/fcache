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
    const auto& name = req.name()->str();
    const auto& items = req.items_flexbuffer_root().AsTypedVector();

    doAddAppend(fbb, name, items, false, req.base(), req.position(), req.items_sorted());
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListAppend& req) noexcept
  {
    const auto& name = req.name()->str();
    const auto& items = req.items_flexbuffer_root().AsTypedVector();

    doAddAppend(fbb, name, items);
  }


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListSet& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    try
    {
      const auto& name = req.name()->str();
      const auto& items = req.items_flexbuffer_root().AsTypedVector();

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListSet); listOpt && items.size())
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
              std::visit(makeSet(items, req.base(), req.position()), fcList->list());
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

    createEmptyBodyResponse(fbb, status, ResponseBody_ListSet);
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
    
    createEmptyBodyResponse(fbb, status, ResponseBody_ListDelete);
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

        const bool createdBuffer = hasStop ?  std::visit(makeGetFullRange(flxb, start, stop, base), fcList->list()) :
                                              std::visit(makeGetPartialRange(flxb, start, base), fcList->list());
        
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
          std::visit(makeRemoveFullRange(start, stop), fcList->list());
        else
          std::visit(makeRemovePartialRange(start), fcList->list());
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

      if (const auto listOpt = haveList(fbb, name, ResponseBody_ListRemoveIf); listOpt)  [[likely]]
      {
        const auto& fcList = (*listOpt)->second;
        auto& list = fcList->list();
        const bool isSorted = fcList->isSorted();
        std::size_t size{0};

        switch (req.value_type())
        {
        case Value_IntValue:
          size = doRemoveIfEquals(start, stop, hasStop, req.value_as_IntValue()->v(), list, isSorted);
        break;

        case Value_UIntValue:
          size = doRemoveIfEquals(start, stop, hasStop, req.value_as_UIntValue()->v(), list, isSorted);
        break;

        case Value_StringValue:
          size = doRemoveIfEquals(start, stop, hasStop, req.value_as_StringValue()->v()->str(), list, isSorted);
        break;

        case Value_FloatValue:
          size = doRemoveIfEquals(start, stop, hasStop, req.value_as_FloatValue()->v(), list, isSorted);
        break;

        default:
        {
          PLOGE << "List::remove_if value type not supported";
        }
        break;
        }
      
        const auto body = fc::response::CreateListRemoveIf(fbb, size);
        const auto rsp = fc::response::CreateResponse (fbb, status, ResponseBody_ListRemoveIf, body.Union());
        fbb.Finish(rsp);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, ResponseBody_ListRemoveIf);
    }
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


  void ListHandler::doAddAppend(  FlatBuilder& fbb, const std::string& name, const flexbuffers::TypedVector& items,
                                  const bool isAppend, const fc::request::Base base, const std::int64_t pos, const bool itemsSorted)
  {
    const fc::response::ResponseBody bodyType = isAppend ? ResponseBody_ListAppend : ResponseBody_ListAdd;

    try
    {
      fc::response::Status status = Status_Ok;

      if (const auto listOpt = haveList(fbb, name, bodyType); listOpt && items.size()) [[likely]]
      {
        const auto& fcList = (*listOpt)->second;
        std::size_t size = 0;

        switch (fcList->type())
        {
          case FlexType::FBT_VECTOR_INT:
          case FlexType::FBT_VECTOR_UINT:
          case FlexType::FBT_VECTOR_FLOAT:
          case FlexType::FBT_VECTOR_KEY:
            if (isAppend && !fcList->isSorted())
              size = std::visit(makeUnsortedAppend(items), fcList->list());
            else if (fcList->isSorted())
              size = std::visit(makeSortedAdd(items, base, itemsSorted), fcList->list());
            else
              size = std::visit(makeUnsortedAdd(items, base, pos), fcList->list());
          break;

          default:  [[unlikely]]
            PLOGE << "Unknown type for list: " << fcList->type();
            status = Status_Fail;
          break;
        }      

        if (isAppend)
        {
          const auto body = fc::response::CreateListAppend(fbb, size);
          const auto rsp = fc::response::CreateResponse (fbb, status, bodyType, body.Union());
          fbb.Finish(rsp);
        }
        else
        {
          const auto body = fc::response::CreateListAdd(fbb, size);
          const auto rsp = fc::response::CreateResponse (fbb, status, bodyType, body.Union());
          fbb.Finish(rsp);
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      createEmptyBodyResponse(fbb, Status_Fail, bodyType);
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

