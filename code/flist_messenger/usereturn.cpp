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

bool ReturnLogin::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch(keyEvent->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			static_cast<flist_messenger*>(parent())->connectClicked();
		default:
			return QObject::eventFilter(obj, event);
		}
	} else {
		return QObject::eventFilter(obj, event);
	}
}
