/*
 * Copyright (c) Casalinuovo Dario. All rights reserved.
 * Copyright (c) Davide Gessa. All rights reserved.
 * Distributed under the terms of the MIT License for non commercial use.
 *
 * Authors:
 *		Casalinuovo Dario, b.vitruvio@gmail.com
 *		Davide Gessa, dak.linux@gmail.com
 */
#include "SettingsView.h"

#include <GroupLayout.h>
#include <GroupView.h>
#include <Box.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>

#include "CronoDefaults.h"

// TODO should save settings on window close.

SettingsView::SettingsView()
	:
	BView("SettingsView", B_WILL_DRAW, 0)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	// Main layout
	BGroupLayout* rootLayout = new BGroupLayout(B_VERTICAL);
	SetLayout(rootLayout);

	// Audiofile entry
	BGroupView* audioGroup = new BGroupView(B_HORIZONTAL);
	BBox* audioBox = new BBox("audiobox");
	audioBox->SetLabel("Audio files");

	BGroupLayout* audioLayout = new BGroupLayout(B_VERTICAL);
	audioLayout->SetInsets(10, audioBox->TopBorderOffset() * 2 + 10, 10, 10);
	audioBox->SetLayout(audioLayout);
	
	fTocSoundEntry = new BTextControl("Toc", "Toc", CRONO_TOC_LOCATION,
		new BMessage(MSG_SET), B_WILL_DRAW);

	fTocSoundEntry->SetDivider(70);
	fTocSoundEntry->SetAlignment(B_ALIGN_CENTER, B_ALIGN_CENTER);
		
	fTicSoundEntry = new BTextControl("Tic", "Tic", CRONO_TIC_LOCATION", 
		new BMessage(MSG_SET), B_WILL_DRAW);

	fTicSoundEntry->SetDivider(70);
	fTicSoundEntry->SetAlignment(B_ALIGN_CENTER, B_ALIGN_CENTER);

	audioLayout->AddView(fTicSoundEntry);
	audioLayout->AddView(fTocSoundEntry);
	audioGroup->GroupLayout()->AddView(audioBox);
	rootLayout->AddView(audioGroup);
	
	// Defaults and revert buttons
	BGroupView* buttonGroup = new BGroupView(B_HORIZONTAL);

	fDefaultsButton = new BButton("Defaults", new BMessage(MSG_DEFAULTS));						
	buttonGroup->GroupLayout()->AddView(fDefaultsButton);

	fRevertButton = new BButton("Revert", new BMessage(MSG_REVERT));							
	buttonGroup->GroupLayout()->AddView(fRevertButton);

	rootLayout->AddView(buttonGroup);
}


SettingsView::~SettingsView()
{
}


void
SettingsView::AttachedToWindow()
{
	fDefaultsButton->SetTarget(this);
	fRevertButton->SetTarget(this);
	fTicSoundEntry->SetTarget(this);
	fTocSoundEntry->SetTarget(this);	
	
	Window()->CenterOnScreen();
}


void
SettingsView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case MSG_REVERT:
		case MSG_DEFAULTS:
		{
			fTocSoundEntry->SetText(CRONO_TOC_LOCATION);
			fTicSoundEntry->SetText(CRONO_TIC_LOCATION);
		}
		break;

		case MSG_SET:
		{
		
		}

		default:
			BView::MessageReceived(message);
	}			
}
