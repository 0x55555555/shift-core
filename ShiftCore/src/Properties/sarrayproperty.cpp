#include "shift/Properties/sarrayproperty.h"
#include "shift/TypeInformation/styperegistry.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Changes/shandler.inl"

namespace Shift
{

namespace detail
{
template <typename T> class ArrayPropertyTraits : public PropertyBaseTraits
  {
public:
  // called by parent
  static void saveProperty( const Property* p_in, Saver &l)
    {
    // saves the data of the parent class (keeps connections)
    detail::PropertyBaseTraits::saveProperty(p_in, l);

    // cast the input property to an ArrayProperty
    const T* ptr = p_in->castTo<T>();
    xAssert(ptr);
    if(ptr)
      {
      writeValue(l, ptr->mData);
      }
    }

  static Property *loadProperty( PropertyContainer* p_in, Loader &l)
    {
    Property *prop = detail::PropertyBaseTraits::loadProperty(p_in, l);

    T* ptr = prop->uncheckedCastTo<T>();
    xAssert(ptr);
    if(ptr)
      {
      readValue(l, ptr->mData);
      }
    return prop;
    }

  static void assignProperty(const Property *from, Property *to)
    {
    const T *f = from->castTo<T>();
    T *t = to->castTo<T>();

    xAssert(f && t);
    if(f && t)
      {
      t->mData = f->mData;
      }
    }
  };
}

S_IMPLEMENT_PROPERTY(FloatArrayProperty, Shift)

void FloatArrayProperty::createTypeInformation(PropertyInformationTyped<FloatArrayProperty> *,
                                                const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(UIntArrayProperty, Shift)

void UIntArrayProperty::createTypeInformation(PropertyInformationTyped<UIntArrayProperty> *,
                                               const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Vector2ArrayProperty, Shift)

void Vector2ArrayProperty::createTypeInformation(PropertyInformationTyped<Vector2ArrayProperty> *,
                                                  const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Vector3ArrayProperty, Shift)

void Vector3ArrayProperty::createTypeInformation(PropertyInformationTyped<Vector3ArrayProperty> *,
                                                  const PropertyInformationCreateData &)
  {
  }

S_IMPLEMENT_PROPERTY(Vector4ArrayProperty, Shift)

void Vector4ArrayProperty::createTypeInformation(PropertyInformationTyped<Vector4ArrayProperty> *,
                                                  const PropertyInformationCreateData &)
  {
  }

}
