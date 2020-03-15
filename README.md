Cinder-MIDI2
============

Alternative approach to interfacing RtMidi lib in Cinder
Using
============
 
1. Create void function in your app, that will be reacting to incoming midi-messages:

 ```cpp
 
 void MidiTestApp::midiListener(midi::Message msg){
	switch (msg.status)
	{
	case MIDI_NOTE_ON:
		notes[msg.pitch] = msg.velocity;
		status = "Pitch: " + toString(msg.pitch) + "\n" + 
			"Velocity: " + toString(msg.velocity);
		break;
	case MIDI_NOTE_OFF:
		break;
	case MIDI_CONTROL_CHANGE:
		cc[msg.control] = msg.value;
		status = "Control: " + toString(msg.control) + "\n" + 
			"Value: " + toString(msg.value);
		break;
	default:
		break;
	}
 ```

2. connect signal from MidiInput instance to your function like so:

 ```mInput.midiSignal.connect( [this](midi::Message msg){ MidiTestApp::midiListener( msg ); }); ```

 this will push the incoming midi messages safely over onto the main thread for consumption. You can however also get a signal directly from the midi thread if you need like so: 

 ```mInput.midiThreadSignal.connect( [this](midi::Message msg){ MidiTestApp::midiListener( msg ); }); ```

3. if you are using Windows, you need to add ```winmm.lib``` to your linker

4. add __WINDOWS_MM__ to preprocessor

============


###Positive changes>
**Using signals:** Possibly THE biggest change, is that now not only block runs stable and fast, but there's no need for processing midi inside update() function. I used boost::signals2 for sending signals with processed midi messages out of instance of MidiInput class. So after initialization, you just connect signal to some void function in your app which is used as listener and does automatically all you need your midi-messages to do.

**Processing midi-messages** is straight copy of ofxMidi format, because I was familiar with it, and I thought it might be familiar to all people moving to Cinder from OF, not to mention it's internal structure is clear, and well designed. I also think this might goes to some sort of credits to Dan Wilcox & Theo Watson (original authors of ofxMidi) in form of comment in header file.

**RtMidi 2.0.1** - made upgrade to newer version of base library, as old version of MidiBlock is still using using 1.0.something. I have not found any particularly noticeable changes in interfacing, but changelog of RtMidi is quite long so there's got to be something worth upgrading :)

###Negative stuff>

**Callback to RtMidiInput:** this is a problem that I tried to solve while I was working on midi-sequencer. I needed to switch virtual midi channels a lot during development, and RtMidi was still complaining, that callback is already set. That is because callback is set inside openPort() function, but I think it would be (somehow) possible to set callback once per initialization and just let midi-messages fly through it once desired port is opened. I remember I tried to do it this way and than I rolled back to current version for some reason.



**iOS version:** OF guys are using some other lib for iOS (coremidi???) for processing midi, because RtMidi does not support it. 

**Missing "openPort(string nameOfThePort)" function:** this is just something I was using before by some terrible workaround but it was on my to do list. Now I found some way how to do this using C++ 11 (namely std::string.find), so in theory, it should work even when you specify just fragment of port name (like "BCR" instead of "BCR2000 MIDI INPUT").


Thanks to [Thomas Sanchez Lengeling](https://github.com/ThomasLengeling) for providing testing and changes to make block work on OSX!

###TODO:

**Auto-Reconnect:** just some little safety function, which tries to reconnect device after it finds that it was disconnected, so you do not have to restart whole application. I had some issues with this kind of situation with using Ableton couple of years back, so this is just for convenience. It should be disabled by default tho. 
