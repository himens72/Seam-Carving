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
void removeHorizontalPixel(Mat& image, Mat& output, int *seam) {
	for (int col = 0; col < image.cols; col++) {
		for (int row = 0; row < image.rows - 1; row++) {
			if (row >= seam[col]) {
				output.at<Vec3b>(row, col) = image.at<Vec3b>(row + 1, col);
			}
			else {
				output.at<Vec3b>(row, col) = image.at<Vec3b>(row, col);
			}
		}
	}
}
int *currentHorizontalSeam(Mat &image) {
	for (int i = 0; i < image.rows; i++) {
		*(verticalMatrix + i * image.rows + 0) = (int)image.at<uchar>(i, 0);
	}
	for (int j = 1; j < image.cols; j++) {
		for (int i = 0; i < image.rows; i++) {
			if (i == 0) {
				int x = *(verticalMatrix + (i)* image.rows + (j - 1));
				int y = *(verticalMatrix + (i + 1) * image.rows + (j - 1));
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(x, y);
			}
			else if (i == image.rows - 1) {
				*(verticalMatrix + i * image.rows + j) = (int)image.at<uchar>(i, j) + min(*(verticalMatrix + (i - 1) * image.rows + (j - 1)), *(verticalMatrix + (i)* image.rows + (j - 1)));
			}
			else {
				int x = *(verticalMatrix + (i)* image.rows + (j - 1));
				int y = *(verticalMatrix + (i + 1) * image.rows + (j - 1));
				int z = *(verticalMatrix + (i - 1)* image.rows + (j - 1));
				int minimum = min({ x,y,z });
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
	int column = image.cols - 1;
	int row = index;
	while (column != 0) {
		value = *(verticalMatrix + row * image.rows + column) - (int)image.at<uchar>(row, column);
		if (row == 0) {
			if (value == *(verticalMatrix + (row + 1) * image.rows + (column - 1))) {
				row += 1;
			}
		}
		else  if (row == image.cols - 1) {
			if (value == *(verticalMatrix + (row - 1) * image.rows + (column - 1))) {
				row -= 1;
			}
		}
		else {
			if (value == *(verticalMatrix + (row - 1) * image.rows + (column - 1))) {
				row -= 1;
			}
			else if (value == *(verticalMatrix + (row + 1) * image.rows + (column - 1))) {
				row += 1;
			}
		}
		column -= 1;
		seamPath[column] = row;
	}
	return seamPath;
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
	//int valueIndex = image.rows - 1;
	//int currentIndex = index;
	int row = image.rows - 1;
	int column = index;
	while (row != 0) {
		value = *(verticalMatrix + row * image.rows + column) - (int)image.at<uchar>(row, column);
		if (column == 0) {
			if (value == *(verticalMatrix + (row - 1) * image.rows + (column + 1))) {
				column += 1;
			}
		}
		else  if (column == image.cols - 1) {
			if (value == *(verticalMatrix + (row - 1)  * image.rows + (column - 1))) {
				column -= 1;
			}

		}
		else {
			if (value == *(verticalMatrix + (row - 1) * image.rows + (column - 1))) {
				column -= 1;
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
	verticalMatrix = (int *)malloc(image.rows * image.cols * sizeof(int));
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
	width = in_image.cols;
	cout << in_image.rows << " " << in_image.cols << endl;
	for (int i = 0; i < height - new_height; i++) {
		Mat output(in_image.rows - 1, in_image.cols, CV_8UC3);
		reduce_horizontal_seam_trivial(in_image, output);
		in_image = output;
	}
	out_image = in_image;
	imwrite("tempOutput.jpg"/*argv[4]*/, in_image);
}
void calculateEnergy(Mat &gray_image, Mat &output) {
	Mat sobel_x;
	Mat sobel_y;
	GaussianBlur(gray_image, gray_image, Size(3, 3), 0, 0, BORDER_DEFAULT);
	cvtColor(gray_image, gray_image, COLOR_BGR2GRAY);
	Mat absolute_x, absolute_y;
	Mat gradient;
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
	int *seam = currentSeam(tempOutput);
	removePixel(in_image, out_image, seam);
}