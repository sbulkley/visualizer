#pragma once

#include "ofMain.h"
#include "fft.h"

#define BUFFER_SIZE 512
#define NUM_WINDOWS 80

class ofApp : public ofBaseApp{
	
	public:
		
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
				
		void audioIn(float * input, int bufferSize, int nChannels);
	
		vector <float> left;
		vector <float> right;
		vector <float> volHistory;
		
		int 	bufferCounter;
		int 	drawCounter;
		
		float smoothedVol;
		float scaledVol;
		
		ofSoundStream streamIn;

		fft		myfft;

		float magnitude[BUFFER_SIZE];
		float phase[BUFFER_SIZE];
		float power[BUFFER_SIZE];

		float freq[NUM_WINDOWS][BUFFER_SIZE / 2];
		float freq_phase[NUM_WINDOWS][BUFFER_SIZE / 2];

		
};
