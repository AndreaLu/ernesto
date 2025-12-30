#ifndef NETWORK_H
#define NETWORK_H

namespace NETW {
    void Init(); // blocking till connected to the WiFi
    bool SignalCompletion(); // returns true on success
}

#endif