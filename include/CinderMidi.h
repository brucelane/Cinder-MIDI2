#pragma once
/*  ====================================================================
    File: MidiPlatform.h
    Date: 02/10/2015
    Revision:
    Creator: Hunter Luisi
    Notice: (C) Copyright 2015 by Hunter Luisi.  All Rights Reserved.
    ==================================================================== */

#include <stdint.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include "cinder/app/App.h"

#if defined( CINDER_COCOA )
#define __MACOSX_CORE__
#elif defined( CINDER_MSW ) | defined( CINDER_WINRT)
#define __WINDOWS_MM__
#else
#define __LINUX_ALSASEQ__
#endif

#define __CINDER_MIDI__

#include "RtMidi.h"

// #define internal static
// #define local_persist static
// #define global_variable static
// 
// typedef int8_t i8;
// typedef int16_t i16;
// typedef int32_t i32;
// typedef int64_t i64;
// 
// typedef uint8_t ui8;
// typedef uint16_t ui16;
// typedef uint32_t ui32;
// typedef uint64_t ui64;
// 
// typedef size_t size;
// 
// typedef float r32;
// typedef double r64;
// 
// typedef int32_t b32;

typedef unsigned int MidiStatus;
enum MidiStatus_ {

  MIDI_UNKNOWN = 0x00,

  // channel voice messages
  MIDI_NOTE_OFF = 0x80,
  MIDI_NOTE_ON = 0x90,
  MIDI_CONTROL_CHANGE = 0xB0,
  MIDI_PROGRAM_CHANGE = 0xC0,
  MIDI_PITCH_BEND = 0xE0,
  MIDI_AFTERTOUCH = 0xD0,	// aka channel pressure
  MIDI_POLY_AFTERTOUCH = 0xA0,	// aka key pressure

  // system messages
  MIDI_SYSEX = 0xF0,
  MIDI_TIME_CODE = 0xF1,
  MIDI_SONG_POS_POINTER = 0xF2,
  MIDI_SONG_SELECT = 0xF3,
  MIDI_TUNE_REQUEST = 0xF6,
  MIDI_SYSEX_END = 0xF7,
  MIDI_TIME_CLOCK = 0xF8,
  MIDI_START = 0xFA,
  MIDI_CONTINUE = 0xFB,
  MIDI_STOP = 0xFC,
  MIDI_ACTIVE_SENSING = 0xFE,
  MIDI_SYSTEM_RESET = 0xFF
};

//---- Novation LaunchPAD control bits ----//
#define LP_COPY_BIT_OFF (0 << 2)
#define LP_COPY_BIT_ON (1 << 2)
#define LP_CLEAR_BIT_OFF (0 << 3)
#define LP_CLEAR_BIT_ON (1 << 3)
#define LP_COPY_AND_CLEAR_OFF (LP_COPY_BIT_OFF | LP_CLEAR_BIT_OFF)
#define LP_COPY_AND_CLEAR_ON (LP_COPY_BIT_ON | LP_CLEAR_BIT_ON)
#define LP_GREEN_LOWBIT_OFF (0 << 4)
#define LP_GREEN_HIGHBIT_OFF (0 << 5)
#define LP_GREEN_LOWBIT_ON (1 << 4)
#define LP_GREEN_HIGHBIT_ON (1 << 5)
#define LP_RED_LOWBIT_OFF (0 << 0)
#define LP_RED_HIGHBIT_OFF (0 << 1)
#define LP_RED_LOWBIT_ON (1 << 0)
#define LP_RED_HIGHBIT_ON (1 << 1)
#define LP_AMBER_LOWBITS_OFF ( LP_GREEN_LOWBIT_OFF | LP_RED_LOWBIT_OFF )
#define LP_AMBER_HIGHBITS_OFF ( LP_GREEN_HIGHBIT_OFF | LP_RED_HIGHBIT_OFF )
#define LP_AMBER_LOWBITS_ON ( LP_GREEN_LOWBIT_ON | LP_RED_LOWBIT_ON )
#define LP_AMBER_HIGHBITS_ON ( LP_GREEN_HIGHBIT_ON | LP_RED_HIGHBIT_ON )

typedef unsigned int LpControl;
enum LpControl_ {
  //---- Buffer settings --//
  LpControl_Copy_On = LP_COPY_BIT_ON,
  LpControl_Copy_Off = LP_COPY_BIT_OFF,
  LpControl_Clear_On = LP_CLEAR_BIT_ON,
  LpControl_Clear_Off = LP_CLEAR_BIT_OFF,
  LpControl_CopyAndClear_Off = ( LP_COPY_BIT_OFF | LP_CLEAR_BIT_OFF ),
  LpControl_CopyAndClear_On = ( LP_COPY_BIT_ON | LP_CLEAR_BIT_ON ),
  //---- Green color settings ----//
  LpControl_Green_Off = ( LP_GREEN_HIGHBIT_OFF | LP_GREEN_LOWBIT_OFF | LP_COPY_AND_CLEAR_ON ),
  LpControl_Green_Low = ( LP_GREEN_HIGHBIT_OFF | LP_GREEN_LOWBIT_ON | LP_COPY_AND_CLEAR_ON ),
  LpControl_Green_Med = ( LP_GREEN_HIGHBIT_ON | LP_GREEN_LOWBIT_OFF | LP_COPY_AND_CLEAR_ON ),
  LpControl_Green_High = ( LP_GREEN_HIGHBIT_ON | LP_GREEN_LOWBIT_ON | LP_COPY_AND_CLEAR_ON ),
  //---- Green color settings ----//
  LpControl_Red_Off = ( LP_RED_HIGHBIT_OFF | LP_RED_LOWBIT_OFF | LP_COPY_AND_CLEAR_ON ),
  LpControl_Red_Low = ( LP_RED_HIGHBIT_OFF | LP_RED_LOWBIT_ON | LP_COPY_AND_CLEAR_ON ),
  LpControl_Red_Med = ( LP_RED_HIGHBIT_ON | LP_RED_LOWBIT_OFF | LP_COPY_AND_CLEAR_ON ),
  LpControl_Red_High = ( LP_RED_HIGHBIT_ON | LP_RED_LOWBIT_ON | LP_COPY_AND_CLEAR_ON ),
  //---- Amber color settings ----//
  LpControl_Amber_Off = ( LP_AMBER_HIGHBITS_OFF | LP_AMBER_LOWBITS_OFF | LP_COPY_AND_CLEAR_ON ),
  LpControl_Amber_Low = ( LP_AMBER_HIGHBITS_OFF | LP_AMBER_LOWBITS_ON | LP_COPY_AND_CLEAR_ON ),
  LpControl_Amber_Med = ( LP_AMBER_HIGHBITS_ON | LP_AMBER_LOWBITS_OFF | LP_COPY_AND_CLEAR_ON ),
  LpControl_Amber_High = ( LP_AMBER_HIGHBITS_ON | LP_AMBER_LOWBITS_ON | LP_COPY_AND_CLEAR_ON )
};


namespace cinder { namespace midi {

  //! ---- Structure for the passing of midi messages ---- !//
  struct MidiMessage {
    int32_t Port;
    int32_t Channel;
    int32_t StatusCode;
    int32_t ByteOne;
    int32_t ByteTwo;
    int32_t Value;			  //! depends on message status type
    uint32_t Pitch;			  //! 0 - 127
    uint32_t Velocity;		//! 0 - 127
    uint32_t Control;		  //! 0 - 127
    float TimeStamp;
    std::string Name;

    MidiMessage& copy( const  MidiMessage& other );
  };

  //! ---- Structure for Midi Input ---- !//
  struct MidiInput {
    uint32_t mPort;      //! -1 if the port isn't open
    uint32_t mPortCount;        // NOTE(hunter): is this needed?
    std::string mPortName;  //! Set after opening a port

    //! A vector of ALL the possible incoming midi notes messages.  
    //! Each vector index corresponds with midi note number (aka the note's pitch) from 0 to 127.
    //! Each value corresponds with the note's velocity (aka the note's volume), also from 0 to 127
    //! If the velocity is greater > 0 then the note is considered on, otherwise it's off.
    //! See this image for more details: http://imgbin.org/index.php?page=image&id=22351
    std::vector <uint32_t> mNotesBuffer;

    //! A vector of ALL the possible incoming midi controller messages.  
    //! Each vector index corresponds with midi control number (aka Controller Name) from 0 to 127.
    //! Each value corresponds with the control's value, also from 0 to 127.  If the value is 
    //! greater > 0 (usally 127 for most controls) then the control is considered on (or set), otherwise it's off.
    //! //! See this website for more details: http://www.indiana.edu/~emusic/cntrlnumb.html
    std::vector <uint32_t> mControlBuffer;

    // TODO(hunter): Better define after touch messages? This may not be needed as 
    // they should already show as velocity in the notes buffer.

    MidiInput();                           

    bool OpenPort( uint32_t CurrentPort = 0 );  //! default's to port 0
    void ClosePort();                      //! Closes port AND removes RtMidi callback
    
    //! Cinder-MidiIn Callback
    //! You can connect (bind) the signal from the MidiInput instance 
    //! to a Cinder main app function as follows:
    //! myVarName.MidiInCallback.connect( boost::bind( &myApp::myFuncName, this, _1 ) );
    //! 
    //! Your callback function should look something like this:
    //! 
    //! void myApp::midiInput( midi::MidiMessage msg ) {
    //!     switch ( msg.StatusCode ) {
    //!         case MIDI_NOTE_OFF:
    //!         case MIDI_NOTE_ON: {
    //!              // Handle note on and off
    //!              //console() << msg.Name << "Pitch: " << toString( msg.Pitch ) << ", Velocity: " << toString( msg.Velocity ) << std::endl;
    //!         } break;
    //!         case MIDI_CONTROL_CHANGE: {
    //!             // Handle control change
    //!             //console() << msg.Name << " - Control: " << toString( msg.Control ) << ", Value: " << toString( msg.Value ) << std::endl;
    //!         } break;
    //!         case MIDI_PROGRAM_CHANGE:
    //!         case MIDI_AFTERTOUCH: {
    //!             // handle program change
    //!         } break;
    //!         case MIDI_PITCH_BEND: {
    //!             // handle pitch bend
    //!         } break;
    //!         case MIDI_POLY_AFTERTOUCH: {
    //!             // handle after touch
    //!         } break;
    //!         default: {
    //!             // you should never get here
    //!         } break;
    //!     }
    //! }
    //boost::signals2::signal<void( MidiMessage )> mMidiInCallback;
    std::function<void( MidiMessage )> mMidiInCallback;
    
    //! Get a vector of output port names.
    //! Each vector index corresponds with the name's port number.
    //! Note: the order may change when new devices are added/removed.
    const std::vector<std::string> GetPortList();
    
    //! Convenience functions to RtMidi's API
    inline const std::string GetPortName( uint32_t Port ) const { return MidiIn->getPortName( Port ); }
    inline void IgnoreTypes( bool sysex, bool time, bool midisense ) const { MidiIn->ignoreTypes( sysex, time, midisense ); }

  protected:
    std::unique_ptr<RtMidiIn> MidiIn;
    void ProcessMessage( double deltatime, std::vector<unsigned char> *message );
    inline static void RtMidiInCallback( double deltatime, std::vector< unsigned char > *message , void *userData )  {
      ( ( MidiInput* ) userData )->ProcessMessage( deltatime, message );
    }

  };

  struct MidiOutput {
    uint32_t CurrentPort;
    uint32_t PortCount;
    std::string Name;

   /// Set the output client name (optional).
    MidiOutput( std::string name = "Cinder-MIDI Client" );
    
    ~MidiOutput();

    /// \section Global Port Info

    //! Get a vector of output port names.
    //! Each vector index corresponds with the name's port number.
    //! Note: the order may change when new devices are added/removed.
    const std::vector<std::string> getPortList();

    /// Get the number of output ports
    int getNumPorts() const { return MidiOut->getPortCount(); };

    /// Get the name of an output port by it's number
    /// \return "" if number is invalid
    const std::string getPortName( uint32_t portNumber ) const { return MidiOut->getPortName( portNumber ); };

    /// \section Connection

    /// Connect to an output port.
    /// Setting port = 0 will open the first available
    bool openPort( uint32_t portNumber = 0 );

    /// Create and connect to a virtual output port (MacOS and Linux ALSA only).
    /// allows for connections between software
    /// note: a connected virtual port has a portNum = -1
    ///	note: an open virtual port ofxMidiOut object cannot see it's virtual
    ///       own virtual port when listing ports
    ///
    bool openVirtualPort( std::string const& portName = "Cinder-MIDI Virtual Output" );

    /// Close the port connection
    void closePort();

    /// Get the port number if connected.
    /// \return -1 if not connected or this is a virtual port
    int getPort() const;

    /// Get the connected output port name
    /// \return "" if not connected
    std::string getName() const;
    bool isOpen() const;
    bool isVirtual() const;

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
    ///		- a note on with vel = 0 is equivalent to a note off
    ///		- send velocity = 64 if not using velocity values
    ///		- most synths don't use the velocity value in a note off
    ///		- the lsb & msb for raw pitch bend bytes are 7 bit
    ///
    /// references:
    ///		http://www.srm.com/qtma/davidsmidispec.html
    ///
    void sendNoteOn( int channel, int pitch, int velocity = 64 );
    void sendNoteOff( int channel, int pitch, int velocity = 64 );
    void sendControlChange( int channel, int control, int value );
    void sendProgramChange( int channel, int value );
    void sendPitchBend( int channel, int value );
    void sendPitchBend( int channel, unsigned char lsb, unsigned char msb );
    void sendAftertouch( int channel, int value );
    void sendPolyAftertouch( int channel, int pitch, int value );

    /// Low level access
    void sendMessage( std::vector<unsigned char>& bytes );
    void sendMessage( unsigned char status, unsigned char byteOne );
    void sendMessage( unsigned char status, unsigned char byteOne, unsigned char byteTwo );

    static bool sVerboseLogging;

    protected:
    std::unique_ptr<RtMidiOut> MidiOut;
    bool IsVirtual; ///< mIsVirtual => port is open as virtual port
    const char* PortName;
    int PortNumber; ///< mPortNumber == -1 => port is not open or is open as virtual port
    // Invariant: mIsVirtual => mPortNumber == -1
    /// Vector used to create midi messages. This should be kept at a length of 3
    /// and may be wiped at any point by any function. It is used by sendMessage( , , )
    std::vector<unsigned char> Bytes;
    // Invariant: mBytes.size() == 3
  };


  }
}