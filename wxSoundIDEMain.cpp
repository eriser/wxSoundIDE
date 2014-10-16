/***************************************************************
 * Name:      wxSoundIDEMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Jonne Valola (jonnev@gmail.com)
 * Created:   2014-10-13
 * Copyright: Jonne Valola ()
 * License:
 **************************************************************/

#include "wxSoundIDEMain.h"
#include <wx/msgdlg.h>
#include "led.h"
#include "Synth.h"

//(*InternalHeaders(wxSoundIDEFrame)
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//Global variables

wxPoint A0Pos;

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(wxSoundIDEFrame)
const long wxSoundIDEFrame::ID_BUTTON3 = wxNewId();
const long wxSoundIDEFrame::ID_BUTTON5 = wxNewId();
const long wxSoundIDEFrame::ID_BUTTON4 = wxNewId();
const long wxSoundIDEFrame::ID_BUTTON1 = wxNewId();
const long wxSoundIDEFrame::ID_CUSTOM1 = wxNewId();
const long wxSoundIDEFrame::ID_PANEL1 = wxNewId();
const long wxSoundIDEFrame::ID_SLIDER1 = wxNewId();
const long wxSoundIDEFrame::ID_STATICTEXT3 = wxNewId();
const long wxSoundIDEFrame::ID_STATICTEXT4 = wxNewId();
const long wxSoundIDEFrame::ID_SLIDER2 = wxNewId();
const long wxSoundIDEFrame::ID_RADIOBOX1 = wxNewId();
const long wxSoundIDEFrame::ID_BUTTON2 = wxNewId();
const long wxSoundIDEFrame::ID_LED1 = wxNewId();
const long wxSoundIDEFrame::ID_PANEL2 = wxNewId();
const long wxSoundIDEFrame::ID_CHECKBOX1 = wxNewId();
const long wxSoundIDEFrame::ID_CHECKBOX2 = wxNewId();
const long wxSoundIDEFrame::ID_STATICTEXT2 = wxNewId();
const long wxSoundIDEFrame::ID_STATICTEXT1 = wxNewId();
const long wxSoundIDEFrame::idMenuQuit = wxNewId();
const long wxSoundIDEFrame::idMenuAbout = wxNewId();
const long wxSoundIDEFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxSoundIDEFrame,wxFrame)
    //(*EventTable(wxSoundIDEFrame)
    //*)
END_EVENT_TABLE()

wxSoundIDEFrame::wxSoundIDEFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(wxSoundIDEFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, wxID_ANY, _("wxSoundIde - Copyright 2014 Jonne Valola"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(800,589));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    Panel1 = new wxPanel(this, ID_PANEL1, wxPoint(144,112), wxSize(632,128), wxSIMPLE_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    Decay = new MovableButton(Panel1, ID_BUTTON3, _("D"), wxPoint(56,48), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Decay->SetMinSize(wxSize(20,20));
    Decay->SetMaxSize(wxSize(0,0));
    Rele = new MovableButton(Panel1, ID_BUTTON5, _("R"), wxPoint(128,24), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    Rele->SetMinSize(wxSize(20,20));
    Rele->SetMaxSize(wxSize(0,0));
    Sust = new MovableButton(Panel1, ID_BUTTON4, _("S"), wxPoint(144,72), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    Sust->SetMinSize(wxSize(20,20));
    Sust->SetMaxSize(wxSize(0,0));
    A0 = new MovableButton(Panel1, ID_BUTTON1, _("A"), wxPoint(64,88), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    A0->SetMinSize(wxSize(20,20));
    A0->SetMaxSize(wxSize(0,0));
    Osc1DC = new wxClientDC(Panel1);
    VolSlider1 = new wxSlider(this, ID_SLIDER1, 127, 0, 255, wxPoint(144,48), wxSize(144,24), wxSL_BOTH, wxDefaultValidator, _T("ID_SLIDER1"));
    VolSlider1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    VolLabel1 = new wxStaticText(this, ID_STATICTEXT3, _("Volume: 127"), wxPoint(144,24), wxSize(128,13), wxALIGN_LEFT, _T("ID_STATICTEXT3"));
    PitchLabel1 = new wxStaticText(this, ID_STATICTEXT4, _("Pitch: 100"), wxPoint(304,24), wxSize(240,13), wxALIGN_LEFT, _T("ID_STATICTEXT4"));
    PitchSlider1 = new wxSlider(this, ID_SLIDER2, 100, 1, 1000, wxPoint(296,48), wxSize(488,24), wxSL_BOTH, wxDefaultValidator, _T("ID_SLIDER2"));
    wxString __wxRadioBoxChoices_1[6] =
    {
    	_("OFF"),
    	_("SQUARE"),
    	_("SAW"),
    	_("TRIANGLE"),
    	_("NOISE"),
    	_("SAMPLE")
    };
    RadioBox1 = new wxRadioBox(this, ID_RADIOBOX1, _("Waveform"), wxPoint(32,88), wxDefaultSize, 6, __wxRadioBoxChoices_1, 1, wxNO_BORDER, wxDefaultValidator, _T("ID_RADIOBOX1"));
    Osc1BtnPanel = new wxPanel(this, ID_PANEL2, wxPoint(32,24), wxSize(80,48), wxNO_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Osc1BtnPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    Osc1 = new wxButton(Osc1BtnPanel, ID_BUTTON2, _("OSC 1"), wxPoint(0,24), wxSize(80,24), 0, wxDefaultValidator, _T("ID_BUTTON2"));
    OscLed1 = new wxLed(Osc1BtnPanel,ID_LED1,wxColour(128,128,128),wxColour(11,125,181),wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),wxDefaultPosition,wxDefaultSize);
    OscLed1->SwitchOn();
    VolEnvOn = new wxCheckBox(this, ID_CHECKBOX1, _("Volume envelope"), wxPoint(144,88), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    VolEnvOn->SetValue(false);
    PitchEnvOn = new wxCheckBox(this, ID_CHECKBOX2, _("Pitch envelope"), wxPoint(304,88), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    PitchEnvOn->SetValue(false);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Xpos"), wxPoint(152,272), wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    Xpos = new wxStaticText(this, ID_STATICTEXT1, _("0"), wxPoint(192,272), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);

    Panel1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&wxSoundIDEFrame::OnPanel1Paint1,0,this);
    Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&wxSoundIDEFrame::OnVolSlider1CmdSliderUpdated);
    Connect(ID_SLIDER2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&wxSoundIDEFrame::OnPitchSlider1CmdSliderUpdated);
    Connect(ID_RADIOBOX1,wxEVT_COMMAND_RADIOBOX_SELECTED,(wxObjectEventFunction)&wxSoundIDEFrame::OnRadioBox1Select);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxSoundIDEFrame::OnOsc1Click);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&wxSoundIDEFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&wxSoundIDEFrame::OnAbout);
    //*)

    A0->SetMinSize(wxSize(20,20));
    A0->SetMaxSize(wxSize(0,0));
    A0->SetLabel(_("A")); A0->SetSize(20,20);
    Decay->SetMinSize(wxSize(20,20));
    Decay->SetMaxSize(wxSize(0,0));
    Decay->SetLabel(_("D")); Decay->SetSize(20,20);
    Sust->SetMinSize(wxSize(20,20));
    Sust->SetMaxSize(wxSize(0,0));
    Sust->SetLabel(_("S")); Sust->SetSize(20,20);
    Rele->SetMinSize(wxSize(20,20));
    Rele->SetMaxSize(wxSize(0,0));
    Rele->SetLabel(_("R")); Rele->SetSize(20,20);

}

wxSoundIDEFrame::~wxSoundIDEFrame()
{
    //(*Destroy(wxSoundIDEFrame)
    //*)
}

void wxSoundIDEFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void wxSoundIDEFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void wxSoundIDEFrame::OnRadioBox1Select(wxCommandEvent& event)
{
    setWave(RadioBox1->GetSelection());
}

void wxSoundIDEFrame::OnVolSlider1CmdSliderUpdated(wxScrollEvent& event)
{
    wxString newlabel;
    newlabel << "Volume: " << VolSlider1->GetValue();
    VolLabel1->SetLabelText(newlabel);
    setOSC(&osc1,osc1.on,osc1.wave,PitchSlider1->GetValue(),VolSlider1->GetValue());
}

void wxSoundIDEFrame::OnPitchSlider1CmdSliderUpdated(wxScrollEvent& event)
{
    wxString newlabel;
    newlabel << "Pitch: " << PitchSlider1->GetValue();
    PitchLabel1->SetLabelText(newlabel);
    //setPitch(PitchSlider1->GetValue());
    setOSC(&osc1,osc1.on,osc1.wave,PitchSlider1->GetValue(),VolSlider1->GetValue());
}

void wxSoundIDEFrame::OnOsc1Click(wxCommandEvent& event)
{
    OscLed1->Switch();
}

BEGIN_EVENT_TABLE(MovableButton,wxButton)
EVT_LEFT_DOWN(MovableButton::onMouseDown)
EVT_LEFT_UP(MovableButton::onMouseUp)
EVT_MOTION(MovableButton::onMove)
END_EVENT_TABLE()



void wxSoundIDEFrame::OnPanel1Paint1(wxPaintEvent& event)
{
    int w,h;
    w = Osc1DC->GetSize().GetWidth();
    h = Osc1DC->GetSize().GetHeight();
    Osc1DC->DrawRectangle(0,0,w,h);
    Osc1DC->DrawLine(wxPoint(0,h),A0->GetPosition()+wxPoint(10,10));
    Osc1DC->DrawLine(A0->GetPosition()+wxPoint(10,10),Decay->GetPosition()+wxPoint(10,10));
    Osc1DC->DrawLine(Sust->GetPosition()+wxPoint(10,10),Decay->GetPosition()+wxPoint(10,10));
    Osc1DC->DrawLine(Sust->GetPosition()+wxPoint(10,10),Rele->GetPosition()+wxPoint(10,10));
    Osc1DC->DrawLine(wxPoint(w,h),Rele->GetPosition()+wxPoint(10,10));
}
