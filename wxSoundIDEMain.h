/***************************************************************
 * Name:      wxSoundIDEMain.h
 * Purpose:   Defines Application Frame
 * Author:    Jonne Valola (jonnev@gmail.com)
 * Created:   2014-10-13
 * Copyright: Jonne Valola ()
 * License:
 **************************************************************/

#ifndef WXSOUNDIDEMAIN_H
#define WXSOUNDIDEMAIN_H

//(*Headers(wxSoundIDEFrame)
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/menu.h>
#include <wx/checkbox.h>
#include <wx/led.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/dcclient.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

extern wxPoint A0Pos;

class MovableButton : public wxButton
    {
        bool dragging;

        wxPanel* parent;
        int x,y;

    public:

        //this is what wxSmith passes
        //Panel1, ID_BUTTON1, _("A0"), wxPoint(0,88), wxDefaultSize, 0,
        //wxDefaultValidator, _T("ID_BUTTON1"));

        // (wxPanel*&, const long int&,
        //const wxString&, wxPoint,
        // const wxSize&, int, const wxValidator&, const wchar_t [11])'|


        MovableButton(wxPanel *parent,
                const long int& dummy = 0,
                const wxString& label = wxControlNameStr,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxControlNameStr) : wxButton(parent, wxID_ANY, wxT(""))
        {
            MovableButton::parent = parent;
        }

        void onMouseDown(wxMouseEvent& evt)
        {
            CaptureMouse();
            x = evt.GetX();
            y = evt.GetY();
            dragging=true;
            //evt.Veto();
        }
        void onMouseUp(wxMouseEvent& evt)
        {
            ReleaseMouse();
            dragging=false;
        }

        void onMove(wxMouseEvent& evt)
        {
            if(dragging)
            {
                wxClientDC* temp;
                wxString s = "Osc1DC";
                wxPoint mouseOnScreen = wxGetMousePosition();
                int newx = mouseOnScreen.x - x;
                int newy = mouseOnScreen.y - y;
                this->Move( parent->ScreenToClient( wxPoint(newx, newy) ) );
                temp = (wxClientDC *) parent->FindWindow(s);
                if (temp) temp->DrawText(wxT("Hello World !!!"),20,10);
            }
        }

        DECLARE_EVENT_TABLE()
    };


class wxSoundIDEFrame: public wxFrame
{

    public:

        wxSoundIDEFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~wxSoundIDEFrame();

    private:

        //(*Handlers(wxSoundIDEFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnRadioBox1Select(wxCommandEvent& event);
        void OnVolSlider1CmdSliderUpdated(wxScrollEvent& event);
        void OnPitchSlider1CmdSliderUpdated(wxScrollEvent& event);
        void OnOsc1Click(wxCommandEvent& event);
        void OnRadioBox1Select1(wxCommandEvent& event);
        void OnPanel1Paint(wxPaintEvent& event);
        void OnPanel1Paint1(wxPaintEvent& event);
        //*)

        //(*Identifiers(wxSoundIDEFrame)
        static const long ID_BUTTON3;
        static const long ID_BUTTON5;
        static const long ID_BUTTON4;
        static const long ID_BUTTON1;
        static const long ID_CUSTOM1;
        static const long ID_PANEL1;
        static const long ID_SLIDER1;
        static const long ID_STATICTEXT3;
        static const long ID_STATICTEXT4;
        static const long ID_SLIDER2;
        static const long ID_RADIOBOX1;
        static const long ID_BUTTON2;
        static const long ID_LED1;
        static const long ID_PANEL2;
        static const long ID_CHECKBOX1;
        static const long ID_CHECKBOX2;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT1;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(wxSoundIDEFrame)
        MovableButton* Rele;
        wxPanel* Osc1BtnPanel;
        wxStaticText* Xpos;
        MovableButton* Sust;
        MovableButton* A0;
        wxClientDC* Osc1DC;
        wxStaticText* StaticText2;
        wxSlider* PitchSlider1;
        wxCheckBox* PitchEnvOn;
        wxStaticText* PitchLabel1;
        wxPanel* Panel1;
        wxCheckBox* VolEnvOn;
        wxSlider* VolSlider1;
        MovableButton* Decay;
        wxStatusBar* StatusBar1;
        wxStaticText* VolLabel1;
        wxRadioBox* RadioBox1;
        wxLed* OscLed1;
        wxButton* Osc1;
        //*)

        DECLARE_EVENT_TABLE()
};





#endif // WXSOUNDIDEMAIN_H
