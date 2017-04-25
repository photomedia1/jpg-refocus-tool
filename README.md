# jpg-refocus-tool
====
Refocus JPG file taken by Huawei P9 and Mate9. This is unofficial tool.

## Description
Importing JPG files, change focus by clicking, and save JPG files.
Developed with C++

## Requirement
This is for Windows.

## How to Use
1. Click button “open Images” to open jpeg files.
2. Change focus-distance by clicking slider or clicking position over image.
3. Click button “Convert focus and Save” to save re-focused images.

## Source Structure
1. CImageB dll project
Jpeg, bmp, png, tiff image reader derived from CxImage which is familiar open source. 

2. TuneFocus project
Main project.

TunefocusDlg.cpp: implementation for main dialog(Interface)  	
TuneFocusDlg.h: Interface header	
PicViewDlg.cpp: Implementation for showing image	
PicViewDlg.h: Interface header	
CxImgA.h: Export functions’ header of CImageB.dll	
ImgFocus.cpp: Implementation of analyzing jpeg and saving
ImgFocus.h: Header of analyzing jpeg file	


## About ImgFocus.cpp

1) Analysis

OpenPhoneImageFile: Highest level of analysis process
OpenHuaweiPJpgFile: Export function
AnaBinData: Analyze file format of Huawei P9 Jpeg.
FindCorpusJpeg: Find image1, Image2 and metadata in file. 
GetMetaDataOfHuaWeiJpeg: Make depth image data from meta data.


2) Refocusing

GetFocusedImage: Highest level of re-focussing.
GetFocusedImageImpl: Make Blurring image by focus value.
CreateIntImage: Create integral image to speed up Gaussian filtration



-GetFocusedImageImpl
This function implements blurring image by focal value. It applies Gaussian filtration to simulate blurring image.
It makes variant type of Gaussian to speed up image processing.
This speed up is provided from CreateIntImage function, which is similar to opencv’s integral image operator.
