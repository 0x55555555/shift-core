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
    ((Eks::ResourceDescription&)*this) += SResourceDescription::createForInstanceInformation<U>();
    return *this;
    }

  template <typename T> X_CONST_EXPR Eks::ResourceDescription createForInstanceInformation()
    {
    return Eks::ResourceDescriptionTypeHelper<typename T::EmbeddedInstanceInformation>::createFor();
    }
  };

}

#endif // SRESOURCEDESCRIPTION_H
