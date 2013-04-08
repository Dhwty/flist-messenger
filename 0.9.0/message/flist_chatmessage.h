#ifndef FLIST_CHATMESSAGE_H
#define FLIST_CHATMESSAGE_H

#include"flist_message.h"
#include"../flist_channel.h"

class FChannel;

class FChatMessage : public FMessage
{
public:
    FChatMessage(FChannel* channel, FCharacter* character, QString& msg);
private:
    void parse();
    bool checkPing();
};

#endif // FLIST_CHATMESSAGE_H
