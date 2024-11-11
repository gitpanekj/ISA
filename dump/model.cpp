/**
 * @file model.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-03
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "Observer.hpp"
#include "exceptions.hpp"
#include "app_state.hpp"
#include "model.hpp"
#include <iostream>
#include <set>

namespace application
{

    int Model::run()
    {
        throw new exceptions::NotImplemented("int Model.int run()");
        /* TODO: implement business logic */

        return 0;
    }

    /* Observer related methods */
    void Model::addObserver(Observer::IObserver<SharedState> *observer)
    {
        this->observers.insert(observer);
    }
    void Model::removeObserver(Observer::IObserver<SharedState> *observer)
    {
        this->observers.erase(observer);
    }
    void Model::notify()
    {
        for (auto observer : this->observers)
        {
            observer->update(this->state);
        }
    }
}