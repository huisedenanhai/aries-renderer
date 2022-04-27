The library is RTTR official release 0.9.6, with following modifications

+ [number_conversion](src/rttr/detail/conversion/number_conversion.h) Remove value range check for number conversion.
  Always do static cast for arithmetic types.