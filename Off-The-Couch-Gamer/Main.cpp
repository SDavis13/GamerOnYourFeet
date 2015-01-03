#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgProc/imgProc.hpp>
#include <string>
#define WINVER 0x0500
#include <windows.h>

using namespace cv;
using namespace std;

//functions for pressing and releasing keys
void presskey(int keycode)
{
	if (keycode >= 0)
	{
		// This structure will be used to create the keyboard
		// input event.
		INPUT ip;

		// Set up a generic keyboard event.
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0; // hardware scan code for key
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		// Press the "A" key
		ip.ki.wVk = keycode; // virtual-key code for the "a" key
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));
	}
}
void releasekey(int keycode)
{
	if (keycode>=0)
	{
		// This structure will be used to create the keyboard
		// input event.
		INPUT ip;

		// Set up a generic keyboard event.
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0; // hardware scan code for key
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		// Release the key
		ip.ki.wVk = keycode; // virtual-key code for the "a" key
		ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
		SendInput(1, &ip, sizeof(INPUT));
	}
}

int main(int argc, char** argv)
{
	VideoCapture cap(0); //capture the video from web cam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	namedWindow("Active Color Range", CV_WINDOW_AUTOSIZE); //create a window called "Active Color Range"
	//RGB max and min values:
	int iLowG = 0;
	int iHighG = 50;
	int iLowB = 0;
	int iHighB = 50;
	int iLowR = 100;
	int iHighR = 255;
	//create trackbars in the window so user can tweak those values:
	cvCreateTrackbar("LowG", "Active Color Range", &iLowG, 255);
	cvCreateTrackbar("HighG", "Active Color Range", &iHighG, 255);
	cvCreateTrackbar("LowB", "Active Color Range", &iLowB, 255); 
	cvCreateTrackbar("HighB", "Active Color Range", &iHighB, 255);
	cvCreateTrackbar("LowR", "Active Color Range", &iLowR, 255); 
	cvCreateTrackbar("HighR", "Active Color Range", &iHighR, 255);

	//variables depending on the size of the image, for now just enter the right size manually
	int numofpixwide = 640;
	int numofpixhigh = 480;
	// but since 0 is included in indexing, the pixels will be indexed 0 - xlastpixindex:
	int xlastpixindex = numofpixwide - 1;
	int ylastpixindex = numofpixhigh - 1;
	// assume the first pixel has an index of 0
	int xfirstpixindex = 0;
	int yfirstpixindex = 0;

	//how many lines will be drawn through the middle of the image (how many cuts will make)?
	int numoflinesx = 5;
	int numoflinesy = 5;
	//the number of regions in the image determined by these lines is that plus 1
	int numofblocksx = numoflinesx + 1;
	int numofblocksy = numoflinesy + 1;
	//the number of boundaries determining these regions, including the boundaries of the image
	int numofboundariesx = numofblocksx + 1;
	int numofboundariesy = numofblocksy + 1;
	/* the locations of these gridlines will be stored in a vector.
	but actually, it's more helpful to think about the gridlines in terms of the maximum
	pixel index that is still 'under' that gridline. so for a 600x400 image, xblocksmaxpixiindex
	could look like a vector with 119, 239, 359, 479, 599. However, I don't want the user to 
	tweak that last one, 599, because that's the last pixel, so for now they only get to
	tweak the number of lines in the middle and I'll add that one on after they do so.*/
	std::vector<int> xblocksmaxpixindex; xblocksmaxpixindex.resize(numoflinesx);
	std::vector<int> yblocksmaxpixindex; yblocksmaxpixindex.resize(numoflinesy);
	//create windows for gridline trackbars
	namedWindow("Grid Control x", CV_WINDOW_AUTOSIZE); //create a window called "Grid Control"
	namedWindow("Grid Control y", CV_WINDOW_AUTOSIZE); //create a window called "Grid Control"
	//create trackbars so user can tweak the grid placements for the middle lines.
	for (int ii = 0; ii < numoflinesx; ++ii)
	{
		string label("line x");  label += to_string(ii);
		xblocksmaxpixindex[ii] = (ii + 1) * xlastpixindex / (numoflinesx + 1);
		cvCreateTrackbar(label.c_str(), "Grid Control x", &xblocksmaxpixindex[ii], xlastpixindex);
	}
	for (int jj = 0; jj < numoflinesy; ++jj)
	{
		string label("line y");  label += to_string(jj);
		yblocksmaxpixindex[jj] = (jj + 1) * ylastpixindex / (numoflinesy + 1);
		cvCreateTrackbar(label.c_str(), "Grid Control y", &yblocksmaxpixindex[jj], ylastpixindex);
	}
	//add on the final max pix index - the one at the boundary of the image.
	xblocksmaxpixindex.push_back(xlastpixindex);
	yblocksmaxpixindex.push_back(ylastpixindex);
	//will need to figure out later if these grid placements are okay
	bool xvalslegit, yvalslegit;
	
	//whether a region (I'm calling them blocks) is active depends on the number of pixels active.
	//two ways to calculate, if either conditions met it counts as activated: 
	int thresholdpercentage = 40;
	int thresholdpix = ((numofpixwide * numofpixhigh) / (numoflinesx*numoflinesy)/15);
	namedWindow("Sensitivity", CV_WINDOW_AUTOSIZE); //create a window called "Grid Control"
	cvCreateTrackbar("Threshold %", "Sensitivity", &thresholdpercentage, 100);
	cvCreateTrackbar("Threshold #pix", "Sensitivity", &thresholdpix, (numofpixwide * numofpixhigh) / (numoflinesx*numoflinesy));

	//if a block is active, what do you want the program to do?
	//keymaps: an array of hexadeximal key codes that correspond to the blocks.
	//when a block is activated, that should correspond to a key press.
	vector<vector <int>> keymaps;
	//make this the same size as the grid of blocks
	for (int i = 0; i < numofblocksx; ++i)
	{
		vector <int> tmp;
		keymaps.push_back(tmp);
		for (int j = 0; j < numofblocksy; ++j)
		{
			keymaps[i].push_back(-1); //fill it in with -1s
		}
	}
	//set the values
	keymaps[0][2] = 0x41; //a right
	keymaps[2][0] = 0x57; //w up
	keymaps[3][0] = 0x57; //w up
	keymaps[5][2] = 0x44; //d right
	keymaps[2][5] = 0x53; //s down
	keymaps[3][5] = 0x53; //s down





	//the processing
	while (true)
	{
		//capture the frame
		Mat imgOriginal;
		bool bSuccess = cap.read(imgOriginal); // read a new frame from video
		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}
		//process the frame and store in imgProcd
		Mat imgProcd;
		try {
			//Size is = imgThresholded.size();
			//int  ic = imgThresholded.channels();
			//int  id = imgThresholded.depth();
			
			typedef Vec<uchar, 3> VT;

			CV_Assert(imgOriginal.type() == DataType<VT>::type);
			Size size = imgOriginal.size(); //set the size of imgProcd to the size of the original
			imgProcd.create(size, imgOriginal.type());
			flip(imgOriginal, imgOriginal, 1);  //flip horizontally


			//check if the x lines are valid
			for (int i = 0; i < numoflinesx; ++i) //for each x line
			{
				int xvalinquestion = xblocksmaxpixindex[i]; //this is the x coord we're examining
				if ((0 < xvalinquestion) //it needs to be greater than 0
					&& (xvalinquestion < xblocksmaxpixindex[i + 1]) //and less than the next x coord
					&& (xvalinquestion < xlastpixindex)) //and less than the last (boundary) pixel
				{
					xvalslegit = true; //so far so good
					//cout << "xvals good...";
				}
				else
				{
					xvalslegit = false; //uh oh, this x val has a problem
					//cout << "xvals bad...";
					break;  //break the loop and leave it at false
				}
			}
			//check if yvals good, same as above
			for (int j = 0; j < numoflinesx; ++j)
			{
				int yvalinquestion = yblocksmaxpixindex[j];
				if ((0 < yvalinquestion) && (yvalinquestion < yblocksmaxpixindex[j + 1]) && (yvalinquestion < ylastpixindex))
				{
					yvalslegit = true;
					//cout << "yvals good...";
				}
				else
				{
					yvalslegit = false;
					//cout << "yvals bad...";
					break;
				}
			}
			//cout << endl;


			//counters: an array thing that counts how many pixels are active in each block
			vector<vector <int>> counters;
			//make this the same size as the grid of blocks
			for (int i = 0; i < numofblocksx; ++i)
			{
				vector <int> tmp;
				counters.push_back(tmp);
				for (int j = 0; j < numofblocksy; ++j)
				{
					counters[i].push_back(0); //fill it in with 0s
				}
			}
			
			//go through each pixel and see if it is within the target color range - if so,
			//add one to the counter of the block it lies in.
			for (int y = 0; y<imgOriginal.rows; y++)
			{
				for (int x = 0; x<imgOriginal.cols; x++)
				{
					Vec3b color = imgOriginal.at<Vec3b>(Point(x, y)); //get pixel
					if ((color[0] >= iLowG) && (color[0] <= iHighG) //if the color's in the g range
						&& (color[1] >= iLowB) && (color[1] <= iHighB) //and the b range
						&& (color[2] >= iLowR) && (color[2] <= iHighR)) //and the r range
					{
						color[0] = 0; color[1] = 0; color[2] = 255; //make it totally red
						if (xvalslegit && yvalslegit) //if the grid is okay, add it to the counters
						{
							int xcounterindex, ycounterindex;
							for (int i = 0; i < numoflinesx + 1; ++i) //one by one, for each line
							{
								//check if it's within the block defined by that line, and move on
								if (x <= xblocksmaxpixindex[i]) { xcounterindex = i; break; }
							}
							for (int j = 0; j < numoflinesy + 1; ++j)
							{
								if (y <= yblocksmaxpixindex[j]) { ycounterindex = j; break; }
							}
							++counters[xcounterindex][ycounterindex]; //add 1 to the appropriate counter
						}
					}
					// set pixel color
					imgProcd.at<Vec3b>(Point(x, y)) = color;
				}
			}

			//active block matrix, to tell true or false whether it's 'active':
			vector<vector <bool>> activeblocks;
			//set the size to the number of blocks
			for (int i = 0; i < numofblocksx; ++i)
			{
				vector <bool> tmp;
				activeblocks.push_back(tmp);
				for (int j = 0; j < numofblocksy; ++j)
				{
					activeblocks[i].push_back(false); //by default, all are false (not active)
				}
			}

			//calculate which are active
			if (xvalslegit && yvalslegit) //if the lines' xvals and yvals are ok
			{
				//iterate through all the blocks
				for (int xindex = 0; xindex < numofblocksx; ++xindex)
				{
					for (int yindex = 0; yindex < numofblocksy; ++yindex)
					{
						//find the minimum and maximum pixel indices of this block
						int minx = ((xindex == 0) ? xfirstpixindex : xblocksmaxpixindex[(xindex - 1)] + 1);
						int miny = ((yindex == 0) ? yfirstpixindex : yblocksmaxpixindex[(yindex - 1)] + 1);
						int maxx = (xblocksmaxpixindex[xindex]);
						int maxy = (yblocksmaxpixindex[yindex]);
						//find the area of the block for thresholdpercent purposes
						int areaofblock = ((maxx - minx) * (maxy - miny));
						if (areaofblock == 0) { areaofblock = 1; cout << "area is zero! \n"; } //if zero, that shouldn't happen, set to 1
						int numactivepix = counters[xindex][yindex]; //get the number of active pixels that was counted up
						//is this block active?
						bool isactive =
							(((100 * numactivepix / areaofblock) > thresholdpercentage) //if the percentage of pixels active is greater than the set threshold
							|| (numactivepix > thresholdpix)); //or if the number of pixels active is greater than the set threshold
						int colorindex = (isactive ? 2 : 0); //if it's active, max out the redness, if not max out the blueness
						//do that to the pixels on the border of the block
						for (int yy = miny; yy < maxy; ++yy) //for the horizontal borders
						{
							imgProcd.at<Vec3b>(Point(minx, yy))[colorindex] = 255;
							imgProcd.at<Vec3b>(Point(maxx, yy))[colorindex] = 255;
						}
						for (int xx = minx; xx < maxx; xx++) //for the vertical borders
						{
							imgProcd.at<Vec3b>(Point(xx, miny))[colorindex] = 255;
							imgProcd.at<Vec3b>(Point(xx, maxy))[colorindex] = 255;
						}
						//if it's active, make its representative in the isactive array true
						if (isactive) 
						{
							activeblocks[xindex][yindex] = true; 
							cout << "("; cout << xindex; cout << ", "; cout << yindex; cout << "), "; //debug
							//press the right key
							presskey(keymaps[xindex][yindex]);
						}
						else releasekey(keymaps[xindex][yindex]);
					}
				}
				cout << endl; //debug
			}
		}
		catch (cv::Exception & err) {
			cerr << err.what() << endl;
		}

		imshow("Proccessed Image", imgProcd); //show the combined image

		
		//to exit
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}