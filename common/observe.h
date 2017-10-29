#ifndef _MIYUKI_OBSERVE_H
#define _MIYUKI_OBSERVE_H

#include "common/ptrdef.h"
#include <vector>

// Observer pattern interface

namespace Miyuki::Common {

    DEFINE_RAW_PTR(IObservable)
    DEFINE_RAW_PTR(IObserver)

    class IObserver {
    public:
        virtual void notify(uint32_t what) = 0;
    };

    class IObservable {
        std::vector<IObserverPtr> observers;
    public:
        void addObserver(IObserverPtr o) { observers.push_back(o); }
        bool removeObserver(IObserverPtr& o) {
            for (std::vector<IObserverPtr>::iterator it = observers.begin();
                    it != observers.end(); ++it)
                if ( *it == o )  { observers.erase(it); return true; }
            return false;
        }
        void notifyAll(uint32_t what) {
            for (std::vector<IObserverPtr>::iterator it = observers.begin(); it != observers.end(); ++it)
                (*it)->notify(what);
        }
    };
}

#endif