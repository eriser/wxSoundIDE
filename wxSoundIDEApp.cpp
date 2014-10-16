/***************************************************************
 * Name:      wxSoundIDEApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Jonne Valola (jonnev@gmail.com)
 * Created:   2014-10-13
 * Copyright: Jonne Valola ()
 * License:
 **************************************************************/

#include "wxSoundIDEApp.h"

//(*AppHeaders
#include "wxSoundIDEMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(wxSoundIDEApp);

bool wxSoundIDEApp::OnInit()
{
    testOsc();
    initSound();
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	wxSoundIDEFrame* Frame = new wxSoundIDEFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    return wxsOK;

}
