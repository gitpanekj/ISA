/**
 * @file exceptions.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <iostream>

namespace exceptions {

class NotImplemented : public std::logic_error
{
public:
    NotImplemented(const std::string function_name) : std::logic_error("Function not implemented: " + function_name) {}
};
}
#endif