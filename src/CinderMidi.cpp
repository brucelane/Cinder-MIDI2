/*  ====================================================================
	File: CinderMidi.cpp
	Date: 02/13/2015
	Revision:
	Creator: Hunter Luisi
	Notice: (C) Copyright 2015 by Hunter Luisi.  All Rights Reserved.
	==================================================================== */

#include "CinderMidi.h"

namespace cinder {
	namespace midi {

		//------------------------------------------------------------------
		//  Midi Message weak copy
		//------------------------------------------------------------------
		MidiMessage& MidiMessage::copy(const MidiMessage& other) {
			Port = other.Port;
			Channel = other.Channel;
			StatusCode = other.StatusCode;
			ByteOne = other.ByteOne;
			ByteTwo = other.ByteTwo;
			Value = other.Value;
			Pitch = other.Pitch;
			Velocity = other.Velocity;
			Control = other.Control;
			TimeStamp = other.TimeStamp;
			Name = other.Name;
			return *this;
		}

		//------------------------------------------------------------------
		//  Midi Input
		//------------------------------------------------------------------
		MidiInput::MidiInput() {
			//RT_MidiIn = nullptr;
			try {
				MidiIn = std::make_unique<RtMidiIn>();
			}
			catch (RtMidiError &error) {
				// Handle the exception here
				error.printMessage();
			}

			MidiIn->getCurrentApi();
			mPortCount = MidiIn->getPortCount();

			for (int i = 0; i < 127; ++i) {
				mNotesBuffer.push_back(0);
				mControlBuffer.push_back(0);
			}
		}

		bool MidiInput::OpenPort(uint32_t port /*= 0 */) {
			// Check available ports.
			mPortCount = MidiIn->getPortCount();
			if ((mPortCount == 0) || ((port + 1) > mPortCount)) {
				std::cout << "No ports available!\n";
				return false;
			}

			mPort = port;
			mPortName = MidiIn->getPortName(mPort);
			MidiIn->openPort(mPort);

			// Set our callback function.This should be done immediately after
			// opening the port to avoid having incoming messages written to the
			// queue instead of sent to the callback function.
			MidiIn->setCallback(&RtMidiInCallback, this);

			// Don't ignore sysex, timing, or active sensing messages.
			MidiIn->ignoreTypes(false, false, false);

			return true;
		}

		void MidiInput::ClosePort() {
			MidiIn->cancelCallback();
			MidiIn->closePort();
		}

		//   void MidiInput::RtMidiInCallback( double deltatime, std::vector< unsigned char > *message, void *userData ) {
		//     ( ( MidiInput* ) userData )->ProcessMessage( deltatime, message );
		//   }


		//! ----  Creates a new midi message struct for each incoming midi message
		void MidiInput::ProcessMessage(double deltatime, std::vector<unsigned char> *message) {
			size_t NumOfBytes = message->size();

			MidiMessage newMsg;
			newMsg.Port = mPort;
			newMsg.Name = mPortName.c_str();
			if ((message->at(0)) >= MIDI_SYSEX) {
				newMsg.StatusCode = (MidiStatus)(message->at(0) & 0xFF);
				newMsg.Channel = 0;
			}
			else {
				newMsg.StatusCode = (MidiStatus)(message->at(0) & 0xF0);
				newMsg.Channel = (int)(message->at(0) & 0x0F) + 1;
			}

			switch (newMsg.StatusCode) {
			case MIDI_NOTE_ON: {
				newMsg.Pitch = (int)message->at(1);
				newMsg.Velocity = (int)message->at(2);
				mNotesBuffer[newMsg.Pitch] = newMsg.Velocity;
			} break;
			case MIDI_NOTE_OFF: {
				newMsg.Pitch = (int)message->at(1);
				newMsg.Velocity = (int)message->at(2);
				mNotesBuffer[newMsg.Pitch] = newMsg.Velocity;
			} break;
			case MIDI_CONTROL_CHANGE: {
				newMsg.Control = (int)message->at(1);
				newMsg.Value = (int)message->at(2);
				mControlBuffer[newMsg.Control] = newMsg.Value;
			} break;
			case MIDI_PROGRAM_CHANGE:
			case MIDI_AFTERTOUCH: {
				newMsg.Value = (int)message->at(1);
			} break;
			case MIDI_PITCH_BEND: {
				newMsg.Value = (int)(message->at(2) << 7) +
					(int)message->at(1); // msb + lsb
			} break;
			case MIDI_POLY_AFTERTOUCH: {
				newMsg.Pitch = (int)message->at(1);
				newMsg.Value = (int)message->at(2);
				mNotesBuffer[newMsg.Pitch] = newMsg.Value;
			} break;
			default: {

			} break;
			}
			if (&mMidiInCallback) {
				mMidiInCallback(newMsg);
			}
		}

		const std::vector<std::string> MidiInput::GetPortList()
		{
			std::vector<std::string> portList;
			mPortCount = MidiIn->getPortCount();
			for (unsigned int i = 0; i < mPortCount; ++i) {
				portList.push_back(MidiIn->getPortName(i));
			}
			return portList;
		}

		//------------------------------------------------------------------
		//  Midi Output
		//------------------------------------------------------------------


		MidiOutput::MidiOutput(std::string name /*= "Cinder-MIDI Client"*/)
			: Name(name), PortNumber(-1), IsVirtual(false), Bytes(3) {
			try {
				MidiOut = std::make_unique<RtMidiOut>();
			}
			catch (RtMidiError &error) {
				error.printMessage();
			}
		}

		const std::vector<std::string> MidiOutput::getPortList() {
			std::vector<std::string> Result;
			PortCount = MidiOut->getPortCount();;
			for (unsigned int i = 0; i < PortCount; ++i) {
				Result.push_back(MidiOut->getPortName(i));
			}
			return Result;
		}

		/// \section Connection

		/// Connect to an output port.
		/// Setting port = 0 will open the first available
		bool MidiOutput::openPort(unsigned int portNumber)
		{
			MidiOut->closePort();
			MidiOut->openPort(portNumber);
			CurrentPort = portNumber;
			Name = MidiOut->getPortName(portNumber);
			return true;
		}
		/// Close the port connection
		void MidiOutput::closePort()
		{
			MidiOut->closePort();
		}
		/// Close the port connection
		MidiOutput::~MidiOutput()
		{
			MidiOut->closePort();
		}

		/// \section Sending

		///
		/// midi events
		///
		/// number ranges:
		///		channel			1 - 16
		///		pitch			0 - 127
		///		velocity		0 - 127
		///		control value	0 - 127
		///		program value	0 - 127
		///		bend value		0 - 16383
		///		touch value		0 - 127
		///
		/// note:
		///		- a noteon with vel = 0 is equivalent to a noteoff
		///		- send velocity = 64 if not using velocity values
		///		- most synths don't use the velocity value in a noteoff
		///		- the lsb & msb for raw pitch bend bytes are 7 bit
		///
		/// references:
		///		http://www.srm.com/qtma/davidsmidispec.html
		///
		void MidiOutput::sendMessage(unsigned char status, unsigned char byteOne, unsigned char byteTwo)
		{
			assert(Bytes.size() == 3);
			Bytes[0] = status;
			Bytes[1] = byteOne;
			Bytes[2] = byteTwo;
			sendMessage(Bytes);
		}

		void MidiOutput::sendMessage(unsigned char status, unsigned char byteOne)
		{
			assert(Bytes.size() == 3);
			Bytes.resize(2);
			Bytes[0] = status;
			Bytes[1] = byteOne;
			sendMessage(Bytes);
			Bytes.resize(3); // restore invariant
		}

		void MidiOutput::sendMessage(std::vector<unsigned char>& bytes)
		{
			MidiOut->sendMessage(&bytes);
		}

		void MidiOutput::sendNoteOn(int channel, int pitch, int velocity)
		{
			sendMessage(MIDI_NOTE_ON + channel - 1, pitch, velocity);
		}
		void MidiOutput::sendNoteOff(int channel, int pitch, int velocity)
		{
			sendMessage(MIDI_NOTE_OFF + channel - 1, pitch, velocity);
		}
		void MidiOutput::sendControlChange(int channel, int control, int value)
		{
			sendMessage(MIDI_CONTROL_CHANGE + channel - 1, control, value);
		}
		void MidiOutput::sendProgramChange(int channel, int value)
		{
			sendMessage(MIDI_PROGRAM_CHANGE + channel - 1, value);
		}
		void MidiOutput::sendPitchBend(int channel, int value)
		{
			if (value >> 14 != 0)
			{
				std::cout << "[ERROR ci::midi::MidiOut::sendPitchBend] Pitch bend values must be less than " << (1 << 14) << std::endl;
			}
			// least significant 7 bits, most significant 7 bits (assuming 14 bit value)
			sendPitchBend(channel, value & 0x7F, (value >> 7) & 0x7F);
		}
		void MidiOutput::sendPitchBend(int channel, unsigned char lsb, unsigned char msb)
		{
			sendMessage(MIDI_PITCH_BEND, lsb, msb);
		}
		void MidiOutput::sendAftertouch(int channel, int value)
		{
			sendMessage(MIDI_AFTERTOUCH + channel - 1, value);
		}
		void MidiOutput::sendPolyAftertouch(int channel, int pitch, int value)
		{
			sendMessage(MIDI_POLY_AFTERTOUCH + channel - 1, pitch, value);
		}

		//------------------------------------------------------------------
		//  Midi Hub
		//------------------------------------------------------------------

	}
}