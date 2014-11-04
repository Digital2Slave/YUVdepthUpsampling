#include "head.h"
#include "function.h"

extern FILE *file_depth_input,*file_color_input;
extern FILE *file_up_output;

extern unsigned char  *src_depth_y, *src_color_y; 
extern unsigned char  *p_y, *p_u,*p_v;

void Load()
{
	//color 
	errno_t err_color;
	if ((err_color = fopen_s(&file_color_input,"F:\\HMseq\\color\\bookarrival\\bookarrival_7.yuv","rb"))!=NULL) 
	{
		cout<<"fail to open the input file !"<<endl;
		exit(-1);
	}

	//depth 
	errno_t err_depth;
	if ((err_depth = fopen_s(&file_depth_input,"F:\\HMseq\\depth\\bookarrival\\depth_book_7.yuv","rb"))!=NULL) 
	{
		cout<<"fail to open the input file !"<<endl;
		exit(-1);
	}

	//upsampling
	errno_t file_up;
	if ((file_up = fopen_s(&file_up_output,"F:\\HMres\\Tian_up_book_7.yuv","wb"))!=NULL) 
	{
		cout<<"fail to open the output file !"<<endl;
		exit(-1);
	}
	//cout<<"Load is done!"<<endl;
}

void AllocateSpace()
{
	src_color_y  = new unsigned char[height*width*sizeof(unsigned char)];//
	src_depth_y  = new unsigned char[height*width*sizeof(unsigned char)];//

	p_y = new unsigned char[height*width*sizeof(unsigned char)];    
	p_u = new unsigned char[height*width/4*sizeof(unsigned char)];
	p_v = new unsigned char[height*width/4*sizeof(unsigned char)];
}

void ReadIntoMemory(int k)
{
	fseek(file_color_input, FRAMESIZE*sizeof(unsigned char)*k*3/2, SEEK_SET);  //
	fread(src_color_y, 1, height*width, file_color_input);

	fseek(file_depth_input, FRAMESIZE*sizeof(unsigned char)*k*3/2, SEEK_SET);  //
	fread(src_depth_y, 1, height*width, file_depth_input);

}

 void DeleteSpace() 
 {
	 delete []src_color_y;
	 delete []src_depth_y;

	 delete []p_y;
	 delete []p_u;
	 delete []p_v;
 }