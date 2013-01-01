#ifndef SRESOURCEDESCRIPTION_H
#define SRESOURCEDESCRIPTION_H

#include "XGlobal"
#include "XResourceDescription"

namespace Shift
{

class SResourceDescription : public Eks::ResourceDescription
  {
public:
  template <typename U, typename PropType>
  SResourceDescription &operator+=(U PropType::* ptr)
    {
    (void)ptr;
    ((XResourceDescription&)*this) += SResourceDescription::createForInstanceInformation<U>();
    return *this;
    }

  template <typename T> X_CONST_EXPR Eks::ResourceDescription createForInstanceInformation()
    {
    return Eks::ResourceDescriptionTypeHelper::createFor<T::EmbeddedInstanceInformation>();
    }
  };

}

#endif // SRESOURCEDESCRIPTION_H
