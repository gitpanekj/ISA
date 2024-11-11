/**
 * @file controller.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "exceptions.hpp"
#include "model.hpp"
#include "view.hpp"

namespace application
{
    /**
     * @brief User input handling
     *
     */
    class Controller
    {
    public:
        Controller(Model &m, View &v);

        /**
         * @brief Handle user input
         *
         * Runs in separate thread
         *
         * @return int status code
         */
        int handle_user_input();

        /**
         * @brief Application runner
         *
         * @return int status code
         */
        int run_app();

    private:
        Model &model;
        View &view;
    };
}

#endif