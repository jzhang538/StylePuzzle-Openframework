#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include <time.h>

#include "ofxXmlPoco.h"
#include "ofxThreadedImageLoader.h"
#include "ofThread.h"

#define NEURAL_STYLE_PATH "/Users/zhangjinghuai/Desktop/chainer-fast-neuralstyle"

//If Use GPU
#define PATH "/usr/local/bin:/usr/local/sbin:/Developer/NVIDIA/CUDA-7.5/bin:/usr/local/cuda/bin:/opt/local/bin:/opt/local/sbin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"
// same thing for "echo $LD_LIBRARY_PATH"
#define LD_LIBRARY_PATH "/usr/local/cuda/lib"
//If Don't Use GPU, Set To 0
#define GPU_ENABLED 0
#include "worker.h"

using namespace ofxCv;
using namespace cv;

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
    void interfaceDraw();
    
    //The width of input image needs to be larger than 640px to meet the design of interface (e.g. MS COCO Dataset)
    //image names/paths 
    string input_name="COCO1.jpg";
    string img_name=input_name;
    string bg_name="bg.jpg";//Recommend same size as input; if no need background, set to ""
    ofImage bg;
    ofImage bg_thumbnail;
    Mat bgMat;
    int bg_flag=0;
    
    
    //Settings related to style references.
    string style_names[3]={"edtaonisl","kanagawa","cubist"};
//   string style_names[6]={"kanagawa","starrynight","scream-style","candy_512_2_49000","edtaonisl","kandinsky_e2_full512"};
    //    string style_names[7]={"kanagawa","starrynight","candy_512_2_49000","edtaonisl","starry","kandinsky_e2_full512","scream-style"};
    int style_num=3;//At most 6 (due to CPU speed and height limited (my setting to meet with Mac))
    int style_flag[8];//8 is a number > 6
    string styleModels[8];
    ofImage stylized_imgs[8];
    ofImage thumb_imgs[8];//thumbnail_imgs
    

    //Set N threads to speed up the initialization process
    int worker_num=3;
    ThreadedObject workers[3];
    float last, t;
    float min_lapse = 3.0;;
    
    
    //Variables used to operate/stylize the display image
    Mat mask;
    Mat stylizedMat;
    Mat originalMat;
    Mat generatedMap;
    Mat UndoMat;
    ofImage Stylized_img;// Main image
    
    //Variables used to control the size of interface
    int Width,Height;
    int thumb_width;
    int thumb_height;
    int window_height;
    int box_w;
    int box_h;
    int y_offset;
    int border_width;
    
    //Event related variables
    int mode;
    vector<cv::Point> keyPoints;
    int selected=-1;//ID of style reference selected
    int order=0;//ID of current preview image
    
    string w;//Warnings
    string modeName;//Model name
    
    //Time Recorder
    clock_t start;
    clock_t end;
};
