#ifndef _FUNCTION_H_
#define _FUNCTION_H_

void Load();
void AllocateSpace();           
void ReadIntoMemory(int k);  

Mat& TJACC(Mat& source, Mat &refIm, Mat &dest, int WinWidth);
Mat& ExtractMaskF   (Mat &inImage,Mat &TmpF);
Mat& ExtractVariaceF(Mat &inImage,Mat &VarF);

void DeleteSpace();                     

#endif