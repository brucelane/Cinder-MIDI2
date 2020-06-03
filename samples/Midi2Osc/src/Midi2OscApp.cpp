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
#include "cinder/osc/Osc.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::osc;
using namespace asio;
using namespace asio::ip;
#define SLIDER_NOTE 1

#define USE_UDP 1

#if USE_UDP
using Sender = osc::SenderUdp;
#else
using Sender = osc::SenderTcp;
#endif

const std::string destinationHost = "127.0.0.1";
const uint16_t destinationPort = 10001;
const uint16_t localPort = 10000;

class Midi2OscApp : public App {
public:
	Midi2OscApp();
	void setup() override;
	void update() override;
	void draw() override;
	void midiListener(midi::Message msg);

	midi::Input mMidiIn;

	float sliderValue = 0.0f;
	string status;
	int notes[128];
	//int cc[128];

	ivec2	mCurrentMousePositon;

	void onSendError(asio::error_code error);
	Sender	mSender;
	bool	mIsConnected;
};

Midi2OscApp::Midi2OscApp()
	: mSender(localPort, destinationHost, destinationPort), mIsConnected(false)
{
}
void Midi2OscApp::setup() {

	if (mMidiIn.getNumPorts() > 0) {
		mMidiIn.listPorts();
		mMidiIn.openPort(0);
		console() << "Opening MIDI port 0" << std::endl;
		mMidiIn.midiSignal.connect(std::bind(&Midi2OscApp::midiListener, this, std::placeholders::_1));
	}
	else {
		console() << "No MIDI Ports found!!!!" << std::endl;
	}
	try {
		// Bind the sender to the endpoint. This function may throw. The exception will
		// contain asio::error_code information.
		mSender.bind();
	}
	catch (const osc::Exception &ex) {
		//CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
		quit();
	}

#if ! USE_UDP
	mSender.connect(
		// Set up the OnConnectFn. If there's no error, you can consider yourself connected to
		// the endpoint supplied.
		[&](asio::error_code error) {
		if (error) {
			CI_LOG_E("Error connecting: " << error.message() << " val: " << error.value());
			quit();
		}
		else {
			CI_LOG_V("Connected");
			mIsConnected = true;
		}
	});
#else
	// Udp doesn't "connect" the same way Tcp does. If bind doesn't throw, we can
	// consider ourselves connected.
	mIsConnected = true;
#endif
}

// Unified error handler. Easiest to have a bound function in this situation,
// since we're sending from many different places.
void Midi2OscApp::onSendError(asio::error_code error)
{
	if (error) {
		CI_LOG_E("Error sending: " << error.message() << " val: " << error.value());
		// If you determine that this error is fatal, make sure to flip mIsConnected. It's
		// possible that the error isn't fatal.
		mIsConnected = false;
		try {
#if ! USE_UDP
			// If this is Tcp, it's recommended that you shutdown before closing. This
			// function could throw. The exception will contain asio::error_code
			// information.
			mSender.shutdown();
#endif
			// Close the socket on exit. This function could throw. The exception will
			// contain asio::error_code information.
			mSender.close();
		}
		catch (const osc::Exception &ex) {
			CI_LOG_E("Cleaning up socket: val -" << ex.value(), ex);
		}
		quit();
	}
}
void Midi2OscApp::midiListener(midi::Message msg) {
	osc::Message oscMsg("/cc");
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
		//cc[msg.control] = msg.value;
		sliderValue = msg.value / 127.0f;
		status = "Control: " + toString(msg.control) + "\n" +
			"Value: " + toString(msg.value);
		
		oscMsg.append(msg.control);
		oscMsg.append(sliderValue);
		// Send the msg and also provide an error handler. If the message is important you
		// could store it in the error callback to dispatch it again if there was a problem.
		mSender.send(oscMsg, std::bind(&Midi2OscApp::onSendError,
			this, std::placeholders::_1));

		break;
	default:
		break;
	}
	// Make sure you're connected before trying to send.
	if (!mIsConnected)
		return;

}
void Midi2OscApp::update() {

}

void Midi2OscApp::draw() {
	gl::clear(Color(0, 0, 0), true);
	gl::color(Color(1, 1, 1));
	gl::drawSolidRect(Rectf(vec2(0, 0), vec2(sliderValue * getWindowWidth(), getWindowHeight())));
}
auto settingsFunc = [](App::Settings *settings) {
#if defined( CINDER_MSW )
	settings->setConsoleWindowEnabled();
#endif
	settings->setMultiTouchEnabled(false);
};
CINDER_APP(Midi2OscApp, RendererGl, settingsFunc)
