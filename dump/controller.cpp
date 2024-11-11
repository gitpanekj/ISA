/**
 * @file controller.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "exceptions.hpp"
#include "app_state.hpp"
#include "model.hpp"
#include "view.hpp"
#include "controller.hpp"


namespace application
{

    Controller::Controller(Model &m, View &v) : model(m), view(v)
    {
        m.addObserver(&v); // let model notify view about the state changes
    }

    int Controller::handle_user_input()
    {
        throw new exceptions::NotImplemented("int Controller.handle_user_input()");
        /* TODO: implement user input handling in a single thread */

        return 0;
    }

    int Controller::run_app()
    {
        // Step1 Run model - thread no. 1
        // Step2 Run user input - thread no. 2

        // Wait both/one ?? - wait until one of the threads end
        // in practice, only termination of the user input thread can cause
        // the application end.

        return 0;
    }
}