#ifndef FLIST_REPORTMESSAGE_H
#define FLIST_REPORTMESSAGE_H

#include "flist_message.h"

class FReportMessage : public FMessage
{
public:
    FReportMessage(FCharacter* character, QString& msg);
private:
    void parse();
    bool checkPing();
};

#endif // FLIST_REPORTMESSAGE_H
