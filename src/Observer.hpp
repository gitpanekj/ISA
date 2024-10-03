/**
 * @file Observer.h
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief 
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef OBSERVER_H
#define OBSERVER_H

/**
 * @brief Observer design pattern
 *
 */
namespace Observer
{
    /**
     * @brief Templated observer abstract class
     *
     * @tparam ObservableState type of shared state
     */
    template <typename ObservableState>
    class IObserver
    {
    public:
        /**
         * @brief Update the observer state after observable changes
         *
         * @param state state to be shared with observers
         */
        virtual void update(const ObservableState &state) = 0;
    };

    /**
     * @brief Templated observable abstract class
     *
     * @tparam ObservableState type of shared state
     */
    template <typename ObservableState>
    class IObservable
    {
    public:
        /**
         * @brief Add observer to the list of observers who are notified about observable state change
         *
         * @param observer Observer to be added
         */
        virtual void addObserver(IObserver<ObservableState> *observer) = 0;

        /**
         * @brief Remove observer from the list of observers who are notified about observable state change
         *
         * @param observer Observer to be removed
         */
        virtual void removeObserver(IObserver<ObservableState> *observer) = 0;

        /**
         * @brief Notify all the registered observers about the a change
         *
         */
        virtual void notify() = 0;
    };
}

#endif