#ifndef USERETURN_H
#define USERETURN_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>

class UseReturn : public QObject
{
	Q_OBJECT

public:
	UseReturn ( QObject* parent )
	{
		setParent ( parent );
	}

protected:
	bool eventFilter ( QObject *obj, QEvent *event );
};

class ReturnFilter : public QObject
{
	Q_OBJECT

public:
	ReturnFilter(QObject* parent);

signals:
	void controlEnter();
	void shiftEnter();
	void plainEnter();
	void enter(Qt::KeyboardModifiers modifiers);

protected:
	bool eventFilter ( QObject *obj, QEvent *event );
};

#endif // USERETURN_H
