#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    //Variable Initialization
    Stylized_img.load(ofToDataPath(img_name));
    if(bg_name!="")
    {
        bg.load(bg_name);
        bg_thumbnail.load(bg_name);
    }
    
    box_w=400;//Instruction Box
    box_h=720;
    y_offset=30;//Notification Box
    Width=Stylized_img.getWidth();//Image
    Height=Stylized_img.getHeight();
    thumb_width=150;//Thumbnail
    thumb_height=60;
    border_width=5;
    
    window_height=max(box_h,Height)+y_offset;
    ofSetWindowShape(Width+box_w, window_height);
    cout<<Width<<" "<<Height<<endl;//Image Height
    stylizedMat=toCv(Stylized_img);
    UndoMat=Mat(cv::Size(Width,Height),CV_8UC3,Scalar(255,255,255));
    stylizedMat.copyTo(originalMat);
    
    bg.resize(Width, Height);
    bgMat=toCv(bg);
    bg_thumbnail.resize(thumb_width, thumb_height);
    
    mode=-1;
    for(int i=0;i<style_num;i++)
    {
        styleModels[i]=ofToDataPath(style_names[i]+".model");
    }
    modeName="";
}

//--------------------------------------------------------------
void ofApp::update(){
    //Mode -1 -> Before Initialization Mode
    if(mode==-1)
    {
        if(selected==98-48)//Press b to change background
        {
            if(bgMat.size()!=mask.size())
            {
                resize(bgMat, bgMat, cv::Size(Width, Height), 0, 0, INTER_CUBIC);
            }
            bgMat.copyTo(stylizedMat,mask);
            Stylized_img.update();
            selected=-1;
            //release memory
            mask.release();
        }
    }
    // Mode 2 -> Initialization Mode
    // Apply multi-threading to process the image.
    if(mode==2)
    {
        //Assign tasks
        t = ofGetElapsedTimef() - last;
        for(int i=0;i<style_num;i++)
        {
            if(style_flag[i]==0)
            {
                int flag=0;
                for(int j=0;j<worker_num;j++)
                {
                    if (workers[j].available() && t > min_lapse) {
                        flag=1;
                        workers[j].setStyleModel(img_name,styleModels[i],i);
                        workers[j].start();
                        last = ofGetElapsedTimef();
                    }
                    if(flag==1)
                    {
                        break;
                    }
                }
            }
        }
        
        //Store synthesized images
        for(int j=0;j<worker_num;j++)
        {
            if (workers[j].hasNew()) {
                int style_label=workers[j].getLabel();
                style_flag[style_label]=1;
            //stylized_img[style_label].save(ofToString(style_label)+".png");
                workers[j].reset();
                
                
                for (int i=0;i<style_num;i++)
                {
                    cout<<style_flag[i]<<" ";
                }
                cout<<endl;
            }
        }
        
        
        int finished=0;
        for (int i=0;i<style_num;i++)
        {
            if(style_flag[i]==0)
            {
                finished=1;break;
            }
        }
        //If finished initialization, stop threads and set the mode to Showing Mode.
        if(finished==0)
        {
//            end=clock();
//            cout<<"Time: "<<start-end<<endl;
            mode=0;//Go to showing mode, interaction prepared
            w="";
            for(int j=0;j<worker_num;j++)
            {
                workers[j].stop();
            }
            
            for (int j=0;j<style_num;j++)
            {
            while(!stylized_imgs[j].load(ofToDataPath("stylized_"+ofToString(j)+".png")))
                {
                    sleep(3);
                }
            thumb_imgs[j].load(ofToDataPath("stylized_"+ofToString(j)+".png"));
                thumb_imgs[j].resize(thumb_width,thumb_height);
            }
        }
    }
    else if(mode!=-1)
    {
        // Mode 1->0, conduct region-based image stylization. 
        if(selected>=1&&selected<=style_num)//stylization
        {
            stylizedMat.copyTo(UndoMat);
            generatedMap=toCv(stylized_imgs[selected-1]);
            
            if(generatedMap.size()!=mask.size())
            {
//            copyMakeBorder(generatedMap,generatedMap,1,1,1,1,BORDER_CONSTANT,Scalar(0));
                resize(generatedMap, generatedMap, cv::Size(Width, Height), 0, 0, INTER_CUBIC);
            }
            generatedMap.copyTo(stylizedMat,mask);
            
            Stylized_img.update();
            selected=-1;
            mask.release();
            generatedMap.release();
            
        }
        if(selected==103-48)//Gaussion Blur
        {
            stylizedMat.copyTo(UndoMat);
            GaussianBlur(stylizedMat, generatedMap, 7);
            
            generatedMap.copyTo(stylizedMat,mask);
            Stylized_img.update();
            selected=-1;
            mask.release();
            generatedMap.release();
        }
        if(selected==0)//Copy Original
        {
            originalMat.copyTo(stylizedMat,mask);
            Stylized_img.update();
            selected=-1;
            mask.release();
        }
        if(selected==106-48)//Copy Background
        {
            bgMat.copyTo(stylizedMat,mask);
            Stylized_img.update();
            selected=-1;
            mask.release();
        }
    }
}

//Draw Interface
void ofApp::interfaceDraw()
{
    //Instruction
    int paddingx=10;
    int paddingy=12;
    int lh=18;//Line Height
    int lcnt=0;//Line Count
    int corner_x=Width+paddingx;
    int corner_y=0+paddingy;
    
    ofSetColor(255,255,255);
    
    ofDrawBitmapString("Press i for initialization.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("-------After Initialization------", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("1: Press d to drawing mode.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("2: Press s to showing mode.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("3: Click/drag mouse to draw points to form polygon.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("(clockwise or anticlockwise, in Drawing mode)", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("4: Press c to clear points or p to pop.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("7:  Press 0-"+ofToString(style_num)+" to set back/stylize the selected region.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("8:  Press g to blur the selected region.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("9:  Press j to set back to background image.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("10: Press u to undo last operation.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("11: Press r to save img.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("12: Press o to reset img.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("13: Press z (left) or x (right) to preview images.", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    ofDrawBitmapString("(in showing mode.)", corner_x, corner_y+lh*lcnt);
    lcnt+=1;
    //instruction on mode -1
    if(mode==-1)
    {
        ofDrawBitmapString("-------Before Initialization------.", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("You Can Change Background!!!", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("1. Click/drag mouse to draw points to form polygon (mask).", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("(clockwise or anticlockwise, in Drawing mode)", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("2. Press b to change background after draw some points.", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("3. Press c to clear points or p to pop.", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("4. Press o to reset img.", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        ofDrawBitmapString("5. Press r to save img.", corner_x, corner_y+lh*lcnt);
        lcnt+=1;
        bg_thumbnail.draw(corner_x,corner_y+lh*lcnt);
    }
    //Thumbnails
    if(mode!=-1 && mode!=2)
    {
        for(int i=0;i<style_num;i++)
        {
            ofDrawBitmapString(style_names[i]+" "+ofToString(i+1), corner_x, corner_y+corner_y+lh*lcnt+60*i);
        thumb_imgs[i].draw(corner_x+box_w-thumb_width-paddingx*2,corner_y+corner_y+lh*lcnt+thumb_height*i-paddingy);
        }
    }
    
    //Invalid Area
//    ofSetColor(139,90,43);
    ofSetColor(128,128,128);
    if(Height>box_h)
    {
        ofFill();
       ofDrawRectangle(Width,box_h+y_offset,box_w,window_height-box_h);
    }
    else if(Height<box_h)
    {
        ofFill();
        ofDrawRectangle(0,Height+y_offset,Width,window_height-Height);
    }
    //Warnings
    ofSetColor(255,255,255);
    ofFill();
    ofDrawRectangle(0,0, 350, y_offset);
    ofSetColor(215,165,0);
    ofNoFill();
    ofSetLineWidth(border_width);
    ofDrawRectangle(0,0, 350, y_offset);
    
    ofSetColor(255,0,0);
    ofDrawBitmapString(w, 0+10, 18);
    //Title
    ofSetColor(139,90,43);
    ofFill();
    ofDrawRectangle(350,0,Width-150-350, y_offset);
    ofSetColor(215,165,0);
    ofNoFill();
    ofSetLineWidth(border_width);
    ofDrawRectangle(350,0,Width-150-350, y_offset);
    
    ofSetColor(176,224,230);
    ofDrawBitmapString("StylePuzzle", 350+10, 18);
    
    //Mode
    if(mode==1)
    {
        modeName="Drawing";
    }
    else if(mode==2)
    {
        modeName="Initialization";
    }
    else if(mode==0)
    {
        modeName="Showing";
    }
    else if(mode==3)
    {
        modeName="Detection";
    }
    ofSetColor(255,255,255);
    ofFill();
    ofDrawRectangle(Width-150,0, 150, y_offset);
    ofSetColor(215,165,0);
    ofNoFill();
    ofSetLineWidth(border_width);
    ofDrawRectangle(Width-150,0, 150, y_offset);
    
    ofSetColor(0,0,0);
    ofDrawBitmapString(modeName, Width-150+20, 18);
}
//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    ofSetColor(255);
    
    //Show corresponding image (stylized image references or target image)
    if(order==0)
    {Stylized_img.draw(0,30);}
    else
    {
        stylized_imgs[order-1].draw(0,30);
    }
    
    //Show points
    ofFill();
    if(mode==1||mode==-1)
    {
        ofSetColor(0, 255, 0);
        for(int i = 0; i < keyPoints.size(); i++){
            ofDrawCircle(keyPoints[i].x, keyPoints[i].y+y_offset, 5);
        }
    }
    interfaceDraw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //Operation Before Initialization Mode
    if(mode==-1)
    {
        //Press the key i to initialize
        if(key=='i')
        {
            for(int i=0;i<style_num;i++)
            {
                style_flag[i]=0;//Indicates the stylized image references haven't been generated
            }
            //start=clock();
            mode=2;
            for(int i=0;i<worker_num;i++)
            {
                workers[i].setIndex(i+1);
            }
            stylizedMat.copyTo(UndoMat);
            stylizedMat.copyTo(originalMat);
            if(bg_flag==1)
            {
                img_name="synthesized.png";
                Stylized_img.save(img_name);
            }
            w="";
//            if(bg_name!="")
//            {
//                bg.clear();
//                bg_thumbnail.clear();
//                bgMat.release();
//            }
        }
        else if(key=='b')//Set to background
        {
            if(keyPoints.size()<3)
            {
                w="Points Less than 3.";
            }
            else if(bg_name=="")
            {
                w="No backgound image in the path.";
            }
            else
            {
                bg_flag=1;
                selected=int(key)-48;
                mask=cv::Mat(cv::Size(Width,Height), CV_8UC3,Scalar(255,255,255));
                fillConvexPoly(mask, keyPoints.data(), keyPoints.size(), Scalar(0,0,0));
                keyPoints.clear();
                w="";
            }
        }
        else if(key=='c')
        {
            keyPoints.clear();
            w="";
        }
        //Pop last points
        else if(key=='p')
        {
            if(keyPoints.size()!=0){
                vector<cv::Point>::iterator it = keyPoints.end()-1;
                keyPoints.erase(it);
                w="";
            }
            else
                w="No key points.";
        }
        else if(key=='o')
        {
            originalMat.copyTo(stylizedMat);
            Stylized_img.update();
            w="";
        }
        //If user want to operation on the changed image, need to save first ('r')
        else if(key=='r')
        {
            img_name="synthesized.png";
            Stylized_img.save(img_name);
            w="";
        }
    }
    else if(mode==2)//System Initialization
    {
        w="System Initializing, Please Waiting.";
    }
    else
    {
        //Left move in Showing Mode
        if(mode==0 && key=='z')
        {
            if(order==0)
            {
                order=style_num;
            }
            else
                order-=1;
            w="";
        }
        //Right move in Showing Mode
        else if(mode==0 && key=='x')
        {
            if(order==style_num)
            {
                order=0;
            }
            else
                order+=1;
            w="";
        }
        else if(key=='z' || key=='x')
        {
            w="Not in Showing Mode.";
        }
        
        //Showing Mode
        if(key=='s')
        {
            if(mode==0)
            {
                w="Already in Showing Mode.";
            }
            else
            {
                mode=0;
                w="";
            }
        }
        //Drawing Mode
        if(key=='d')
        {
            if(mode==1)
            {
                w="Already in Drawing Mode.";
            }
            else
            {
                mode=1;
                order=0;
                w="";
            }
        }
        //Save img
        if(key=='r')//save img
        {
            Stylized_img.save("result.png");
            w="";
        }
        //Clear points
        if(key=='c')
        {
            if(mode==1)
            {
                keyPoints.clear();
                w="";
            }
            else
                w="Not in Drawing Mode.";
        }
        //Pop points
        if(key=='p')
        {
            if(keyPoints.size()!=0 && mode==1)
            {
                vector<cv::Point>::iterator it = keyPoints.end()-1;
                keyPoints.erase(it);;
                w="";
            }
            else
                w="Not in Drawing Mode or no key points.";
        }
        //Set to original image
        if(key=='o')
        {
            originalMat.copyTo(stylizedMat);
            Stylized_img.update();
            w="";
        }
        //Undo previous operation on image
        if(key=='u')
        {
            UndoMat.copyTo(stylizedMat);
            Stylized_img.update();
            w="";
        }
        //Stylization or blur or set back to background (if any)
        if(key=='1' || key=='2' || key=='3' || key=='4'|| key=='5' || key=='6'||key=='7'||key=='8'||key=='0'||key=='g'||key=='j')
        {
            if(keyPoints.size()<3)
            {
                w="Points Less than 3.";
            }
            else if(key=='j' and bg_name=="")
            {
                w="No background image.";
            }
            else
            {
                selected=int(key)-48;
                mask=Mat::zeros(cv::Size(Width,Height), CV_8UC3);
                fillConvexPoly(mask, keyPoints.data(), keyPoints.size(), Scalar(255,255,255));
                mode=0;
                keyPoints.clear();
                w="";
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if(mode==1||mode==-1)
    {
        if(x>=0&&x<Width&&y>=y_offset&&y<y_offset+Height){
            cv::Point p(x,y-y_offset);
            keyPoints.push_back(p);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    //click points
    if(mode==1||mode==-1)
    {
        if(x>=0&&x<Width&&y>=y_offset&&y<y_offset+Height){
            cv::Point p(x,y-y_offset);
            keyPoints.push_back(p);
        }
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
