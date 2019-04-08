/*
 * https://docs.opencv.org/2.4.13.7/doc/tutorials/imgproc/imgtrans/laplace_operator/laplace_operator.html
 * https://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/sobel_derivatives/sobel_derivatives.html#explanation
 * https://en.wikipedia.org/wiki/Seam_carving
 * http://eric-yuan.me/seam-carving/
*/
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
void removePixel(Mat& image, Mat& output, int *seam) {
	for (int row = 0; row < image.rows; row++) {
		for (int col = 0; col < image.cols - 1; col++) {
			if (col >= seam[row]) {
				output.at<Vec3b>(row, col) = image.at<Vec3b>(row, col + 1);
			}
			else {
				output.at<Vec3b>(row, col) = image.at<Vec3b>(row, col);
			}
		}
	}
}
int *currentSeam(Mat &image) {
	for (int i = 0; i < image.cols; i++) {
		*(verticalMatrix + 0 * image.rows + i) = (int)image.at<uchar>(0, i);
	}
	for (int i = 1; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if (j == 0) {
				int x = *(verticalMatrix + (i - 1) * image.rows + j);
				int y = *(verticalMatrix + (i - 1) * image.rows + (j + 1));
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(x, y);
			}
			else if (j == image.cols - 1) {
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(*(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i - 1) * image.rows + j));
			}
			else {
				int minimum = min({ *(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i - 1) * image.rows + j),*(verticalMatrix + (i - 1) * image.rows + (j + 1)) });
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + minimum;
			}
		}
	}
	int value = 2147483647;
	int index = -1;
	for (int i = 0; i < image.cols; i++) {
		if (*(verticalMatrix + (image.rows - 1) * image.rows + i) < value) {
			value = *(verticalMatrix + (image.rows - 1) * image.rows + i);
			index = i;
		}
	}
	int *seamPath = new int[image.rows];
	seamPath[image.rows - 1] = index;
	int row = image.rows - 1;
	int column = index;
	while (row != 0) {
		value = *(verticalMatrix + row * image.rows + column) - (int)image.at<uchar>(row, column);
		if (column == 0) {
			if (value == *(verticalMatrix + (row - 1) * image.rows + (column))) {

			}
			else if (value == *(verticalMatrix + (row - 1) * image.rows + (column + 1))) {
				column += 1;
			}
		}
		else  if (column == image.cols - 1) {
			if (value == *(verticalMatrix + (row - 1)  * image.rows + (column - 1))) {
				column -= 1;
			}
			else if (value == *(verticalMatrix + (row - 1) * image.rows + (column))) {

			}
		}
		else {
			if (value == *(verticalMatrix + (row - 1) * image.rows + (column - 1))) {
				column -= 1;
			}
			else if (value == *(verticalMatrix + (row - 1) * image.rows + (column))) {

			}
			else if (value == *(verticalMatrix + (row - 1) * image.rows + (column + 1))) {
				column += 1;
			}
		}
		row -= 1;
		seamPath[row] = column;
	}
	return seamPath;
}
void generateVerticalMatrix(Mat &image) {
	int x = max(image.rows, image.cols);
	verticalMatrix = (int *)malloc(x * x * sizeof(int));
}
void performCarving(Mat &in_image, int new_width, int new_height, Mat &out_image) {
	int width = in_image.cols;
	int height = in_image.rows;
	generateVerticalMatrix(in_image);
	for (int i = 0; i < width - new_width; i++) {
		Mat output(in_image.rows, in_image.cols - 1, CV_8UC3);
		reduce_vertical_seam_trivial(in_image, output);
		in_image = output;
	}
	transpose(in_image, in_image);
	flip(in_image, in_image, +1);
	width = in_image.cols;
	height = in_image.rows;
	for (int i = 0; i < width - new_height; i++) {
		Mat output(in_image.rows, in_image.cols - 1, CV_8UC3);
		reduce_vertical_seam_trivial(in_image, output);
		in_image = output;
	}
	transpose(in_image, in_image);
	flip(in_image, in_image, +1);
	transpose(in_image, in_image);
	flip(in_image, in_image, +1);
	transpose(in_image, in_image);
	flip(in_image, in_image, +1);
	out_image = in_image;
	cout << "Seam Carving Done";
}
void calculateEnergy(Mat &image, Mat &output) {
	Mat gray_image;
	int size = 3;
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	GaussianBlur(image, gray_image, Size(3, 3), 0, 0, BORDER_DEFAULT);
	cvtColor(image, gray_image, COLOR_RGB2GRAY);
	Laplacian(gray_image, output, ddepth, size, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(output, output);
}


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image) {

	// some sanity checks
	// Check 1 -> new_width <= in_image.cols
	if (new_width > in_image.cols) {
		cout << "Invalid request!!! new_width has to be smaller than the current size!" << endl;
		return false;
	}
	if (new_height > in_image.rows) {
		cout << "Invalid request!!! ne_height has to be smaller than the current size!" << endl;
		return false;
	}

	if (new_width <= 0) {
		cout << "Invalid request!!! new_width has to be positive!" << endl;
		return false;

	}

	if (new_height <= 0) {
		cout << "Invalid request!!! new_height has to be positive!" << endl;
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

bool reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image) {
	return true;
}

// vertical trivial seam is a seam through the center of the image
void reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image) {
	int width = in_image.cols;
	int height = in_image.rows;
	Mat tempOutput;
	Mat GRAY_IMAGE = in_image.clone();
	calculateEnergy(GRAY_IMAGE, tempOutput);
	int *seam = currentSeam(tempOutput);
	removePixel(in_image, out_image, seam);
}
