#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgProc/imgProc.hpp>
#include <string>
#define WINVER 0x0500
#include <windows.h>
#include <chrono>

using namespace cv;
using namespace std;

//make a vector of vectors and fill each entry with a certain element
void makevectorvector(int width, int height, int fillwiththis, vector<vector<int>> &destinationvectorvector)
{
	for (int i = 0; i < width; ++i)
	{
		vector <int> tmp;
		destinationvectorvector.push_back(tmp);
		for (int j = 0; j < height; ++j)
		{
			destinationvectorvector[i].push_back(fillwiththis);
		}
	}
}
void makevectorvector(int width, int height, bool fillwiththis, vector<vector<bool>> &destinationvectorvector)
{
	for (int i = 0; i < width; ++i)
	{
		vector <bool> tmp;
		destinationvectorvector.push_back(tmp);
		for (int j = 0; j < height; ++j)
		{
			destinationvectorvector[i].push_back(fillwiththis);
		}
	}
}

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

//check if a color is in the range
bool colorinGBRrange(Vec3b color, vector<int> rangevector)
{
	if ((color[0] >= rangevector[0]) && (color[0] <= rangevector[1]) //if the color's in the g range
		&& (color[1] >= rangevector[2]) && (color[1] <= rangevector[3]) //and the b range
		&& (color[2] >= rangevector[4]) && (color[2] <= rangevector[5])) //and the r range
		return true;
	else return false;
}
Mat getvideoframe(Mat &imgdestination)
{
	VideoCapture cap(0); //capture the video from web cam
	if (!cap.isOpened())  // if not success
	{
		cout << "Cannot open the web cam" << endl;
	}
	bool bSuccess = cap.read(imgdestination); // read a new frame from video
	if (!bSuccess) //if not success
	{
		cout << "Cannot read a frame from video stream" << endl;
	}
	flip(imgdestination, imgdestination, 1); //flip horizontally
	return imgdestination;
}

bool blocksokay(vector<int> blocksmaxpixindex, int maxpossiblepixelindex)
{
	if (blocksmaxpixindex.size() < 1) { return false; }
	else
	{
		for (int i = 0; i < blocksmaxpixindex.size(); ++i) //for each line
		{
			int valinquestion = blocksmaxpixindex[i]; //this is the coord we're examining
			if ((0 < valinquestion) //it needs to be greater than 0
				&& (valinquestion < blocksmaxpixindex[i + 1]) //and less than the next coord
				&& (valinquestion < maxpossiblepixelindex)) //and less than the last (boundary) pixel
			{
				return true; //so far so good
				//cout << "xvals good...";
			}
			else
			{
				return false; //uh oh, this  val has a problem
				//cout << "xvals bad...";
			}
		}
	}
}

using namespace cv;

//calibrate the grid
int calibrateblocks(vector<int> &xblocksmaxpixindexi, vector<int> &yblocksmaxpixindexj, vector<int> &colorrangevector, int xlastpix, int ylastpix)
{

	namedWindow("Active Color Range", CV_WINDOW_AUTOSIZE); //create a window called "Active Color Range"
	//create trackbars in the window so user can tweak those values:
	cvCreateTrackbar("LowG", "Active Color Range", &colorrangevector[0], 255);
	cvCreateTrackbar("HighG", "Active Color Range", &colorrangevector[1], 255);
	cvCreateTrackbar("LowB", "Active Color Range", &colorrangevector[2], 255);
	cvCreateTrackbar("HighB", "Active Color Range", &colorrangevector[3], 255);
	cvCreateTrackbar("LowR", "Active Color Range", &colorrangevector[4], 255);
	cvCreateTrackbar("HighR", "Active Color Range", &colorrangevector[5], 255);

	Mat imgframe;
	chrono::high_resolution_clock::time_point starttime = chrono::high_resolution_clock::now();
	while ((chrono::duration_cast<chrono::duration<double> > (chrono::high_resolution_clock::now() - starttime)).count() < 4.0)
	{
		getvideoframe(imgframe);
		imshow("Calibration Standby...", imgframe); //show the image
	}

	int maxofxmin, minofxmax, maxofymin, minofymax;

	while ((chrono::duration_cast<chrono::duration<double> > (chrono::high_resolution_clock::now() - starttime)).count() < 10.0)
	{
		getvideoframe(imgframe);

		int xmin, xmax, ymin, ymax;
		for (int y = 0; y<imgframe.rows; y++)
		{
			for (int x = 0; x<imgframe.cols; x++)
			{
				Vec3b color = imgframe.at<Vec3b>(Point(x, y)); //get pixel
				if (colorinGBRrange(color, colorrangevector))
				{
					if ((x == 0) && (y == 0))
					{
						xmin = x;
						xmax = x;
						ymin = y;
						ymax = y;

						maxofxmin = x;
						minofxmax = x;
						maxofymin = y;
						minofymax = y;
					}
					else
					{
						xmin = min(xmin, x);
						xmax = max(xmax, x);
						ymin = min(ymin, y);
						ymax = max(ymax, y);

						maxofxmin = max(maxofxmin, xmin);
						minofxmax = min(minofxmax, xmax);
						maxofymin = max(maxofymin, ymin);
						minofymax = min(minofymax, ymax);
					}
				}
			}
		}
		Point pt1 = (xmin, ymin);
		Point pt2 = (xmax, ymax);
		rectangle(imgframe, pt1, pt2, Scalar(0, 255, 0), 2, 8, 0);

		Point otherpt1 = (maxofxmin, maxofymin);
		Point otherpt2 = (minofxmax, minofymax);
		rectangle(imgframe, otherpt1, otherpt2, Scalar(0, 0, 255), 2, 8, 0);

		imshow("Calibrating...", imgframe); //show the image
		
	}
	
	float xcenter = (maxofxmin + minofxmax) / 2;
	float ycenter = (maxofymin + minofymax) / 2;
	float xscale = (minofxmax - maxofxmin);
	float yscale = (minofymax - maxofymin);

	xblocksmaxpixindexi[0] = xcenter - .48193*xscale;
	xblocksmaxpixindexi[1] = xcenter - .21687*xscale;
	xblocksmaxpixindexi[2] = xcenter;
	xblocksmaxpixindexi[3] = xcenter + .21687*xscale;
	xblocksmaxpixindexi[4] = xcenter + .48193*xscale;

	yblocksmaxpixindexj[0] = ycenter - .60667*yscale;
	yblocksmaxpixindexj[1] = ycenter - .39333*yscale;
	yblocksmaxpixindexj[2] = ycenter - .23333*yscale;
	yblocksmaxpixindexj[3] = ycenter - .06267*yscale;
	yblocksmaxpixindexj[4] = ycenter + .08667*yscale;
	yblocksmaxpixindexj[5] = ycenter + .25733*yscale;
	yblocksmaxpixindexj[6] = ycenter + .41733*yscale;

	if (blocksokay(xblocksmaxpixindexi, xlastpix) && blocksokay(yblocksmaxpixindexj, ylastpix))
	{
		return 0;
	}
	else return -1;
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
	//create vector to store these nicely
	vector<int> GBRrange = { iLowG, iHighG, iLowB, iHighB, iLowR, iHighR };

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
	int numoflinesy = 7;
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
	/*//create windows for gridline trackbars
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
	*/
	//add on the final max pix index - the one at the boundary of the image.
	xblocksmaxpixindex.push_back(int(xlastpixindex));
	yblocksmaxpixindex.push_back(int(ylastpixindex));
	
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
	makevectorvector(numofblocksx, numofblocksy, -1, keymaps);
	/*//set the values
	keymaps[0][2] = 0x41; //a right
	keymaps[2][0] = 0x57; //w up
	keymaps[3][0] = 0x57; //w up
	keymaps[5][2] = 0x44; //d right
	keymaps[2][5] = 0x53; //s down
	keymaps[3][5] = 0x53; //s down

	*/
	
	//Calibrate x and y gridlines automatically
	int n = -1;
	while (n == -1)
	{
		n = calibrateblocks(xblocksmaxpixindex, yblocksmaxpixindex, GBRrange, xlastpixindex, ylastpixindex);
	}

	//the processing
	while (true)
	{
		//capture the frame
		Mat imgOriginal;
		getvideoframe(imgOriginal);

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


			//counters: an array thing that counts how many pixels are active in each block
			vector<vector <int>> counters;
			//make this the same size as the grid of blocks
			makevectorvector(numofblocksx, numofblocksy, 0, counters);
			
			//go through each pixel and see if it is within the target color range - if so,
			//add one to the counter of the block it lies in.
			for (int y = 0; y<imgOriginal.rows; y++)
			{
				for (int x = 0; x<imgOriginal.cols; x++)
				{
					Vec3b color = imgOriginal.at<Vec3b>(Point(x, y)); //get pixel
					if (colorinGBRrange(color, GBRrange))
					{
						color[0] = 0; color[1] = 0; color[2] = 255; //make it totally red
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
			makevectorvector(numofblocksx, numofblocksy, false, activeblocks);

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