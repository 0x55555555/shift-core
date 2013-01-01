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
  static void createTypeInformation(Shift::PropertyInformationTyped<name> *info, \
    const Shift::PropertyInformationCreateData &data); \
  static const Shift::PropertyInformation *staticTypeInformation(); \
  static const Shift::PropertyInformation *bootstrapStaticTypeInformation();

#define S_ADD_INSTANCE_INFORMATION(name) \
  inline const EmbeddedInstanceInformation *embeddedInstanceInformation() const \
    { return static_cast<const EmbeddedInstanceInformation *>(embeddedBaseInstanceInformation()); } \
  inline const DynamicInstanceInformation *dynamicInstanceInformation() const \
    { return static_cast<const DynamicInstanceInformation *>(dynamicBaseInstanceInformation()); }

#define S_ADD_STATIC_INFO(name, version) \
  public: enum { Version = version, IsAbstract = false };

#define S_ADD_ABSTRACT_STATIC_INFO(name, version) \
  public: enum { Version = version, IsAbstract = true };

#define S_PROPERTY_ROOT(myName, version) \
  public: \
  S_ADD_STATIC_INFO(myName, version); \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef void ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

#define S_PROPERTY(myName, superName, version) \
  public: \
  S_ADD_STATIC_INFO(myName, version) \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef superName ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

#define S_ABSTRACT_PROPERTY(myName, superName, version) \
  public: \
  S_ADD_ABSTRACT_STATIC_INFO(myName, version) \
  S_ADD_INSTANCE_INFORMATION(myName) \
  typedef superName ParentType; \
  S_REGISTER_TYPE_FUNCTION(myName)

// its possible we might want to not use a handler globally, and just apply all changes directly.
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
#endif

#define PropertyDoChange(type, ...) PropertyDoChangeNonLocal(type, this, __VA_ARGS__)

#endif // SPROPERTYMACROS_H
