#ifndef SRESOURCEDESCRIPTION_H
#define SRESOURCEDESCRIPTION_H

#include "XGlobal"
#include "XResourceDescription"

namespace Shift
{

class ResourceDescription : public Eks::ResourceDescription
  {
public:
  template <typename U, typename PropType>
  ResourceDescription &operator+=(U PropType::* ptr)
    {
    (void)ptr;
    ((Eks::ResourceDescription&)*this) += ResourceDescription::createForInstanceInformation<U>();
    return *this;
    }

  template <typename T> X_CONST_EXPR Eks::ResourceDescription createForInstanceInformation()
    {
    return Eks::ResourceDescriptionTypeHelper<typename T::EmbeddedInstanceInformation>::createFor();
    }
  };

}

#endif // SRESOURCEDESCRIPTION_H
