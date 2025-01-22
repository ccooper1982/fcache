#include <fc/ListHandler.hpp>
#include <fc/List.hpp>


namespace fc
{
  using enum fc::common::ListType;
  using enum fc::response::Status;


  void ListHandler::handle(FlatBuilder& fbb, const fc::request::ListCreate& req) noexcept
  {
    fc::response::Status status = Status_Ok;

    if (req.type() == ListType_Int)
    {
      PLOGD << "ListCreate: creating Int list";
      if (const auto [_, created] = m_lists.try_emplace(req.name()->str(), new FcList{FlexType::FBT_VECTOR_INT, IntList{}}); !created)
        status = Status_Duplicate;
    }
    else if (req.type() == ListType_UInt)
    {
      PLOGD << "ListCreate: creating UInt list";
      if (const auto [_, created] = m_lists.try_emplace(req.name()->str(), new FcList{FlexType::FBT_VECTOR_UINT, UIntList{}}); !created)
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
    const auto& name = req.name()->str();

    if (const auto listOpt = getList(name); !listOpt)
    {
      createEmptyBodyResponse(fbb, Status_Fail, fc::response::ResponseBody_ListAdd);
    }
    else
    {
      const auto& itemsVector = req.items_flexbuffer_root().AsTypedVector();
      const auto list = (*listOpt)->second;

      auto add = [&itemsVector, list = list->list()]<typename T>()
      {
        for (std::size_t i = 0 ; i < itemsVector.size() ; ++i)
          std::visit(AddHead{itemsVector[i].As<T>()}, list);
      };

      switch (list->type())
      {
        case FlexType::FBT_VECTOR_INT:
          add.template operator()<std::int64_t>();
        break;

        case FlexType::FBT_VECTOR_UINT:
          add.template operator()<std::uint64_t>();
        break;

        default:
        break;
      }

      createEmptyBodyResponse(fbb, Status_Ok, fc::response::ResponseBody_ListAdd);
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