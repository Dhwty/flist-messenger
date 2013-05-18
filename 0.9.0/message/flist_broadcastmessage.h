#ifndef FLIST_BROADCASTMESSAGE_H
#define FLIST_BROADCASTMESSAGE_H

#include "flist_message.h"

class FBroadcastMessage : public FMessage
{
public:
    FBroadcastMessage(QString& msg);
private:
    void parse();
    bool checkPing();
};

#endif // FLIST_BROADCASTMESSAGE_H
