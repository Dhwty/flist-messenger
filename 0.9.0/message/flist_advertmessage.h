#ifndef FLIST_ADVERTMESSAGE_H
#define FLIST_ADVERTMESSAGE_H

#include "flist_message.h"

class FAdvertMessage : public FMessage
{
public:
    FAdvertMessage(FChannel* channel, FCharacter* character, QString& msg);
private:
    void parse();
    bool checkPing();
};

#endif // FLIST_ADVERTMESSAGE_H
