#include "pch.h"
#include "sc.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include<fstream>
#include<sstream>
using namespace cv;
using namespace std;
int  *verticalMatrix;
//int  *horizontalMatrix;
ofstream OUTPUT_FILE("matrix.txt");
void removePixel(Mat& image, Mat& output, int *seam) {
	//OUTPUT_FILE << "Start Removing Pixel : " << image.rows << " " << image.cols << endl;
	/*for (int r = 0; r < image.rows; r++) {
		OUTPUT_FILE << " SEAM " << r << " : " << seam[r]<<endl;
	}*/
	for (int r = 0; r < image.rows; r++) {
		for (int c = 0; c < image.cols - 1; c++) {
			if (c >= seam[r]) {
				//		OUTPUT_FILE << r << " " << c << " : " << seam[r] << endl;
				output.at<Vec3b>(r, c) = image.at<Vec3b>(r, c + 1);
			}
			else {
				//			OUTPUT_FILE << r << " " << c << " : " << seam[r] << endl;
				output.at<Vec3b>(r, c) = image.at<Vec3b>(r, c);
			}
		}
	}
	//OUTPUT_FILE << "END Removing Pixel" << endl;
	//imwrite("tempOutput.jpg"/*argv[4]*/, output);

}
void removeHorizontalPixel(Mat& image, Mat& output, int *seam) {
	for (int c = 0; c < image.cols; c++) {
		for (int r = 0; r < image.rows - 1; r++) {
			if (r >= seam[c]) {
				output.at<Vec3b>(r, c) = image.at<Vec3b>(r + 1,c);
			}
			else {
				output.at<Vec3b>(r, c) = image.at<Vec3b>(r, c);
			}
		}
	}
}
int *currentHorizontalSeam(Mat &image) {
	for (int i = 0; i < image.rows; i++) {
		//OUTPUT_FILE<<image.rows<<" --->" << i << " " << (int)image.at<uchar>(i, 0) << endl;
		*(verticalMatrix + i * image.rows + 0) = (int)image.at<uchar>(i, 0);
	}
	for (int j = 1; j < image.cols; j++) {
		for (int i = 0; i < image.rows; i++) {
			if (i == 0) {
				int x = *(verticalMatrix + (i) * image.rows + (j - 1));
				int y = *(verticalMatrix + (i + 1) * image.rows + (j - 1));
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(x, y);
			}
			else if (i == image.rows - 1) {
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(*(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i) * image.rows + (j - 1)));
			}
			else {
				int x = *(verticalMatrix + (i)* image.rows + (j - 1));
				int y = *(verticalMatrix + (i + 1) * image.rows + (j - 1));
				int z = *(verticalMatrix + (i - 1)* image.rows + (j - 1));
				//OUTPUT_FILE<<i <<","<<j<<" --------- >" << x << " : "<< y << " : " << z << endl;
				int minimum = min({x,y,z});//min({ *(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i) * image.rows + (j - 1)),*(verticalMatrix + (i + 1) * image.rows + (j - 1)) });
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + minimum;
			}
		}
	}
	int value = 2147483647;
	int index = -1;
	for (int i = 0; i <= image.rows; i++) {
		if (*(verticalMatrix + i * image.rows + (image.cols - 1)) < value) {
			value = *(verticalMatrix + i * image.rows + (image.cols - 1));
			index = i;
		}
	}
	int *seamPath = new int[image.cols];
	seamPath[image.cols - 1] = index;

	int valueIndex = image.rows - 1;
	int currentIndex = index;
	//OUTPUT_FILE << valueIndex << " " << currentIndex << endl;
	while (valueIndex != 0) {
		value = *(verticalMatrix + valueIndex * image.rows + currentIndex) - (int)image.at<uchar>(valueIndex, currentIndex);
		if (currentIndex == 0) {
			if (value == *(verticalMatrix + (currentIndex + 1) * image.rows + (valueIndex - 1) )) {
				currentIndex += 1;
			}
		}
		else  if (currentIndex == image.cols - 1) {
			if (value == *(verticalMatrix + (currentIndex - 1) * image.rows + (valueIndex - 1))) {
				currentIndex -= 1;
			}
			else if (value == *(verticalMatrix + (currentIndex + 1) * image.rows + (valueIndex - 1))) {
				currentIndex += 1;
			}
		}
		else {
			if (value == *(verticalMatrix + (currentIndex - 1) * image.rows + (valueIndex - 1))) {
				currentIndex -= 1;
			}
		}
		valueIndex -= 1;
		//OUTPUT_FILE << valueIndex << " " << currentIndex << endl;
		seamPath[valueIndex] = currentIndex;
	}
	//OUTPUT_FILE << "END " << endl;
	return seamPath;
}

int *currentSeam(Mat &image) {
	//ofstream OUTPUT_FILE11("matrix111.txt");
	for (int i = 0; i < image.cols; i++) {
		*(verticalMatrix + 0 * image.rows + i) = (int)image.at<uchar>(0, i);
		//OUTPUT_FILE << *(matrix + 0 * image.rows + i) << " " ;
	}
	//OUTPUT_FILE << endl;
	//OUTPUT_FILE11 << "Image Width : " << image.cols << endl;
	//OUTPUT_FILE11 << "Image Height : " << image.rows << endl;
	for (int i = 1; i < image.rows; i++) {
		//OUTPUT_FILE << i << " :  ";
		for (int j = 0; j < image.cols; j++) {
			if (j == 0) {
				int x = *(verticalMatrix + (i - 1) * image.rows + j);
				int y = *(verticalMatrix + (i - 1) * image.rows + (j + 1));
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(x, y);
				//	OUTPUT_FILE11 << *(matrix + i * image.rows + j) << " ";
			}
			else if (j == image.cols - 1) {
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(*(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i - 1) * image.rows + j));
				//	OUTPUT_FILE11 << *(matrix + i * image.rows + j) << " ";
			}
			else {
		//		OUTPUT_FILE11 << "--->" << i << " " << j << endl;
				int minimum = min({ *(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i - 1) * image.rows + j),*(verticalMatrix + (i - 1) * image.rows + (j + 1)) });
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + minimum;
				//	OUTPUT_FILE << *(matrix + i * image.rows + j) << " ";
			}
		}
		//OUTPUT_FILE << endl;
	}
	int value = 2147483647;
	int index = -1;
	for (int i = 0; i < image.cols; i++) {
		if (*(verticalMatrix + (image.rows - 1) * image.rows + i) <= value) {
			value = *(verticalMatrix + (image.rows - 1) * image.rows + i);
			index = i;
		}
	}
	int *seamPath = new int[image.rows];
	seamPath[image.rows - 1] = index;

	int valueIndex = image.rows - 1;
	int currentIndex = index;
	//OUTPUT_FILE << valueIndex << " " << currentIndex << endl;
	while (valueIndex != 0) {
		value = *(verticalMatrix + valueIndex * image.rows + currentIndex) - (int)image.at<uchar>(valueIndex, currentIndex);
		if (currentIndex == 0) {
			if (value == *(verticalMatrix + (valueIndex - 1) * image.rows + (currentIndex + 1))) {
				currentIndex += 1;
			}
		}
		else  if (currentIndex == image.cols - 1) {
			if (value == *(verticalMatrix + (valueIndex - 1)  * image.rows + (currentIndex - 1))) {
				currentIndex -= 1;
			}
			else if (value == *(verticalMatrix + (valueIndex - 1) * image.rows + (currentIndex + 1))) {
				currentIndex += 1;
			}
		}
		else {
			if (value == *(verticalMatrix + (valueIndex - 1) * image.rows + (currentIndex - 1))) {
				currentIndex -= 1;
			}
		}
		valueIndex -= 1;
		//OUTPUT_FILE << valueIndex << " " << currentIndex << endl;
		seamPath[valueIndex] = currentIndex;
	}
	//OUTPUT_FILE << "END " << endl;
	return seamPath;
}
void generateVerticalMatrix(Mat &image) {
	verticalMatrix = (int *)malloc(image.rows * image.cols * sizeof(int));
}
void generateHorizontalMatrix(Mat &image) {
	//horizontalMatrix = (int *)malloc(image.rows * image.cols * sizeof(int));
}

void performCarving(Mat &in_image, int new_width, int new_height, Mat &out_image) {
	//imwrite("output1.jpg"/*argv[4]*/, out_image);
	int width = in_image.cols;
	int height = in_image.rows;
	generateVerticalMatrix(in_image);
	for (int i = 0; i < width - new_width; i++) {
		cout << "----> " << in_image.cols << endl;
		Mat output(in_image.rows, in_image.cols - 1, CV_8UC3);
		reduce_vertical_seam_trivial(in_image, output);
		in_image = output;
	}
	//free(verticalMatrix);
	//generateHorizontalMatrix(in_image);
	width = in_image.cols;
	cout << in_image.rows << " " << in_image.cols << endl;
	for (int i = 0; i < height - new_height; i++) {
		cout << "1111----> " << in_image.rows << endl;
		Mat output(in_image.rows - 1, in_image.cols, CV_8UC3);
		reduce_horizontal_seam_trivial(in_image, output);
		in_image = output;
	}
	imwrite("tempOutput.jpg"/*argv[4]*/, in_image);
}
void normalizeImage(Mat &output) {
	double maximumElement;
	double minimumElement;
	minMaxLoc(output, &minimumElement, &maximumElement);
	output = output * (1 / maximumElement * 255);
	output.convertTo(output, CV_8U);
}
void calculateEnergy(Mat &gray_image, Mat &output) {
	Mat sobel_x;
	Mat sobel_y;
	cvtColor(gray_image, gray_image, COLOR_BGR2GRAY);/*
	Sobel(gray_image, sobel_x, CV_64F, 1, 0);
	Sobel(gray_image, sobel_y, CV_64F, 0, 1);
	magnitude(sobel_x, sobel_y, output);
	normalizeImage(output);*/
	Mat absolute_x, absolute_y;
	Sobel(gray_image, sobel_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(gray_image, sobel_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(sobel_x, absolute_x);
	convertScaleAbs(sobel_y, absolute_y);
	addWeighted(absolute_x, 0.5, absolute_y, 0.5, 0, output);

}


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image) {

	// some sanity checks
	// Check 1 -> new_width <= in_image.cols
	if (new_width > in_image.cols) {
		return false;
	}
	if (new_height > in_image.rows) {
		return false;
	}

	if (new_width <= 0) {
		return false;

	}

	if (new_height <= 0) {
		return false;

	}
	performCarving(in_image, new_width, new_height, out_image);
	return seam_carving_trivial(in_image, new_width, new_height, out_image);
}


// seam carves by removing trivial seams
bool seam_carving_trivial(Mat& in_image, int new_width, int new_height, Mat& out_image) {
	return true;
}

// horizontl trivial seam is a seam through the center of the image
void reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image) {
	int width = in_image.cols;
	int height = in_image.rows;
	Mat tempOutput;
	Mat GRAY_IMAGE = in_image.clone();
	calculateEnergy(GRAY_IMAGE, tempOutput);
	//imwrite("tempOutput.jpg"/*argv[4]*/, tempOutput);
	int *seam = currentHorizontalSeam(tempOutput);
	removeHorizontalPixel(in_image, out_image, seam);
}

// vertical trivial seam is a seam through the center of the image
void reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image) {
	int width = in_image.cols;
	int height = in_image.rows;
	Mat tempOutput;
	Mat GRAY_IMAGE = in_image.clone();
	calculateEnergy(GRAY_IMAGE, tempOutput);
	//imwrite("tempOutput.jpg"/*argv[4]*/, tempOutput);
	int *seam = currentSeam(tempOutput);
	removePixel(in_image, out_image, seam);
}