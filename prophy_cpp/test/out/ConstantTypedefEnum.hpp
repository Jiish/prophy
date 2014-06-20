#ifndef _PROPHY_GENERATED_ConstantTypedefEnum_HPP
#define _PROPHY_GENERATED_ConstantTypedefEnum_HPP

#include <prophy/prophy.hpp>

enum { CONSTANT = 3 };

typedef uint16_t TU16;

enum Enum
{
    Enum_One = 1
};

struct ConstantTypedefEnum
{
    uint16_t a[CONSTANT];
    TU16 b;
    Enum c;
};

namespace prophy
{

template <>
inline ConstantTypedefEnum* swap<ConstantTypedefEnum>(ConstantTypedefEnum* payload)
{
    swap_n_fixed(payload->a, CONSTANT);
    swap(&payload->b);
    swap(reinterpret_cast<int*>(&payload->c));
    return payload + 1;
}

} // namespace prophy

#endif  /* _PROPHY_GENERATED_ConstantTypedefEnum_HPP */