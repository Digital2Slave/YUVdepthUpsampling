#include "head.h"
#include "function.h"

FILE *file_depth_input = nullptr;
FILE *file_color_input = nullptr;
FILE *file_up_output = nullptr;

unsigned char *src_depth_y = nullptr;
unsigned char *src_color_y = nullptr;
unsigned char *p_y = nullptr;
unsigned char *p_u = nullptr;
unsigned char *p_v = nullptr;

Mat F;                          
Mat VarCF, VarDF, VarF;

int main(int argc, char *argv[])
{

	//Load();
	
	char *color_input,*depth_input,*up_output;
	color_input = argv[1];
	depth_input = argv[2];
	up_output   = argv[3];

	file_color_input = fopen(color_input,"rb");
	file_depth_input = fopen(depth_input,"rb");
	file_up_output   = fopen(up_output,"wb");

	if (argc!=4)
	{
		cout<<"Files input can't read in memory! "<<endl;
		exit(0);
	}

	AllocateSpace();

	double longBPR = 0.0, longMSE = 0.0, longRMSE = 0.0, longPSNR = 0.0;
	cout<<"2014 Tian TJACC imagescale "<<dusize<<" :"<<endl;

	for (int k=0;k!=PICNUM;k++)
	{
		ReadIntoMemory(k); 

		//INTER_NEAREST Downsampling
		Mat CYimg = Mat::zeros(height,width,CV_8UC1);
		Mat DYimg = Mat::zeros(height,width,CV_8UC1);
		Mat CYdimg = Mat::zeros(s_height,s_width,CV_8UC1);
		Mat DYdimg = Mat::zeros(s_height,s_width,CV_8UC1);
		for (int j = 0; j < height; j++)
		{
			for ( int i = 0; i < width; i++)
			{
				CYimg.at<uchar>(j,i) = src_color_y[j*width+i];
				DYimg.at<uchar>(j,i) = src_depth_y[j*width+i];
			}
		}

		resize(CYimg,CYdimg,CYdimg.size(),0,0,INTER_NEAREST);//
		resize(DYimg,DYdimg,DYdimg.size(),0,0,INTER_NEAREST);//

		VarCF = Mat::zeros(s_height, s_width, CV_32FC1);
		VarDF = Mat::zeros(s_height, s_width, CV_32FC1);
		VarF  = Mat::zeros(s_height, s_width, CV_32FC1);
	
		ExtractVariaceF(CYdimg,VarCF);
		ExtractVariaceF(DYdimg,VarDF);

#pragma region CA

		const int w = 3;
		const int bordval = 2;
		const int val = 1;
		int top,bottom,left,right;
		top = bottom = left = right = val;

		Mat TmpCDownBorder = Mat::zeros(s_height+bordval, s_width+bordval, CYdimg.type());
		Mat TmpDDownBorder = Mat::zeros(s_height+bordval, s_width+bordval, DYdimg.type());
		copyMakeBorder(CYdimg, TmpCDownBorder, top, bottom, left, right, BORDER_REPLICATE);
		copyMakeBorder(DYdimg, TmpDDownBorder, top, bottom, left, right, BORDER_REPLICATE);

		for (int j = 1; j < s_height+1; j++)
		{
			for (int i = 1; i < s_width+1; i++)
			{
				double a1 = 0.0, a2 = 0.0;
				double sumPix1 = 0.0,sumPix2 = 0.0, sumPix = 0.0;
				double meanPix1 = 0.0, meanPix2 = 0.0, meanPix= 0.0;
				for (int m = -w/2; m<=w/2; m++)
				{
					int y = j + m;
					y = (y > 0 ? (y < s_height + 2 ? y : s_height + 1 ) : 0);  

					for (int n = -w/2; n<=w/2; n++)
					{
						int x = i + n;
						x = (x > 0 ? (x < s_width + 2 ? x : s_width + 1 ) : 0); 
						a1 = TmpCDownBorder.at<uchar>(y,x);
						a2 = TmpDDownBorder.at<uchar>(y,x);
						sumPix1 += a1;
						sumPix2 += a2;
						sumPix += (a1 * a2);
					}//end for n
				}//end for m

				meanPix1 = sumPix1/(w*w);//EX
				meanPix2 = sumPix2/(w*w);//EY
				meanPix = sumPix/(w*w);  //EXY
				double CA = meanPix - meanPix1 * meanPix2;//EXY - EXEY
				double dx = VarCF.at<float>(j - 1,i - 1);
				double dy = VarDF.at<float>(j - 1,i - 1);   
				CA /= sqrt(dx * dy);                      //CA = ((EXY - EXEY)/sqrt(DXDY))
				VarF.at<float>(j-1, i-1) = CA;
			}//end for i
		}//end for j
#pragma endregion

		Mat Edge = Mat::zeros(DYdimg.size(),DYdimg.type());
		ExtractMaskF(DYdimg,Edge);
		threshold(Edge, F, 0, 255, THRESH_OTSU);  

		Mat SrcUp = Mat::zeros(DYimg.size(),DYimg.type());
		int WinSize   = dusize * dusize + 1;                        //window's size for different scals downsampling

		TJACC(DYdimg,CYimg,SrcUp,WinSize); 

		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				p_y[j*width+i] = SrcUp.at<uchar>(j,i);
				p_u[j/2*width/2 + i/2] = (unsigned char)128;
				p_v[j/2*width/2 + i/2] = (unsigned char)128;
			}
		}
		//
		fseek(file_up_output,width*height*sizeof(unsigned char)*k*3/2,SEEK_SET);
		fwrite(p_y,width*height,sizeof(unsigned char),file_up_output);
		fwrite(p_u,width*height/4,sizeof(unsigned char),file_up_output);
		fwrite(p_v,width*height/4,sizeof(unsigned char),file_up_output);

#pragma region Criterion BPR MSE RMSE PSNR

		Mat BadImage = Mat::zeros(SrcUp.size(),SrcUp.type());
		double BRP = 0.0,Mse = 0.0 , Rmse = 0.0, Psnr = 0.0;
		double blackcnt = 0.0,cnt = 0.0;
		int dv = 0;
		long sum = 0;

		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				if (DYimg.at<uchar>(j,i) != 0)
				{
					//MSE RMSE PSNR
					dv = abs(DYimg.at<uchar>(j,i) - SrcUp.at<uchar>(j,i));
					dv = pow(dv,2);
					sum += dv;
					//BPR
					if (dv > 1)
					{
						BadImage.at<uchar>(j,i) = 0;
						cnt++;
					}
					else
					{
						BadImage.at<uchar>(j,i) = 255;
					}
				} 
				else
				{
					BadImage.at<uchar>(j,i) = 255;
					blackcnt++;
				}
			}
		}

		BRP = 1.0 * cnt / (height*width - blackcnt)*100;
		Mse = 1.0 * sum /(height*width - blackcnt);
		Rmse = sqrt(Mse);
		Psnr = 10 * log10(255*255/(Mse));

		longBPR  += BRP;
		longMSE  += Mse;
		longRMSE += Rmse;
		longPSNR += Psnr;
#pragma endregion
	}

	longBPR  = 1.0*longBPR/PICNUM;
	longMSE  = 1.0*longMSE/PICNUM;
	longRMSE = 1.0*longRMSE/PICNUM;
	longPSNR = 1.0*longPSNR/PICNUM;

	cout<<"Average "<<PICNUM<<" frame: "<<endl;
	cout<<"BPR: "<<longBPR<<"%"<<"\t"<<"MSE: "<<longMSE<<"\t"<<"RMSE: "<<longRMSE<<"\t"<<"PSNR: "<<longPSNR<<endl;
	cout<<"The TJACC upsampling of video is done!"<<endl<<endl;

	DeleteSpace();

	fclose(file_color_input);
	fclose(file_depth_input);
	fclose(file_up_output);

	return 0;
}

Mat& ExtractMaskF(Mat& inImage,Mat &TmpF)
{
	int height = inImage.rows;
	int width = inImage.cols;

	//grad_x and grad_y
	Mat grad;
	Mat grad_x, grad_y;  
	Mat abs_grad_x, abs_grad_y;  
	int scale0 = 1;//  
	int delta0 = 0;//  
	int ddepth = CV_16S;

	// Gradient X 
	Scharr( inImage, grad_x, ddepth, 1, 0, scale0, delta0, BORDER_DEFAULT );  
	convertScaleAbs( grad_x, abs_grad_x );  

	// Gradient Y 
	Scharr( inImage, grad_y, ddepth, 0, 1, scale0, delta0, BORDER_DEFAULT );  
	convertScaleAbs( grad_y, abs_grad_y );  
	
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

	for (int j = 0; j < height;j++)
	{
		for (int i = 0; i < width;i++)
		{
			TmpF.at<uchar>(j, i) = grad.at<uchar>(j,i);
		}
	}
	return TmpF;
}

Mat& ExtractVariaceF(Mat &inImage,Mat &VarF)
{
	//--- 3*3 Varance image---
	const int w = 3;
	const int bordval = 2;
	const int val = 1;

	Mat TmpDownBorder = Mat::zeros(s_height+bordval, s_width+bordval, inImage.type());

	int top,bottom,left,right;
	top = bottom = left = right = val;

	copyMakeBorder(inImage, TmpDownBorder, top, bottom, left, right, BORDER_REPLICATE);

	for (int j = 1; j < s_height+1; j++)
	{
		for (int i = 1; i < s_width+1; i++)
		{
			double sumPix = 0.0,sumPix2 = 0.0;
			double meanPix = 0.0, meanPix2 = 0.0, variancePix = 0.0;
			for (int m = -w/2; m<=w/2; m++)
			{
				int y = j + m;
				y = (y > 0 ? (y < s_height + 2 ? y : s_height + 1 ) : 0);  

				for (int n = -w/2; n<=w/2; n++)
				{
					int x = i + n;
					x = (x > 0 ? (x < s_width + 2 ? x : s_width + 1 ) : 0); 
					uchar a = TmpDownBorder.at<uchar>(y,x);
					sumPix += a;
					sumPix2 += (a*a);
				}//end for n
			}//end for m

			meanPix = sumPix/(w*w);
			meanPix2 = sumPix2/(w*w);
			variancePix = meanPix2 - meanPix*meanPix;  
			VarF.at<float>(j-1, i-1) = variancePix;
		}//end for i
	}//end for j
	return VarF;
}