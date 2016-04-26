
#include "main.h"

class ProgressBar : public wxFrame
{
private:
	wxStaticText	*message;
	wxGauge			*progress;

public:
	ProgressBar();
	~ProgressBar();

	void setProgress(int val) { progress->SetValue(val); }
	void setMessage(string message);
	void showProgress(bool prog) { progress->Show(prog); }
};

ProgressBar *progress_bar;

ProgressBar::ProgressBar()
:	wxFrame(NULL, -1, _T(""), wxDefaultPosition, wxSize(590, -1), wxRAISED_BORDER)//wxCAPTION)
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	//logo = new ImageBox(this, get_image_from_pk3(_T("res/edit_tex/logo.png"), wxBITMAP_TYPE_PNG));
	//vbox->Add(logo, 0, wxEXPAND);

	message = new wxStaticText(this, -1, _T("Hello"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	vbox->Add(message, 0, wxEXPAND);

	progress = new wxGauge(this, -1, 100);
	vbox->Add(progress, 0, wxEXPAND);

	Layout();
	Show(true);
	SetBestFittingSize();
	Center();
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::setMessage(string message)
{
	this->message->SetLabel(message);
	Layout();
}

void setup_progbar()
{
	progress_bar = new ProgressBar();
}

void progbar_show(string message, bool progbar)
{
	progress_bar->Show();
	progress_bar->showProgress(progbar);
	progress_bar->setMessage(message);
	progress_bar->setProgress(0);
	progress_bar->SetBestFittingSize();
	progress_bar->Layout();
	progress_bar->Update();
}

void progbar_hide()
{
	progress_bar->Hide();
}

void progbar_update(double prog)
{
	progress_bar->setProgress(prog * 100);
	progress_bar->Update();
}
