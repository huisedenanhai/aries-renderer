/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014 - 2018 Axel Menzel <info@rttr.org>                           *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef RTTR_NUMBER_CONVERSION_H_
#define RTTR_NUMBER_CONVERSION_H_

#include "rttr/detail/base/core_prerequisites.h"
#include <limits>

namespace rttr
{
namespace detail
{

/////////////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Identity function
 *
 */
template<typename F, typename T>
typename std::enable_if<std::is_same<F, T>::value, bool>::type
convert_to(const F& from, T& to)
{
    to = from;
    return true;
}

template <typename F, typename T>
typename std::enable_if<std::is_arithmetic<F>::value &&
                            std::is_arithmetic<T>::value,
                        bool>::type
convert_to(const F &from, T &to) {
    to = static_cast<T>(from);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// string conversion

template<typename F, typename T>
typename std::enable_if<std::is_same<T, std::string>::value,
                        bool>::type
convert_to(const F& from, T& to)
{
    bool ok = false;
    to = to_string(from, &ok);
    return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////

} // end namespace detail
} // end namespace rttr

#endif // RTTR_NUMBER_CONVERSION_H_
