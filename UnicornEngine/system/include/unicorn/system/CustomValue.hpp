/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#ifndef UNICORN_SYSTEM_CUSTOM_VALUE_HPP
#define UNICORN_SYSTEM_CUSTOM_VALUE_HPP

#include <cstdint>

namespace unicorn
{
namespace system
{

/** @brief  Custom values that can be understood by window manager subsystem */
struct CustomValue
{
    //! Mask to check for custom value
    static const int32_t Mask = 0x7FFFFF00;

    //! @{
    //! Bool values
    static const int32_t False = Mask | 0x00;
    static const int32_t True = Mask | 0x01;
    //! @}

    //! @{
    //! ClientAPI
    static const int32_t OpenGL_API = Mask | 0x10;
    static const int32_t OpenGL_ES_API = Mask | 0x11;
    static const int32_t No_API = Mask | 0x12;
    //! @}

    //! @{
    //! ContextCreationAPI
    static const int32_t NativeContextAPI = Mask | 0x20;
    static const int32_t EGLContextAPI = Mask | 0x21;
    //! @}

    //! @{
    //! Global values
    static const int32_t DontCare = Mask | 0xFF;
    //! @}
};

}
}

#endif /* UNICORN_SYSTEM_CUSTOM_VALUE_HPP */
