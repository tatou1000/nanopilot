// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: rpc-twoparty.capnp

#ifndef CAPNP_INCLUDED_a184c7885cdaf2a1_
#define CAPNP_INCLUDED_a184c7885cdaf2a1_

#include <capnp/generated-header-support.h>

#if CAPNP_VERSION != 6001
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(9fd69ebc87b9719c);
enum class Side_9fd69ebc87b9719c: uint16_t {
  SERVER,
  CLIENT,
};
CAPNP_DECLARE_ENUM(Side, 9fd69ebc87b9719c);
CAPNP_DECLARE_SCHEMA(d20b909fee733a8e);
CAPNP_DECLARE_SCHEMA(b88d09a9c5f39817);
CAPNP_DECLARE_SCHEMA(89f389b6fd4082c1);
CAPNP_DECLARE_SCHEMA(b47f4979672cb59d);
CAPNP_DECLARE_SCHEMA(95b29059097fca83);
CAPNP_DECLARE_SCHEMA(9d263a3630b7ebee);

}  // namespace schemas
}  // namespace capnp

namespace capnp {
namespace rpc {
namespace twoparty {

typedef ::capnp::schemas::Side_9fd69ebc87b9719c Side;

struct VatId {
  VatId() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(d20b909fee733a8e, 1, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct ProvisionId {
  ProvisionId() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(b88d09a9c5f39817, 1, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct RecipientId {
  RecipientId() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(89f389b6fd4082c1, 0, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct ThirdPartyCapId {
  ThirdPartyCapId() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(b47f4979672cb59d, 0, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct JoinKeyPart {
  JoinKeyPart() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(95b29059097fca83, 1, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct JoinResult {
  JoinResult() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(9d263a3630b7ebee, 1, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class VatId::Reader {
public:
  typedef VatId Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::capnp::rpc::twoparty::Side getSide() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class VatId::Builder {
public:
  typedef VatId Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::capnp::rpc::twoparty::Side getSide();
  inline void setSide( ::capnp::rpc::twoparty::Side value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class VatId::Pipeline {
public:
  typedef VatId Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class ProvisionId::Reader {
public:
  typedef ProvisionId Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getJoinId() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class ProvisionId::Builder {
public:
  typedef ProvisionId Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getJoinId();
  inline void setJoinId( ::uint32_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class ProvisionId::Pipeline {
public:
  typedef ProvisionId Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class RecipientId::Reader {
public:
  typedef RecipientId Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class RecipientId::Builder {
public:
  typedef RecipientId Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class RecipientId::Pipeline {
public:
  typedef RecipientId Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class ThirdPartyCapId::Reader {
public:
  typedef ThirdPartyCapId Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class ThirdPartyCapId::Builder {
public:
  typedef ThirdPartyCapId Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class ThirdPartyCapId::Pipeline {
public:
  typedef ThirdPartyCapId Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class JoinKeyPart::Reader {
public:
  typedef JoinKeyPart Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getJoinId() const;

  inline  ::uint16_t getPartCount() const;

  inline  ::uint16_t getPartNum() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class JoinKeyPart::Builder {
public:
  typedef JoinKeyPart Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getJoinId();
  inline void setJoinId( ::uint32_t value);

  inline  ::uint16_t getPartCount();
  inline void setPartCount( ::uint16_t value);

  inline  ::uint16_t getPartNum();
  inline void setPartNum( ::uint16_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class JoinKeyPart::Pipeline {
public:
  typedef JoinKeyPart Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class JoinResult::Reader {
public:
  typedef JoinResult Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getJoinId() const;

  inline bool getSucceeded() const;

  inline bool hasCap() const;
  inline ::capnp::AnyPointer::Reader getCap() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class JoinResult::Builder {
public:
  typedef JoinResult Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getJoinId();
  inline void setJoinId( ::uint32_t value);

  inline bool getSucceeded();
  inline void setSucceeded(bool value);

  inline bool hasCap();
  inline ::capnp::AnyPointer::Builder getCap();
  inline ::capnp::AnyPointer::Builder initCap();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class JoinResult::Pipeline {
public:
  typedef JoinResult Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline  ::capnp::rpc::twoparty::Side VatId::Reader::getSide() const {
  return _reader.getDataField< ::capnp::rpc::twoparty::Side>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::capnp::rpc::twoparty::Side VatId::Builder::getSide() {
  return _builder.getDataField< ::capnp::rpc::twoparty::Side>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void VatId::Builder::setSide( ::capnp::rpc::twoparty::Side value) {
  _builder.setDataField< ::capnp::rpc::twoparty::Side>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline  ::uint32_t ProvisionId::Reader::getJoinId() const {
  return _reader.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::uint32_t ProvisionId::Builder::getJoinId() {
  return _builder.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void ProvisionId::Builder::setJoinId( ::uint32_t value) {
  _builder.setDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline  ::uint32_t JoinKeyPart::Reader::getJoinId() const {
  return _reader.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::uint32_t JoinKeyPart::Builder::getJoinId() {
  return _builder.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void JoinKeyPart::Builder::setJoinId( ::uint32_t value) {
  _builder.setDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline  ::uint16_t JoinKeyPart::Reader::getPartCount() const {
  return _reader.getDataField< ::uint16_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS);
}

inline  ::uint16_t JoinKeyPart::Builder::getPartCount() {
  return _builder.getDataField< ::uint16_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS);
}
inline void JoinKeyPart::Builder::setPartCount( ::uint16_t value) {
  _builder.setDataField< ::uint16_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS, value);
}

inline  ::uint16_t JoinKeyPart::Reader::getPartNum() const {
  return _reader.getDataField< ::uint16_t>(
      ::capnp::bounded<3>() * ::capnp::ELEMENTS);
}

inline  ::uint16_t JoinKeyPart::Builder::getPartNum() {
  return _builder.getDataField< ::uint16_t>(
      ::capnp::bounded<3>() * ::capnp::ELEMENTS);
}
inline void JoinKeyPart::Builder::setPartNum( ::uint16_t value) {
  _builder.setDataField< ::uint16_t>(
      ::capnp::bounded<3>() * ::capnp::ELEMENTS, value);
}

inline  ::uint32_t JoinResult::Reader::getJoinId() const {
  return _reader.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::uint32_t JoinResult::Builder::getJoinId() {
  return _builder.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void JoinResult::Builder::setJoinId( ::uint32_t value) {
  _builder.setDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline bool JoinResult::Reader::getSucceeded() const {
  return _reader.getDataField<bool>(
      ::capnp::bounded<32>() * ::capnp::ELEMENTS);
}

inline bool JoinResult::Builder::getSucceeded() {
  return _builder.getDataField<bool>(
      ::capnp::bounded<32>() * ::capnp::ELEMENTS);
}
inline void JoinResult::Builder::setSucceeded(bool value) {
  _builder.setDataField<bool>(
      ::capnp::bounded<32>() * ::capnp::ELEMENTS, value);
}

inline bool JoinResult::Reader::hasCap() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool JoinResult::Builder::hasCap() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline ::capnp::AnyPointer::Reader JoinResult::Reader::getCap() const {
  return ::capnp::AnyPointer::Reader(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline ::capnp::AnyPointer::Builder JoinResult::Builder::getCap() {
  return ::capnp::AnyPointer::Builder(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline ::capnp::AnyPointer::Builder JoinResult::Builder::initCap() {
  auto result = ::capnp::AnyPointer::Builder(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
  result.clear();
  return result;
}

}  // namespace
}  // namespace
}  // namespace

#endif  // CAPNP_INCLUDED_a184c7885cdaf2a1_
