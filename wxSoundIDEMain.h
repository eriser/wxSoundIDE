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

#include <wx/timer.h>

extern wxPoint A0Pos;
extern uint16_t scopew;
extern int16_t attax, decayx, sustx, relex;


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

        void SetPos(uint16_t pos, uint16_t val)
        {
            uint16_t w,h,h0;
            w = parent->GetSize().GetWidth();
            h = parent->GetSize().GetHeight();
            h0 = h - 20;

            //this->Move(parent->ScreenToClient(wxPoint(w*pos/32727, val*h/256)));
            this->Move(wxPoint(w*(float)pos/scopew, h0-(float)val/255*h0));
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
                wxPoint newpoint = parent->ScreenToClient( wxPoint(newx, newy) );
                if (newpoint.x < 0) newpoint.x = 0;
                if (newpoint.y < 0) newpoint.y = 0;
                if (newpoint.x > parent->GetSize().GetWidth()-20) newpoint.x = parent->GetSize().GetWidth()-20;
                if (newpoint.y > parent->GetSize().GetHeight()-20) newpoint.y = parent->GetSize().GetHeight()-20;

                // Check for ADSR rules
                if (this->GetLabel()=="R") newpoint.y = parent->GetSize().GetHeight()-20;
                if (this->GetLabel()=="A") newpoint.y = 0;
                if (this->GetLabel()=="A" && newpoint.x > decayx) newpoint.x = decayx;
                if (this->GetLabel()=="D" && newpoint.x > sustx) newpoint.x = sustx;
                if (this->GetLabel()=="D" && newpoint.x < attax) newpoint.x = attax;
                if (this->GetLabel()=="S" && newpoint.x > relex) newpoint.x = relex;
                if (this->GetLabel()=="S" && newpoint.x < decayx) newpoint.x = decayx;
                if (this->GetLabel()=="R" && newpoint.x < sustx) newpoint.x = sustx;

                if (this->GetLabel()=="A") attax = newpoint.x;
                if (this->GetLabel()=="D") decayx = newpoint.x;
                if (this->GetLabel()=="S") sustx = newpoint.x;
                if (this->GetLabel()=="R") relex = newpoint.x;

                this->Move(newpoint);
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
        void UpdateScope();


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
        void OnPWMBtnClick(wxCommandEvent& event);
        void OnLengthSlider1CmdSliderUpdated(wxScrollEvent& event);
        void OnLoopClick(wxCommandEvent& event);
        void OnPlayClick(wxCommandEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnCloseWindow(wxCloseEvent& event);
        void OnADSRCheckBoxClick(wxCommandEvent& event);
        void OnOutputFileClick(wxCommandEvent& event);
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
        static const long ID_SLIDER3;
        static const long ID_SLIDER2;
        static const long ID_RADIOBOX1;
        static const long ID_BUTTON6;
        static const long ID_LED2;
        static const long ID_PANEL3;
        static const long ID_BUTTON7;
        static const long ID_LED3;
        static const long ID_PANEL4;
        static const long ID_BUTTON2;
        static const long ID_LED1;
        static const long ID_PANEL2;
        static const long ID_CHECKBOX1;
        static const long ID_CHECKBOX2;
        static const long ID_STATICTEXT1;
        static const long ID_BUTTON8;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(wxSoundIDEFrame)
        MovableButton* Rele;
        wxPanel* Osc1BtnPanel;
        MovableButton* Sust;
        wxButton* OutputFile;
        MovableButton* A0;
        wxClientDC* Osc1DC;
        wxCheckBox* ADSRCheckBox;
        wxSlider* PitchSlider1;
        wxCheckBox* PitchCheckBox;
        wxStaticText* PitchLabel1;
        wxStaticText* LengthLabel;
        wxPanel* Panel1;
        wxButton* Loop;
        wxPanel* Panel3;
        wxSlider* VolSlider1;
        MovableButton* Decay;
        wxStatusBar* StatusBar1;
        wxButton* Play;
        wxStaticText* VolLabel1;
        wxLed* PlayLed;
        wxRadioBox* RadioBox1;
        wxPanel* Panel2;
        wxLed* PWMLed;
        wxButton* PWMBtn;
        wxSlider* LengthSlider1;
        wxLed* LoopLed;
        //*)

        wxTimer* timer;
        DECLARE_EVENT_TABLE()
};





#endif // WXSOUNDIDEMAIN_H
