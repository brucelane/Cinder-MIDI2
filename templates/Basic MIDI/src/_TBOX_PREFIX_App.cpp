/*
 Copyright (c) 2016, Bruce Lane - Martin Blasko - Hunter Luisi All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 This file is part of Cinder-MIDI.

 Cinder-MIDI is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Cinder-MIDI is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Cinder-MIDI.  If not, see <http://www.gnu.org/licenses/>.
*/

// don't forget to add winmm.lib to the linker

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include <list>
#include "CinderMidi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define SLIDER_NOTE 1


class _TBOX_PREFIX_App : public App {
 public:
	void setup();
	void update();
	void draw();
	void midiListener(midi::MidiMessage msg);

	midi::MidiInput mInput;
	
	float sliderValue;
	string status;
	int notes[128];
	int cc[128];
};

void _TBOX_PREFIX_App::setup(){
	
	mInput.GetPortList();
	console() << "NUMBER OF PORTS: " << mInput.mPortCount << endl;
	for (int i = 0; i < mInput.mPortCount; i++)
	{
		console() << mInput.GetPortName(i) << endl;
	}
	mInput.OpenPort(0);

	mInput.mMidiInCallback = std::bind(&_TBOX_PREFIX_App::midiListener, this, std::placeholders::_1);
	sliderValue = 0.5;
}
void _TBOX_PREFIX_App::midiListener(midi::MidiMessage msg){
  switch (msg.StatusCode)
  {
  case MIDI_NOTE_ON:
      notes[msg.Pitch] = msg.Velocity;
      status = "Pitch: " + toString(msg.Pitch) + "\n" + "Velocity: " + toString(msg.Velocity);
      sliderValue = msg.Pitch / 127.0f;
      break;
  case MIDI_NOTE_OFF:
      break;
  case MIDI_CONTROL_CHANGE:
      cc[msg.Control] = msg.Value;
	  sliderValue = msg.Value / 127.0f;
	  status = "Control: " + toString(msg.Control) + "\n" + 
          "Value: " + toString(msg.Value);
      break;
  default:
      break;
  }

}
void _TBOX_PREFIX_App::update(){
	
}

void _TBOX_PREFIX_App::draw(){
	gl::clear(Color(0,0,0), true);
	gl::color(Color(0.5, 0, 0.6));
	gl::drawSolidRect(Rectf(vec2(0, 0), vec2(sliderValue * getWindowWidth(), getWindowHeight())));
}

CINDER_APP( _TBOX_PREFIX_App, RendererGl )
