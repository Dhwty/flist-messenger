#ifndef FLIST_IUSERINTERFACE_H
#define FLIST_IUSERINTERFACE_H

class FSession;

class iUserInterface
{
public:
	virtual void setChatOperator(FSession *session, QString characteroperator, bool opstatus) = 0;
};

#endif // FLIST_IUSERINTERFACE_H
