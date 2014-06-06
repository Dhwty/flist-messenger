#include "usereturn.h"
#include "flist_messenger.h"

bool UseReturn::eventFilter ( QObject* obj, QEvent* event )
{
	if ( event->type() == QEvent::KeyPress )
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );
		if (keyEvent->modifiers() == Qt::ShiftModifier)
		{
			switch ( keyEvent->key() )
			{
			case Qt::Key_Enter:
			case Qt::Key_Return:
				static_cast<flist_messenger*>(parent())->insertLineBreak();
				return true;
				break;
			default:
				return QObject::eventFilter( obj, event);
			}
		}
		else
		{
			switch ( keyEvent->key() )
			{
			case Qt::Key_Enter:

			case Qt::Key_Return:
				static_cast<flist_messenger*> ( parent() )->enterPressed();
				return true;
				break;
			default:
				return QObject::eventFilter ( obj, event );
			}
		}
	}
	else
	{
		return QObject::eventFilter ( obj, event );
	}
}

ReturnFilter::ReturnFilter(QObject *parent) : QObject(parent) { }

bool ReturnFilter::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() != QEvent::KeyPress) { return QObject::eventFilter(obj, event); }

	QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );
	switch ( keyEvent->key() )
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
		emit enter(keyEvent->modifiers());

		switch(keyEvent->modifiers())
		{
		case Qt::ControlModifier:	emit controlEnter(); break;
		case Qt::ShiftModifier:   emit shiftEnter();   break;
		default:                  emit plainEnter();   break;
		}

		return true;

	default:
		return QObject::eventFilter(obj, event);
	}
}
