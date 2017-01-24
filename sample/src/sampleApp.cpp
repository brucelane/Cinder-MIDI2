#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "CinderMidi.h"

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
    
    void midiListener(midi::MidiMessage msg);
    
    midi::MidiInput mInput;
    vector <int> notes;
    vector <int> cc;
    
    Font mFont;
    std::string status;
    
};

void MidiTestApp::midiListener(midi::MidiMessage msg){
    switch (msg.StatusCode)
    {
        case MIDI_NOTE_ON:
            notes[msg.Pitch] = msg.Velocity;
            status = "Pitch: " + toString(msg.Pitch) + "\n" +
            "Velocity: " + toString(msg.Velocity);
            break;
        case MIDI_NOTE_OFF:
            break;
        case MIDI_CONTROL_CHANGE:
            cc[msg.Control] = msg.Value;
            status = "Control: " + toString(msg.Control) + "\n" +
            "Value: " + toString(msg.Value);
            break;
        default:
            break;
    }
    
}

void MidiTestApp::setup()
{
    
    mInput.GetPortList();
    console() << "NUMBER OF PORTS: " << mInput.mPortCount << endl;
    for (int i = 0; i < mInput.mPortCount; i++)
    {
        console() << mInput.GetPortName(i) << endl;
    }
    mInput.OpenPort(0);
    
    mInput.mMidiInCallback = std::bind(&MidiTestApp::midiListener, this, std::placeholders::_1);
    
    
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



CINDER_APP( MidiTestApp, RendererGl )
