/**
 * @file view.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef VIEW_H
#define VIEW_H

#include "Observer.hpp"
#include "app_state.hpp"

namespace application {
    /**
     * @brief Data presentation
     *
     */
    class View : public Observer::IObserver<SharedState>
    {
    public:
        /* View related functionality */
        View();
        ~View();
        
        void display();

        /* Observer related functionality */
        void update(const SharedState &state) override;
    };
}

#endif