/*
*  MidiIn.cpp
*  glitches
*
*  Created by hec on 5/20/10.
*  Copyright 2010 aer studio. All rights reserved.
*
*/

#include "MidiIn.h"
#include "cinder/Log.h"

namespace cinder { namespace midi {

	void MidiInCallback(double deltatime, std::vector<unsigned char> *message, void *userData){
		// one-shot ground truth for "no midi message is received": if this never logs, RtMidi's
		// own OS-level callback (registered in Input::openPort()) is never firing at all - the
		// problem is below this point (driver/device/port), not in VDMidi's signal wiring above it
		CI_LOG_V("MidiInCallback: " << (message ? message->size() : 0) << " bytes");
		((Input*)userData)->processMessage(deltatime, message);
	}

	void RtMidiErrorLogCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
		CI_LOG_E("RtMidi error (type " << (int)type << "): " << errorText);
		if (Input* input = static_cast<Input*>(userData)) input->mHadOpenError = true;
	}


	Input::Input(){
		mMidiIn = new RtMidiIn();
		// surface RtMidi's own internal failures (a failed midiInOpen/midiInStart, in particular)
		// into the app's actual log - see RtMidiErrorLogCallback's comment by its declaration
		mMidiIn->setErrorCallback(&RtMidiErrorLogCallback, this);
		// this snapshot is only ever used as openPort()'s bounds check, refreshed every time
		// listPorts() runs - getNumPorts() itself no longer reads it (see MidiIn.h), since a
		// port count fixed at construction time never reflects a controller plugged in
		// afterward, which was the actual cause of "enabling midi doesn't show any devices":
		// the app constructs this Input once at startup, so an interface enumerated later
		// (or simply not yet ready when RtMidiIn probed it) would stay invisible for the
		// entire session no matter how many times the UI's "Enable"/rescan button was pressed.
		mNumPorts = mMidiIn->getPortCount();
		mMidiIn->getCurrentApi();
	}

	Input::~Input(){
		closePort();
	}

	void Input::listPorts(){
		// re-query live and start from an empty list - previously this appended to mPortNames
		// without clearing it first, so calling listPorts() more than once (every "Enable"
		// click) duplicated every entry already in the list on top of using a stale count
		mNumPorts = mMidiIn->getPortCount();
		mPortNames.clear();
		std::cout << "MidiIn: " << mNumPorts << " available." << std::endl;
		for (size_t i = 0; i < mNumPorts; ++i){
			std::cout << i << ": " << mMidiIn->getPortName(i).c_str() << std::endl;
			std::string name( mMidiIn->getPortName( i ).c_str() ); // strip null chars introduced by rtmidi
			mPortNames.push_back( name );
		}
	}

	void Input::ignoreTypes(bool sysex, bool time, bool midisense){
		mMidiIn->ignoreTypes(sysex, time, midisense);

	}

	std::string Input::getPortName(int number){

		return mPortNames.at(number);
	}

	void Input::openPort(unsigned int port){
		mHadOpenError = false;
		if (mNumPorts == 0){
			throw MidiExcNoPortsAvailable();
		}

		if (port + 1 > mNumPorts){
			throw MidiExcPortNotAvailable();
		}

		mPort = port;
		mName = mMidiIn->getPortName(port);

		mMidiIn->openPort(mPort);

		mMidiIn->setCallback(&MidiInCallback, this);

		mMidiIn->ignoreTypes(false, false, false);
	}

	void Input::closePort(){
		mMidiIn->closePort();
		mMidiIn->cancelCallback();
	}

	void Input::processMessage(double deltatime, std::vector<unsigned char> *message){
		unsigned int numBytes = message->size();

		// solution for proper reading anything above MIDI_TIME_CODE goes to miguelvb
		// http://forum.openframeworks.cc/t/incorrect-handling-of-midiin-messages-in-ofxmidi-solved/8719
		

			Message msg;
			msg.port = mPort;
			if((message->at(0)) >= MIDI_SYSEX) {
				msg.status = (MidiStatus)(message->at(0) & 0xFF);
				msg.channel = 0;
			} else {
				msg.status = (MidiStatus) (message->at(0) & 0xF0);
				msg.channel = (int) (message->at(0) & 0x0F)+1;
			}


			msg.port = mPort;

			switch(msg.status) {
			case MIDI_NOTE_ON :
			case MIDI_NOTE_OFF:
				msg.pitch = (int) message->at(1);
				msg.velocity = (int) message->at(2);
				break;
			case MIDI_CONTROL_CHANGE:
				msg.control = (int) message->at(1);
				msg.value = (int) message->at(2);
				break;
			case MIDI_PROGRAM_CHANGE:
			case MIDI_AFTERTOUCH:
				msg.value = (int) message->at(1);
				break;
			case MIDI_PITCH_BEND:
				msg.value = (int) (message->at(2) << 7) +
					(int) message->at(1); // msb + lsb
				break;
			case MIDI_POLY_AFTERTOUCH:
				msg.pitch = (int) message->at(1);
				msg.value = (int) message->at(2);
				break;
			default:
				break;
			}
        
            midiThreadSignal.emit( msg );
        
            if (mDispatchToMainThread)
                ci::app::App::get()->dispatchAsync( [this, msg](){ midiSignal.emit( msg ); });
		}

		// bool Input::hasWaitingMessages(){
		// 	int queue_length = (int)mMessages.size();
		// 	return queue_length > 0;
		// }

		// bool Input::getNextMessage(Message* message){
		// 	if (mMessages.size() == 0){
		// 		return false;
		// 	}

		// 	Message* src_message = mMessages.front();
		// 	message->copy(*src_message);
		// 	delete src_message;
		// 	mMessages.pop_front();

		// 	return true;
		// }

		unsigned int Input::getPort()const{
			return mPort;

		}


	} // namespace midi
} // namespace cinder
