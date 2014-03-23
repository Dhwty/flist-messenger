#ifndef FLIST_IUSERINTERFACE_H
#define FLIST_IUSERINTERFACE_H

class FSession;

class iUserInterface
{
public:
	virtual void setChatOperator(FSession *session, QString characteroperator, bool opstatus) = 0;
	
	virtual void addChannel(FSession *session, QString name, QString title) = 0;
	virtual void removeChannel(FSession *session, QString name) = 0;
	virtual void addChannelCharacter(FSession *session, QString channelname, QString charactername) = 0;
	virtual void removeChannelCharacter(FSession *session, QString channelname, QString charactername) = 0;
	virtual void setChannelOperator(FSession *session, QString channelname, QString charactername, bool opstatus) = 0;

};

#endif // FLIST_IUSERINTERFACE_H
