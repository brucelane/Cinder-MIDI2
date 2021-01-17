/*
 Copyright (c) 2020, Bruce Lane - Martin Blasko All rights reserved.
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
#include "MidiIn.h"
#include "MidiMessage.h"
#include "MidiConstants.h"
// WebSockets
#include "WebSocketClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace asio;

#define SLIDER_NOTE 1


class Midi2WebsocketApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void midiListener(midi::Message msg);

	midi::Input					mMidiIn;

	float						sliderValue = 0.0f;
	string						status = "";
	int							notes[128];
	int							cc[128];
private:
	// Websocket client
	bool						clientConnected = false;
	void						wsWrite(std::string msg);
	void						wsConnect();
	void						wsClientConnect();
	void						parseMessage(string msg);
	void						disconnect();
	WebSocketClient				mClient;
};
void Midi2WebsocketApp::cleanup() {
	disconnect();
}
void Midi2WebsocketApp::setup() {

	wsConnect();
	if (mMidiIn.getNumPorts() > 0) {
		mMidiIn.listPorts();
		mMidiIn.openPort(0);
		console() << "Opening MIDI port 0" << std::endl;
		mMidiIn.midiSignal.connect(std::bind(&Midi2WebsocketApp::midiListener, this, std::placeholders::_1));
	}
	else {
		console() << "No MIDI Ports found!!!!" << std::endl;
	}
}
void Midi2WebsocketApp::wsConnect() {
	mClient.connectOpenEventHandler([&]() {
		clientConnected = true;
		status += "\nConnected";
	});
	mClient.connectCloseEventHandler([&]() {
		clientConnected = false;
		status += "\nDisconnected";
	});
	mClient.connectFailEventHandler([&](string err) {
		status += "\nWS Error";
		if (!err.empty()) {
			status += ": " + err;
		}
	});
	mClient.connectInterruptEventHandler([&]() {
		status += "\nWS Interrupted";
	});
	mClient.connectPingEventHandler([&](string msg) {
		status += "\nWS Ponged";
		if (!msg.empty())
		{
			status += ": " + msg;
		}
	});
	mClient.connectMessageEventHandler([&](string msg) {
		parseMessage(msg);
	});
	wsClientConnect();

}
void Midi2WebsocketApp::wsClientConnect()
{
	mClient.connect("ws://localhost:8088");
}
void Midi2WebsocketApp::wsWrite(string msg)
{
	if (clientConnected) {
		mClient.write(msg);
	}

}
void Midi2WebsocketApp::disconnect()
{
	if (clientConnected) {
		mClient.disconnect();
	}
}
void Midi2WebsocketApp::parseMessage(string msg) {
	status += "\nWS onRead";
	if (!msg.empty()) {
		status += ": " + msg;
	}
}
void Midi2WebsocketApp::midiListener(midi::Message msg) {
	stringstream sParams;
	string strParams = "";
	switch (msg.status)
	{
	case MIDI_NOTE_ON:
		notes[msg.pitch] = msg.velocity;
		status = "Pitch: " + toString(msg.pitch) + "\n" + "Velocity: " + toString(msg.velocity);
		sliderValue = msg.pitch / 127.0f;
		break;
	case MIDI_NOTE_OFF:
		break;
	case MIDI_CONTROL_CHANGE:
		cc[msg.control] = msg.value;
		sliderValue = msg.value / 127.0f;
		status = "Control: " + toString(msg.control) + "\n" +
			"Value: " + toString(msg.value);
		// TODO check boolean value:
		sParams << "{\"params\" :[{\"name\" : " << msg.control << ",\"value\" : " << sliderValue << "}]}";
		strParams = sParams.str();
		wsWrite(strParams);
		break;
	default:
		break;
	}

}
void Midi2WebsocketApp::update() {
	mClient.poll();
}

void Midi2WebsocketApp::draw() {
	gl::clear(Color(0, 0, 0), true);
	gl::color(Color(1, 1, 1));
	gl::drawSolidRect(Rectf(vec2(0, 0), vec2(sliderValue * getWindowWidth(), getWindowHeight())));
}

CINDER_APP(Midi2WebsocketApp, RendererGl)
