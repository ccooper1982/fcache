#include <fc/ListHandler.hpp>
#include <fc/List.hpp>
#include <fc/Common.hpp>


namespace fc
{
  using enum fc::common::ListType;
  using enum fc::response::Status;


  #ifdef FC_DEBUG
  template<typename T>
  static void dumpList(List& list)
  {
    const auto& l = std::get<T>(list);

    for (const auto n : l)
      PLOGD << n;
  }
  #endif


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

    if (const auto listOpt = getList(name); !listOpt)
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