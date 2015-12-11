#include "ofApp.h"

float Rad = 600;		//Cloud raduis parameter
float Vel = 1.1;		//Cloud points velocity parameter
int bandRad = 2;		//Band index in spectrum, affecting Rad value
int bandVel = 100;		//Band index in spectrum, affecting Vel value

const int n = 300;		//Number of cloud points	

						//Offsets for Perlin noise calculation for points
float tx[n], ty[n];
ofPoint p[n];			//Cloud's points positions

float time0 = 0;		//Time value, used for dt computing

float distList[4] = { 0,15,30,45 };
int currDist = 2;

int cursorX = 0;
int cursorY = 0;

const int N = 512;		//Number of bands in spectrum
float spectrum[N];	 //Smoothed spectrum values

static int index = 0;
float avg_power = 0.0f;

int counter = 0;

int rgbColour[3];

int incColour = 0;
int decColour = 0;

int color = 0;


//--------------------------------------------------------------
void ofApp::setup(){

	rgbColour[0] = 255;
	rgbColour[1] = 0;
	rgbColour[2] = 0;

	srand((unsigned int)time((time_t *)NULL));
	
	ofSetVerticalSync(true);
	ofSetCircleResolution(80);
	//ofBackground(54, 54, 54);
	ofBackground(0, 0, 0);
	
	// 0 output channels, 
	// 2 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	streamIn.listDevices();
	
	//if you want to set a different device id 
	streamIn.setDeviceID(3); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
	
	int bufferSize = 512;
	
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;

	streamIn.setup(this, 0, 2, 44100, bufferSize, 4);

	for (int i = 0; i<N; i++) {
		spectrum[i] = 0.0f;
	}

	//Initialize points offsets by random numbers
	for (int j = 0; j<n; j++) {
		tx[j] = ofRandom(0, 1000);
		ty[j] = ofRandom(0, 1000);
	}

	ofHideCursor();

}

//--------------------------------------------------------------
void ofApp::update(){
	/*
	//lets scale the vol up to a 0-1 range 
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

	//lets record the volume into an array
	volHistory.push_back( scaledVol );
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
	*/

	if (counter == 720)
	{
		counter = 0;
	}
	else
	{
		counter++;
	}

	color++;
	if (color == 766)
	{
		color = 0;
	}

	if (color < 255)
	{
		decColour = 0;
	}
	else if(color < 510)
	{
		decColour = 1;
	}
	else
	{
		decColour = 2;
	}

	incColour = decColour == 2 ? 0 : decColour + 1;

	rgbColour[decColour] -= 1;
	rgbColour[incColour] += 1;
	
}

//--------------------------------------------------------------
void ofApp::draw(){

	//Draw spectrum
	/*
	ofSetColor( 255, 255, 255 );
	for (int i=0; i<N; i++) {
	ofRect( 10 + i * 5, ofGetHeight(), 3, -spectrum[i] * 100 );
	}
	*/


	

	/* draw the FFT
	for (int i = 1; i < (int)(BUFFER_SIZE / 2); i++) {
		ofLine(200 + (i * 8), 400, 200 + (i * 8), 400 - magnitude[i] * 10.0f);
	}
	*/

	ofPushMatrix();
	ofTranslate(cursorX, cursorY);
	ofRotate(counter / 2);

	//Draw points
	//ofSetColor(30, 100, 250);
	//ofSetColor(255, 255, 255);
	ofSetColor(rgbColour[0], rgbColour[1], rgbColour[2]);

	ofFill();
	for (int i = 0; i<n; i++) {
		ofCircle(p[i], 2);
	}

	//Draw lines between near points
	float dist = distList[currDist];	//Threshold parameter of distance
	for (int j = 0; j<n; j++) {
		for (int k = j + 1; k<n; k++) {
			if (ofDist(p[j].x, p[j].y, p[k].x, p[k].y)
				< dist) {
				ofLine(p[j], p[k]);
			}
		}
	}

	//Restore coordinate system
	ofPopMatrix();
		
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){	
	
	for (int i = 0; i < bufferSize; i++) {
		left[i] = input[i * 2] / 30;
		right[i] = input[i * 2 + 1] / 30;
	}

	bufferCounter++;


	if (index < 80)
		index += 1;
	else
		index = 0;

	myfft.powerSpectrum(0, (int)BUFFER_SIZE / 2, &left[0], BUFFER_SIZE, &magnitude[0], &phase[0], &power[0], &avg_power);


	/* start from 1 because mag[0] = DC component */
	/* and discard the upper half of the buffer */
	for (int j = 1; j < BUFFER_SIZE / 2; j++) {
		freq[index][j] = magnitude[j];
	}



	for (int i = 0; i<N; i++) {
		spectrum[i] *= 0.97;	//Slow decreasing
		spectrum[i] = max(spectrum[i], magnitude[i]);
	}

	//Update particles using spectrum values

	//Computing dt as a time between the last
	//and the current calling of update() 	
	float time = ofGetElapsedTimef();
	float dt = time - time0;
	dt = ofClamp(dt, 0.0, 0.1);
	time0 = time; //Store the current time	

				  //Update Rad and Vel from spectrum
				  //Note, the parameters in ofMap's were tuned for best result
				  //just for current music track
	Rad = ofMap(spectrum[bandRad], 1, 3, 400, 800, true);
	Vel = ofMap(spectrum[bandVel], 0, 0.1, 0.05, 0.5);

	//Update particles positions
	for (int j = 0; j<n; j++) {
		tx[j] += Vel * dt;	//move offset
		ty[j] += Vel * dt;	//move offset
							//Calculate Perlin's noise in [-1, 1] and
							//multiply on Rad
		p[j].x = ofSignedNoise(tx[j]) * Rad;
		p[j].y = ofSignedNoise(ty[j]) * Rad;
	}
	
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){ 
	if( key == 's' ){
		streamIn.start();
	}
	
	if( key == 'e' ){
		streamIn.stop();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	cursorX = x;
	cursorY = y;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (currDist == 3)
	{
		currDist = 0;
	}
	else
	{
		currDist++;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

