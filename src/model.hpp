/**
 * @file model.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef MODEL_H
#define MODEL_H

#include "Observer.hpp"
#include "app_state.hpp"
#include <set>

namespace application
{
    /**
     * @brief Class implementing business logic of the application
     *
     * Inherits from the Observable design pattern so that a view
     * is automatically notified about model change without user interaction.
     *
     */
    class Model : public Observer::IObservable<SharedState>
    {
    public:
        /* Business logic implementation */

        /**
         * @brief Run the main loop of the business logic
         *
         * Runs on a separate thread
         *
         * @return int status code
         */
        int run();

        /* Observer related methods */
        void addObserver(Observer::IObserver<SharedState> *observer) override;
        void removeObserver(Observer::IObserver<SharedState> *observer) override;
        void notify() override;

    private:
        ///> Vector of observers to be notified on change
        std::set<Observer::IObserver<SharedState> *> observers;
        ///> Shared state
        SharedState state;
    };
}

#endif