#ifndef SPROPERTYMACROS_H
#define SPROPERTYMACROS_H

#ifdef S_PROPERTY_USER_DATA
#define S_USER_DATA_TYPE(typeId) public: \
  enum { UserDataType = SUserDataTypes::typeId }; \
  virtual xuint32 userDataTypeId() const { return UserDataType; } \
  private:
#endif

#define S_REGISTER_TYPE_FUNCTION(name) \
  public: \
  static Shift::Module &module(); \
  static void createTypeInformation(Shift::PropertyInformationTyped<name> *info, \
    const Shift::PropertyInformationCreateData &data); \
  static const Shift::PropertyInformation *staticTypeInformation(); \
  static const Shift::PropertyInformation *bootstrapStaticTypeInformation(Eks::AllocatorBase *alloc);

#define S_ADD_INSTANCE_INFORMATION(name) \
  inline const EmbeddedInstanceInformation *embeddedInstanceInformation() const \
    { return static_cast<const EmbeddedInstanceInformation *>(Attribute::embeddedBaseInstanceInformation()); } \
  inline const DynamicInstanceInformation *dynamicInstanceInformation() const \
    { return static_cast<const DynamicInstanceInformation *>(Attribute::dynamicBaseInstanceInformation()); }

#define S_ADD_STATIC_INFO(name) \
  public: enum { IsAbstract = false };

#define S_ADD_ABSTRACT_STATIC_INFO(name) \
  public: enum { IsAbstract = true };

#define S_PROPERTY_ROOT(myName) \
  public: \
  S_ADD_STATIC_INFO(myName); \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef void ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

#define S_PROPERTY(myName, superName) \
  public: \
  S_ADD_STATIC_INFO(myName) \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef superName ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

#define S_ABSTRACT_PROPERTY(myName, superName) \
  public: \
  S_ADD_ABSTRACT_STATIC_INFO(myName) \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef superName ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

// its possible we might want to not use a handler globally, and just apply all changes directly.
#define S_CENTRAL_CHANGE_HANDLER
#ifdef S_CENTRAL_CHANGE_HANDLER
# define PropertyDoChangeNonLocal(type, ths, ...) {\
  Handler *hand = ths->handler(); \
  hand->doChange<type>(__VA_ARGS__); \
  }
#else
# define PropertyDoChangeNonLocal(type, ths, ...) {\
  type change(__VA_ARGS__); \
  ((Change&)change).apply(); \
  }

// todo: informing when there is no handler???
//((Change&)change).inform(false);
#endif

#define PropertyDoChange(type, ...) PropertyDoChangeNonLocal(type, this, __VA_ARGS__)

#endif // SPROPERTYMACROS_H
