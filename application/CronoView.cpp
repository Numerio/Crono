/*
 * Copyright (c) 2012-2015  Dario Casalinuovo (b.vitruvio@gmail.com)
 * Copyright (c) 2012-2015  Davide Gessa (gessadavide@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Authors:
 *		Davide Gessa, dak.linux@gmail.com
 *		Dario Casalinuovo
 */

#include "CronoView.h"

#include <Alert.h>
#include <Button.h>
#include <Dragger.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Roster.h>
#include <Slider.h>
#include <TextControl.h>
#include <View.h>
#include <Box.h>
#include <RadioButton.h>

#include "App.h"
#include "Core.h"
#include "CronoDefaults.h"
#include "SettingsWindow.h"
#include "VolumeSlider.h"

#include <math.h>
#include <stdlib.h>

struct TempoNames {
	int32 min;
	int32 max;
	const char* name;
};

// This is used as a map to fill the
// the speed meter label
static TempoNames gTempoNames[] = {
	{ 10, 39, "Grave" },
	{ 40, 59, "Largo" },
	{ 60, 65, "Larghetto" },
	{ 66, 75, "Lento/Adagio" },
	{ 76, 107, "Andante" },
	{ 108, 119, "Moderato" },
	{ 120, 169, "Allegro" },
	{ 170, 184, "Vivace" },
	{ 185, 209, "Presto" },
	{ 210, 500, "Prestissimo" },
	{ 0, 0, NULL }
};


CronoView::CronoView()
	:
	BView("CronoView", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fAccentsList(false)
{
    fReplicated = false;

	// Core
	fCore = new Core();

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	rgb_color barColor = { 0, 200, 0 };
	rgb_color fillColor = { 240, 240, 240 };


	_BuildMenu();

	// Volume slider
	BBox* volBox = new BBox("volbox");
	volBox->SetLabel("Volume");

	BGroupLayout* volLayout = new BGroupLayout(B_VERTICAL);
	volLayout->SetInsets(10, volBox->TopBorderOffset() * 2 + 10, 10, 10);
	volBox->SetLayout(volLayout);
	
	fVolumeSlider = new VolumeSlider("",
		0, 1000, DEFAULT_VOLUME, new BMessage(MSG_VOLUME));

	fVolumeSlider->SetLimitLabels("Min", "Max");
	fVolumeSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fVolumeSlider->SetHashMarkCount(20);
	fVolumeSlider->SetValue((int32)fCore->Volume()*10);
	fVolumeSlider->UseFillColor(true, &fillColor);
	fVolumeSlider->SetPosition(gCronoSettings.CronoVolume);
	fVolumeSlider->SetLabel(BString() << gCronoSettings.CronoVolume);
	volLayout->AddView(fVolumeSlider);

	// Speed Slider & TextControl
	BBox* speedBox = new BBox("speedbox");
	speedBox->SetLabel("BPM");
	BGroupLayout* speedLayout = new BGroupLayout(B_HORIZONTAL);
	speedLayout->SetInsets(10, speedBox->TopBorderOffset() * 2 + 10, 10, 10);
	speedBox->SetLayout(speedLayout);

	fSpeedSlider = new VolumeSlider("",
		MIN_SPEED, MAX_SPEED, DEFAULT_SPEED, new BMessage(MSG_SPEED_SLIDER));

	fSpeedSlider->SetLimitLabels(BString() << MIN_SPEED,
		BString() << MAX_SPEED);

	fSpeedSlider->SetKeyIncrementValue(5);
	fSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fSpeedSlider->SetHashMarkCount(15);
	fSpeedSlider->SetValue(fCore->Speed());
	fSpeedSlider->UseFillColor(true, &fillColor);
	_UpdateTempoName(gCronoSettings.Speed);

	fSpeedEntry = new BTextControl("", "", BString() << gCronoSettings.Speed,
		new BMessage(MSG_SPEED_ENTRY), B_WILL_DRAW);

	fSpeedEntry->SetDivider(70);
	fSpeedEntry->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_RIGHT);
	fSpeedEntry->SetExplicitSize(BSize(35, 20));

	speedLayout->AddView(fSpeedSlider);
	speedLayout->AddView(fSpeedEntry);

	// Meter buttons
	BBox* tempoBox = new BBox("tempoBox");
	tempoBox->SetLabel("Tempo");

	for(int i = 0; i < 5; i++)
		fTempoRadios[i] = new BRadioButton("", "",
			new BMessage(MSG_METER_RADIO));

	fTempoRadios[0]->SetLabel("1/4");
	fTempoRadios[1]->SetLabel("2/4");
	fTempoRadios[2]->SetLabel("3/4");
	fTempoRadios[3]->SetLabel("4/4");
	fTempoRadios[4]->SetLabel("Other");

	fTempoRadios[fCore->Meter()]->SetValue(1);

	fTempoEntry = new BTextControl("", "", "4",
		new BMessage(MSG_METER_ENTRY), B_WILL_DRAW);

	fTempoEntry->SetDivider(70);

	if (fTempoRadios[4]->Value() == 1)
		fTempoEntry->SetEnabled(true);
	else
		fTempoEntry->SetEnabled(false);		

	fAccentsView = new BGroupView(B_HORIZONTAL, 0);

	BLayoutBuilder::Group<>(tempoBox, B_VERTICAL, 0)
		.SetInsets(10, tempoBox->TopBorderOffset() * 2 + 10, 10, 10)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fTempoRadios[0])
			.Add(fTempoRadios[1])
			.Add(fTempoRadios[2])
			.Add(fTempoRadios[3])
			.Add(fTempoRadios[4])
			.Add(fTempoEntry)
			.AddGlue()
		.End()
		.Add(fAccentsView)
		.AddGlue()
	.End();

	if (gCronoSettings.AccentTable == true)
		_ShowTable(true);

	fStartButton = new BButton("Start", new BMessage(MSG_START));						
	fStartButton->MakeDefault(true);	
	fStopButton = new BButton("Stop", new BMessage(MSG_STOP));							

#ifdef CRONO_REPLICANT_ACTIVE
	// Dragger
	BRect frame(Bounds());
	frame.left = frame.right - 7;
	frame.top = frame.bottom - 7;
	BDragger *dragger = new BDragger(frame, this,
		B_FOLLOW_RIGHT | B_FOLLOW_TOP); 
#endif

	// Create view
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.Add(volBox)
		.Add(speedBox)
		.Add(tempoBox)
		.AddGroup(B_HORIZONTAL)
			.Add(fStartButton)
			.Add(fStopButton)
		.End()
#ifdef CRONO_REPLICANT_ACTIVE
		.Add(dragger)
#endif
		.End();
}


CronoView::CronoView(BMessage* archive)
	:
	BView(archive)
{
    fReplicated = true;

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Core
	fCore = new Core();

	fMenuBar = new BMenuBar(archive);
	fHelpMenu = new BMenu(archive);
	fFileMenu = new BMenu(archive);
	fEditMenu = new BMenu(archive);
	fStartButton = new BButton(archive);
	fStopButton = new BButton(archive);
	fVolumeSlider = new BSlider(archive);
	
	for(int i = 0; i < 5; i++)
		fTempoRadios[i] = new BRadioButton(archive);
	fTempoEntry = new BTextControl(archive);
	fSpeedEntry = new BTextControl(archive);
	fSpeedSlider = new BSlider(archive);

	fReplicated = true;
}


CronoView::~CronoView()
{
	fCore->Stop();
	delete fCore;
}


void
CronoView::AboutRequested()
{
	BAlert *alert = new BAlert("About Crono", 
	"\nCrono Metronome v1.0 Alpha1\n\n"
	"Copyright ©2009-2013 Dario Casalinuovo\n"
	"Copyright ©2009-2013 Davide Gessa \n\n"
	"Crono is a Software Metronome for Haiku\n"
	"http://www.versut.com/\n\n"
	"Written By:\n"
	"Dario Casalinuovo (GUI, Core)\n"
	"Davide Gessa (GUI)\n\n"
	"Released under the terms of the MIT license.\n\n"
	"Special Thanks :\n Stefano D'Angelo\n"
	" Stephan Aßmus, for the VolumeSlider class",
	"OK", NULL, NULL, B_WIDTH_FROM_WIDEST, B_EVEN_SPACING, B_INFO_ALERT);
	alert->Go();
}


status_t
CronoView::Archive(BMessage* archive, bool deep) const
{
    BView::Archive(archive, deep);
    archive->AddString("add_on", CRONO_APP_TYPE);
    archive->AddString("class", "CronoView");
    return B_OK;
}


BArchivable*
CronoView::Instantiate(BMessage *data)
{
    return new CronoView(data);
}


void
CronoView::AttachedToWindow()
{
	fStartButton->SetTarget(this);
	fStopButton->SetTarget(this);
	fVolumeSlider->SetTarget(this);

	for(int i = 0; i < 5; i++)
		fTempoRadios[i]->SetTarget(this);
	fTempoEntry->SetTarget(this);
	
	fSpeedEntry->SetTarget(this);
	fSpeedSlider->SetTarget(this);

	fFileMenu->SetTargetForItems(this);
	fEditMenu->SetTargetForItems(this);
	fHelpMenu->SetTargetForItems(this);
	fShowMenu->SetTargetForItems(this);

	if (!fReplicated)
		Window()->CenterOnScreen();
}


void
CronoView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case MSG_CLOSE:
		{
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}

		case MSG_ABOUT:
		{
			AboutRequested();
			break;
		}

		case MSG_HOMEPAGE:
		{
			const char* homepage = CRONO_HOMEPAGE_URL;
			be_roster->Launch("text/html",1, const_cast<char**>(&homepage));
			break;
		}

		case MSG_HELP:
		{
			const char* guide = CRONO_USERGUIDE_URL;
			be_roster->Launch("text/html",1, const_cast<char**>(&guide));
			break;
		}

		case MSG_SETTINGS:
		{
			BRect windowRect(150,150,460,445);
			SettingsWindow *settWindow = new SettingsWindow(windowRect, fCore);
			settWindow->Show();
			break;
		}

		case MSG_START:
		{
			status_t ret = fCore->Start();
			if (ret != B_OK) {
				BString str("\nError starting Crono :\n");
				str << strerror(ret);
				BAlert *alert = new BAlert("Error", 
					str.String(),
					"OK", NULL, NULL, B_WIDTH_FROM_WIDEST, B_EVEN_SPACING,
					B_INFO_ALERT);
					alert->Go();
				break;
			}
			fStartButton->SetEnabled(false);
			fStopButton->SetEnabled(true);
			fStopButton->MakeDefault(true);
			fEditMenu->FindItem(MSG_SETTINGS)->SetEnabled(false);
			fFileMenu->FindItem(MSG_START)->SetEnabled(false);
			fFileMenu->FindItem(MSG_STOP)->SetEnabled(true);
			break;
		}

		case MSG_STOP:
		{
			fCore->Stop();
			fStopButton->SetEnabled(false);
			fStartButton->SetEnabled(true);
			fStartButton->MakeDefault(true);
			fEditMenu->FindItem(MSG_SETTINGS)->SetEnabled(true);
			fFileMenu->FindItem(MSG_START)->SetEnabled(true);
			fFileMenu->FindItem(MSG_STOP)->SetEnabled(false);
			break;
		}

		case MSG_VOLUME:
		{
			float position = fVolumeSlider->Position();
			fCore->SetVolume(position);
			fVolumeSlider->SetLabel(BString() << position);
			break;
		}

		case MSG_METER_RADIO:
		{
			int selected = _GetCurrentMeter();

			// If "Other" is selected, enable the fTempoEntry
			if (fTempoRadios[4]->Value() == true) {
				fTempoEntry->SetEnabled(true);
			} else {
				fTempoEntry->SetEnabled(false);
			}
			fCore->SetMeter(selected);

			if (gCronoSettings.AccentTable)
				_SetAccentCheckBox(selected);
			break;
		}

		case MSG_METER_ENTRY:
		{
			int position = abs(atoi(fTempoEntry->Text()));
			if (position < 1) {
				fTempoEntry->SetText("1");
				position = 1;
				return;
			} else if (position > 100) {
				fTempoEntry->SetText("100");
				position = 100;		
			}

			fCore->SetMeter(position);
			_SetAccentCheckBox(position);
			break;
		}

		case MSG_SPEED_ENTRY:
		{
			unsigned bpm = abs(atoi(fSpeedEntry->Text()));

			if (bpm > MAX_SPEED) {
				fSpeedEntry->SetText(BString() << MAX_SPEED);
				bpm = MAX_SPEED;
			} else if (bpm < MIN_SPEED) {
				fSpeedEntry->SetText(BString() << MIN_SPEED);
				bpm = MIN_SPEED;
			}

			fCore->SetSpeed(((int) bpm));
			fSpeedSlider->SetPosition(((float) bpm / MAX_SPEED));
			printf("Crono Speed: %s %d\n", fSpeedEntry->Text(), bpm);
			_UpdateTempoName(bpm);
			break;
		}

		case MSG_SPEED_SLIDER:
		{
			int v = fSpeedSlider->Value();

			BString str;
			fSpeedEntry->SetText(str << v);
			fCore->SetSpeed(v);
			_UpdateTempoName(v);
			break;
		}

		case MSG_ACCENT_TABLE:
		{
			bool marked = !fAccentTableItem->IsMarked();
			gCronoSettings.AccentTable = marked;

			_ShowTable(marked);
			break;
		}

		default:
			BView::MessageReceived(message);
	}
}


void
CronoView::_BuildMenu()
{
	// Menu bar
	fMenuBar = new BMenuBar("MenuBar");

	fFileMenu = new BMenu("Metronome");

	fFileMenu->AddItem(new BMenuItem("Quit", new BMessage(MSG_CLOSE), 'Q'));
	fFileMenu->AddItem(new BSeparatorItem);
	fFileMenu->AddItem(new BMenuItem("Start", new BMessage(MSG_START), 'G'));
	fFileMenu->AddItem(new BMenuItem("Stop", new BMessage(MSG_STOP), 'H'));
	fMenuBar->AddItem(fFileMenu);
	
	fEditMenu = new BMenu("Edit");
	fEditMenu->AddItem(new BMenuItem("Settings", new BMessage(MSG_SETTINGS), 'S'));
	fEditMenu->AddItem(new BSeparatorItem);

	fShowMenu = new BMenu("Show");
	fEditMenu->AddItem(fShowMenu);
	
	BMenuItem* item = new BMenuItem("Visual Metronome", NULL, 0);
	item->SetEnabled(false);
	fShowMenu->AddItem(item);

	item = new BMenuItem("Speed trainer", NULL, 0);
	item->SetEnabled(false);
	fShowMenu->AddItem(item);

	fAccentTableItem = new BMenuItem("Show accents table", 
		new BMessage(MSG_ACCENT_TABLE), 0);

	fShowMenu->AddItem(fAccentTableItem);

	fMenuBar->AddItem(fEditMenu);

	fHelpMenu = new BMenu("Help");

	fHelpMenu->AddItem(new BMenuItem("Help",
		new BMessage(MSG_HELP), 'H'));

	fHelpMenu->AddItem(new BMenuItem("Homepage",
		new BMessage(MSG_HOMEPAGE),'P'));

	fHelpMenu->AddItem(new BSeparatorItem);

	fHelpMenu->AddItem(new BMenuItem("About",
		new BMessage(MSG_ABOUT), 'A'));

	fMenuBar->AddItem(fHelpMenu);
}


void
CronoView::_UpdateTempoName(int32 value)
{
	for (int i = 0; gTempoNames[i].name != NULL; i++) {
		if (value <= gTempoNames[i].max && value >= gTempoNames[i].min) {
			fSpeedSlider->SetLabel(gTempoNames[i].name);
			return;
		}
	}
}


int
CronoView::_GetCurrentMeter()
{
	int radioSelected = 0;
	// Get the selected radiobutton
	for (int i = 0; i < 5; i++) {
		if (fTempoRadios[i]->Value())
			radioSelected = i;
	}

	if (radioSelected == 4)
		radioSelected = abs(atoi(fTempoEntry->Text()));
	else
		return radioSelected+1;

	return radioSelected;
}


void
CronoView::_ShowTable(bool show)
{
	int meter = 0;

	if (show)
		meter = _GetCurrentMeter();

	fAccentTableItem->SetMarked(show);
	_SetAccentCheckBox(meter);

	if (show)
		fAccentsView->Show();
	else
		fAccentsView->Hide();

}


void
CronoView::_SetAccentCheckBox(int value)
{
	int numControls = fAccentsList.CountItems()-1;
	value -= 1;
	if (numControls > value) {
		for (int i = numControls; i != value; i--) {
			fAccentsView->GroupLayout()->RemoveView(fAccentsList.ItemAt(i));
			fAccentsList.RemoveItemAt(i);
		}
	} else if (numControls < value) {
		for (int i = numControls; i != value; i++) {
			BCheckBox* box = new BCheckBox(BString() << i+2, NULL);
			fAccentsList.AddItem(box);
			fAccentsView->GroupLayout()->AddView(box);
		}
	}
}
