#ifndef SENTITY_H
#define SENTITY_H

#include "shift/sglobal.h"
#include "shift/Properties/scontainer.h"
#include "shift/Properties/sset.h"
#include "shift/Changes/sobserver.h"
#include "Memory/XWeakShared.h"

namespace Shift
{

#define S_ENTITY(name, parent) S_PROPERTY_CONTAINER(name, parent)
#define S_ABSTRACT_ENTITY(name, parent) S_ABSTRACT_PROPERTY_CONTAINER(name, parent)

/** \brief An item in a database.

  \section Introduction
  An Entity represents a chunk of data in a database, it can hold its own data,
  and other Entities, allowing a tree structure to be built.

  \section Deriving from Entity
  Conceptually (this is not enforced), an Entity should embed (see below) Property and PropertyContainers types,
  and hold Entity types in its Entity::children array.

  \sa PropertyArray

  To create your own Entity type, a new class should be implemented, which publically inherits from Entity.
  The class should contain either the S_ENTITY, or S_ABSTRACT_ENTITY as its first line. This macro declares
  some required functions. The new class should also have a S_PROPERTY_INTERFACE(NewClass) after its
  declaration, in the global namespace. This macro ensures the class is reflectable in scripting integrations.
  An Entity type must also have a S_IMPLEMENT_PROPERTY(NewClass, Module) definition in the cpp file, and
  a method NewClass::createTypeInformation(...) defined:

  \code
  // this adds NewClass to CustomModule, see PropertyGroup, for information on groups.
  S_IMPLEMENT_PROPERTY(NewClass, CustomGroup)

  void NewEntity::createTypeInformation(PropertyInformationTyped<NewEntity> *info,
                                     const PropertyInformationCreateData &data)
   {
   // if the create data asks for attribute registrations, then build up the attribute list.
   // Note parent type information is copied by default, but can be overridden.
   if(data.registerAttributes)
     {
     // Creating this childBlock on the stack managed the new children, and pushes them into info when destroyed.
     auto childBlock = info->createChildrenBlock(data);

     // This informs the type that it has a child at the location &NewEntity::member, called "member".
     // the return value is a helper class derived from EmbeddedInstanceInformation, which allows setting up type specific
     // computation, and affectors.
     auto *childInst = childBlock.add(&NewEntity::children, "member");
     }

   // If the create data asks for interface registrations, then build up the interface list.
   // Note, the parent class interface is automatically used by default, so a new interface is rarely required.
   if(data.registerInterfaces)
     {
     // See Script Interfaces section
     }
   }
  \endcode

  \todo Ensure the S_PROPERTY_INTERFACE is optional, when scripting isn't required.
  \todo registerAttributes should really be registerProperties, but are either required anymore?
  \sa EmbeddedPropertyInstanceInformation
  \sa PropertyGroup
  \sa S_ENTITY S_ABSTRACT_ENTITY

  \section Dynamic Properties
  A property can hold any number of embedded properties, but an entity can also have dynamic data added to it at
  runtime. This dynamic data is less efficient, and less capable (it cannot be computed, or affect computed data),
  but useful for Entity types that must be dynamically created. An alternative to Dynamic properties, may be to
  Dynamically design Entity Types (see below)

  \section Script Interfaces
  Any Property class in Shift has its own script interface, and also inherits any interface supplied by its parent.
  Any useful functions can be exposed by the entity by using the supplied interface:

  \code
    // In the createTypeInformation function for the new type:
    XScript::Interface<NewEntity> *api = info->apiInterface();
  \code

  \section Dynamic Entity Types
  It is possible to dynamically create an Entity type by calling derive() on an existing PropertyInformation, and adding
  new embedded properties to the class. This method is considerably more efficient than using dynamic properties when
  more than one of the new Entities is required and allows computation of members.

  \sa PropertyInformation PropertyInformation::derive

 */
class SHIFT_EXPORT Entity : public Container, public XWeakSharedData
  {
  S_ENTITY(Entity, Container);

public:
  enum
    {
    StaticChildMode = Container::StaticChildMode | Shift::NamedChildren,
    DynamicChildMode = Container::DynamicChildMode | Shift::NamedChildren
    };

  Entity();
  ~Entity();

  /// Used to hold dynamic child entities, allowing dynamic tree structures to be built.
  Set children;

  /// Move this Entity under another Entity (will actually move from one children member to another children member)
  void reparent(Entity *);

  /// \defgroup addChild Adding children
  /// @{
  template <typename T>T *addChild(const NameArg& name="")
    {
    Attribute *p = addChild(T::staticTypeInformation(), name);
    xAssert(p);
    return p->uncheckedCastTo<T>();
    }

  Attribute *addChild(const PropertyInformation *info, const NameArg& name=NameArg());
  /// @}

  /// \defgroup addAttribute Adding dynamic properties
  /// @{
  template <typename T>T *addAttribute(
      xsize index = std::numeric_limits<xsize>::max(),
      const NameArg& name="",
      PropertyInstanceInformationInitialiser *init=0)
    {
    Attribute *p = addAttribute(T::staticTypeInformation(), index, name, init);
    xAssert(p);

    return p->uncheckedCastTo<T>();
    }

  Attribute *addAttribute(
    const PropertyInformation *info,
    xsize index = std::numeric_limits<xsize>::max(),
    const NameArg& name=NameArg(),
    PropertyInstanceInformationInitialiser *inst=0);
  /// @}

  /// Remove a dynamic property
  void removeAttribute(Property *prop)
    {
    Container::removeAttribute(prop);
    }

  /// Move a dynamic property to a different Entity
  void moveAttribute(Entity *newParent, Attribute *prop)
    {
    Container::moveAttribute(newParent, prop);
    }

  /// Accessing child properties by index
  /// \note children is the first member, then any other embedded properties, then dynamic properties.
  using Container::at;

  /// \defgroup parentEntity Find the parent entity, normally equivalent to parent()->parent()
  /// \note If the entity is embedded in a different parent than children, the function will return the first encountered Entity ancestor.
  /// @{
  const Entity *parentEntity() const;
  Entity *parentEntity();
  /// @}

  /// \defgroup Observers Add and remove Observers of this Entity.
  /// @{
  void addTreeObserver(TreeObserver *);
  void addConnectionObserver(ConnectionObserver *);

  void removeTreeObserver(TreeObserver *);
  void removeConnectionObserver(ConnectionObserver *);
  void removeObserver(Observer *);

  void informTreeObservers(const Change *event, bool backwards);
  void informConnectionObservers(const Change *event, bool backwards);
  /// @}

  /// \defgroup findChildEntity Find child Entities by name (searches children)
  /// @{
  Entity *findChildEntity(const NameArg &);
  const Entity *findChildEntity(const NameArg &) const;
  /// @}

#ifdef S_CENTRAL_CHANGE_HANDLER
  /// \brief find the change handler for this entity
  Handler *handler() { return _handler; }
#else
  Database *_database;
#endif

private:
  struct ObserverStruct
    {
    enum
      {
      Dirty,
      Tree,
      Connection
      };
    xuint8 mode;
    void *observer;

    Observer *getObserver();
    };
  Eks::Vector<ObserverStruct> _observers;

#ifdef S_CENTRAL_CHANGE_HANDLER
  Handler *_handler;
#endif

  void setupObservers();

  friend class Database;
  friend class Container;
  };

}

S_PROPERTY_INTERFACE(Shift::Entity)

#endif // SENTITY_H
