#include "helpdialog.h"
#include "flist_character.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextBrowser>

namespace Ui
{
class FHelpDialog
{

	QVBoxLayout *vbl;
	QTabWidget *tabs;
	QTextBrowser *commands, *admin, *colors, *bbcode;
	QDialogButtonBox *buttons;
	QPushButton *closebutton;

	QTextBrowser *addTab(QString name, QString content)
	{
		QTextBrowser *te = new QTextBrowser();
		tabs->addTab(te, name);
		te->setHtml(content);
		te->viewport()->setAutoFillBackground(false);
		te->setFrameStyle(QFrame::NoFrame);
		return te;
	}

public:
	void setupUi(QDialog *dialog)
	{
		dialog->setWindowTitle("Help");
		dialog->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

		vbl = new QVBoxLayout(dialog);

		tabs = new QTabWidget(dialog);

		QString str = "/me <message><br />";
		str +=        "/join &lt;channel&gt;<br />";
		str +=        "/priv &lt;character&gt;<br />";
		str +=        "/ignore &lt;character&gt;<br />";
		str +=        "/unignore &lt;character&gt;<br />";
		str +=        "/ignorelist<br />";
		str +=        "/code<br />";
		str +=        "/roll &lt;1d10&gt; (WIP)<br />";
		str +=        "/status &lt;Online|Looking|Busy|DND&gt; &lt;optional message&gt;<br />";
		str +=        "<b>Channel owners:</b><br />";
		str +=        "/makeroom &lt;name&gt;<br />";
		str +=        "/invite &lt;person&gt;<br />";
		str +=        "/openroom<br />";
		str +=        "/closeroom<br />";
		str +=        "/setdescription &lt;description&gt;<br />";
		str +=        "/getdescription<br />";
		str +=        "/setmode &lt;chat|ads|both&gt;";
		commands = addTab("Commands", str);

		str="<b>Admin commands</b><br /><br />";
		str+="/broadcast <message><br />";
		str+="/op<br />";
		str+="/deop<br />";
		str+="<b>Chatop commands</b><br />";
		str+="<br />";
		str+="/gkick<br />";
		str+="/timeout<br />";
		str+="/ipban<br />";
		str+="/accountban<br />";
		str+="/gunban<br />";
		str+="/createchannel<br />";
		str+="/killchannel<br />";
		str+="<b>Chan-Op commands</b><br />";
		str+="<br />";
		str+="/warn<br />";
		str+="/kick<br />";
		str+="/ban<br />";
		str+="/unban<br />";
		str+="/banlist<br />";
		str+="/coplist<br />";
		str+="<b>Chan Owner commands</b><br />";
		str+="<br />";
		str+="/cop<br />";
		str+="/cdeop<br />";
		str+="/setmode $lt;chat|ads|both&gt;<br />";
		admin = addTab("Admin", str);

		str = "<b>BBCode tags:</b><br /><br />"
			  "<i>[i]italic[/i]</i><br />"
			  "<u>[u]underline[/u]</u><br />"
			  "<b>[b]bold[/b]</b><br />"
			  "[user]name[/user] - Link to user<br />"
			  "[channel]channel name[/channel] - Link to channel<br />"
			  "[session=name]linkcode[/session] - Link to private room<br />"
			  "[url=address]label[/url] - The word \"label\" will link to the address<br />";
		admin = addTab("BBCode", str);

		str = "<b>Genders:</b><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_NONE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_NONE] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_MALE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_MALE] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_FEMALE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_FEMALE] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_TRANSGENDER].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_TRANSGENDER] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_SHEMALE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_SHEMALE] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_HERM].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_HERM] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_MALEHERM].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_MALEHERM] + "</span><br />";
		str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_CUNTBOY].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_CUNTBOY] + "</span><br /><br />";
		str += "<b><i>Global OP</i></b><br />";
		str += "<b>Channel OP</b><br />";
		colors = addTab("Colors", str);

		buttons = new QDialogButtonBox(dialog);
		closebutton = new QPushButton(QIcon(":/images/cross.png"), "Close",dialog);
		buttons->addButton(closebutton, QDialogButtonBox::RejectRole);

		vbl->addWidget(tabs);
		vbl->addWidget(buttons);
		dialog->setLayout(vbl);
		dialog->setMinimumSize(500,400);

		QObject::connect(buttons, SIGNAL(rejected()), dialog, SLOT(hide()));
	}
};
}

FHelpDialog::FHelpDialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::FHelpDialog)
{
	ui->setupUi(this);
}
