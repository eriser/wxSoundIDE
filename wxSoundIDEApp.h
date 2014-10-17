/***************************************************************
 * Name:      wxSoundIDEApp.h
 * Purpose:   Defines Application Class
 * Author:    Jonne Valola (jonnev@gmail.com)
 * Created:   2014-10-13
 * Copyright: Jonne Valola ()
 * License:
 **************************************************************/

#ifndef WXSOUNDIDEAPP_H
#define WXSOUNDIDEAPP_H

#include <wx/app.h>
#include "Synth.h"

class wxSoundIDEApp : public wxApp
{
    public:
        virtual bool OnInit();
        //virtual int OnExit();
};

#endif // WXSOUNDIDEAPP_H
