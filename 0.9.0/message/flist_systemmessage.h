#ifndef FLIST_SYSTEMMESSAGE_H
#define FLIST_SYSTEMMESSAGE_H

#include "flist_message.h"

class FSystemMessage : public FMessage
{
public:
    enum SystemMessageType
    {
        SYSTYPE_JOIN, // Leave/join messages
        SYSTYPE_ONLINE, // online/offline/status messages
        SYSTYPE_KICKBAN, // Kicks or bans
        SYSTYPE_ADDOP, // Adding/removing OPs, channel OPs, etc.
        SYSTYPE_FEEDBACK, // Feedback about the user having done something.
        SYSTYPE_DICE, // Rolling dice, etc.
        SYSTYPE_MAX
    };
    FSystemMessage(SystemMessageType sysType, FChannel* channel, QString& msg);
private:
    void parse();
    bool checkPing();
    SystemMessageType systemType;
};

#endif // FLIST_SYSTEMMESSAGE_H
