#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

#include "MidiHeaders.h"
#include "cinder/Font.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define TAU M_PI*2

class MidiTestApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void initPads();
	
	void midiListener( midi::Message msg );
    void midiThreadListener( midi::Message msg );

	midi::Input mInput;
	vector <int> notes;
	vector <int> cc;

	Font mFont;
	std::string status;
	
};

void MidiTestApp::midiThreadListener( midi::Message msg )
{
    // This will be called from a background midi thread
}

void MidiTestApp::midiListener( midi::Message msg )
{
    // This will be called on on the main thread and
    // safe to use with update and draw.
    
	switch (msg.status)
	{
	case MIDI_NOTE_ON:
		notes[msg.pitch] = msg.velocity;
		status = "Pitch: " + toString(msg.pitch) + "\n" + 
			"Velocity: " + toString(msg.velocity);
		break;
	case MIDI_NOTE_OFF:
        notes[msg.pitch] = 0;
		break;
	case MIDI_CONTROL_CHANGE:
		cc[msg.control] = msg.value;
		status = "Control: " + toString(msg.control) + "\n" + 
			"Value: " + toString(msg.value);
		break;
	default:
		break;
	}

}

void MidiTestApp::setup()
{
	mInput.listPorts();
	console() << "NUMBER OF PORTS: " << mInput.getNumPorts() << endl;
    
	if (mInput.getNumPorts() > 0) 
	{
		for (int i = 0; i < mInput.getNumPorts(); i++)
			console() << mInput.getPortName(i) << endl;
		
		mInput.openPort(0);
        
        // Connect midi signal to our callback function
        // This connects to our main thread
		mInput.midiSignal.connect( [this](midi::Message msg){ midiListener( msg ); });
        
        // Optionally, this connects directly to the midi thread
        mInput.midiThreadSignal.connect( [this](midi::Message msg){ midiThreadListener( msg ); });
    }

	for (int i = 0; i < 127; i++)
	{
		notes.push_back(0);
		cc.push_back(0);
	}
    
	mFont = Font("Arial", 25);
}

void MidiTestApp::mouseDown( MouseEvent event )
{
}

void MidiTestApp::update()
{
}

void MidiTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::pushMatrices();
	gl::translate(getWindowCenter());
    
	for (int i = 0; i < notes.size(); i++)
	{
		float x = 200*sin((i*2.83) * M_PI / 180);
		float y = 200*cos((i*2.83) * M_PI / 180);
		float lx = (200 - cc[i])*sin((i*2.83) * M_PI / 180);
		float ly = (200 - cc[i])*cos((i*2.83) * M_PI / 180);

		gl::color(Color(1,1,1));
		gl::drawStrokedCircle(vec2(x, y), 5+(notes[i]/4));
		gl::drawLine(vec2(x, y), vec2(lx, ly));
		gl::color(Color(notes[i], notes[i], notes[i]));
		gl::drawSolidCircle(vec2(x, y), 5 + (notes[i] / 4));
		
	}
	
	
	gl::popMatrices();
	gl::drawStringCentered(status, getWindowCenter(), Color(1,1,1), mFont);
}



CINDER_APP( MidiTestApp, RendererGl( RendererGl::Options().msaa( 4 ) ), [](App::Settings* settings)
{
    settings->setWindowSize( 600, 600 );
} )
