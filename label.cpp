#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include "bmplib.h" 
#include "queue.h"
using namespace std;

void usage() { 
    cerr << "usage: ./label <options>" << endl;
    cerr <<"Examples" << endl;
    cerr << "./label test_queue" << endl;
    cerr << "./label gray <input file> <outputfile>" << endl;
    cerr << "./label binary <inputfile> <outputfile>" << endl;
    cerr << "./label segment <inputfile> <outputfile>" << endl;
}

//function prototypes
void test_queue();
void rgb2gray(unsigned char ***in,unsigned char **out,int height,int width);
void gray2binary(unsigned char **in,unsigned char **out,int height,int width);
int component_labeling(unsigned char **binary_image,int **labeled_image,int height,int width);
void label2RGB(int  **labeled_image, unsigned char ***rgb_image,int num_segment,int height,int width);
void clean(unsigned char ***input,unsigned char **gray,unsigned char **binary, int **labeled_image,int height , int width);


// main function
int main(int argc,char **argv) {

    srand( time(0) );
    if(argc < 2 )  {
        usage();
        return -1;
    }        
    unsigned char ***input=0;
    unsigned char **gray=0;
    unsigned char **binary=0;
    int **labeled_image=0;
    if( strcmp("test_queue",argv[1]) == 0 ) { 
        test_queue();
    } 
    else if(strcmp("gray",argv[1]) == 0 ) {
        if(argc <4 ) {
            cerr << "not enough arguemnt for gray" << endl;
            return -1;
        }
        int height,width;
        input = readRGBBMP(argv[2],&height,&width);
        if(input == 0)
        {
            cerr << "unable to open " << argv[2] << " for input." << endl;
            return -1;
        }            
        gray = new unsigned char*[height];
        for(int i=0;i<height;i++) 
            gray[i] = new unsigned char[width];
        rgb2gray(input,gray,height,width);
        if(writeGSBMP(argv[3],gray,height,width) != 0) { 
            cerr << "error writing file " << argv[3] << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }
        clean(input,gray,binary,labeled_image,height,width);

    }
    else if(strcmp("binary",argv[1]) == 0 ) {
        if(argc <4 ) {
            cerr << "not enough arguemnt for binary" << endl;
            return -1;
        }            
        int height,width;
        input = readRGBBMP(argv[2],&height,&width);
        if(input == 0)
        {
            cerr << "unable to open " << argv[2] << " for input." << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }            

        gray = new unsigned char*[height];
        for(int i=0;i<height;i++) 
            gray[i] = new unsigned char[width];
        rgb2gray(input,gray,height,width);

        binary = new unsigned char*[height];
        for(int i=0;i<height;i++)
            binary[i] = new unsigned char[width];

        gray2binary(gray,binary,height,width);
        if(writeBinary(argv[3],binary,height,width) != 0) { 
            cerr << "error writing file " << argv[3] << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }
        clean(input,gray,binary,labeled_image,height,width);
     
    }
    else if(strcmp("segment",argv[1]) == 0 ) {
        if(argc <4 ) {
            cerr << "not enough arguemnt for segment" << endl;
            return -1;
        } 
        int height,width;
        input = readRGBBMP(argv[2],&height,&width);
        if(input == 0)
        {
            cerr << "unable to open " << argv[2] << " for input." << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }            

        gray = new unsigned char*[height];
        for(int i=0;i<height;i++) 
            gray[i] = new unsigned char[width];
        rgb2gray(input,gray,height,width);

        binary = new unsigned char*[height];
        for(int i=0;i<height;i++)
            binary[i] = new unsigned char[height];

        gray2binary(gray,binary,height,width);


        labeled_image = new int*[height];
        for(int i=0;i<height;i++) labeled_image[i] = new int[width];
        int segments= component_labeling(binary, labeled_image,height,width); 

        for(int i=0;i<height;i++) 
            for(int j=0;j<width;j++) 
                for(int k=0;k<RGB;k++) 
                    input[i][j][k] = 0;
        label2RGB(labeled_image, input ,segments, height,width);
        if(writeRGBBMP(argv[3],input,height,width) != 0) {
            cerr << "error writing file " << argv[3] << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }
        clean(input,gray,binary,labeled_image,height,width);

    }

   return 0;
}



void test_queue() { 
    // create some locations;
    Location three_one, two_two;
    three_one.row = 3; three_one.col = 1;
    two_two.row = 2; two_two.col = 2;

    //create an Queue with max capacity 5
    Queue q(5);

    cout << boolalpha;
    cout << q.is_empty() << endl;           // true
    q.push(three_one);
    cout << q.is_empty() << endl;           // false
    q.push(two_two);

    Location loc = q.pop();
    cout << loc.row << " " << loc.col << endl; // 3 1
    loc = q.pop();
    cout << loc.row << " " << loc.col << endl; // 2 2
    cout << q.is_empty() << endl;           // true
}


void rgb2gray(unsigned char ***in,unsigned char **out,int height,int width) {
    double gs; 
    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) { 
            gs = (in[i][j][0]*0.2989) + (in[i][j][1]*0.5870) + (in[i][j][2]*0.1140); 
            out[i][j] = (unsigned char) gs; 
        }
    }
     
}


void gray2binary(unsigned char **in,unsigned char **out,int height,int width) {
    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) { 
            if (in[i][j] > THRESHOLD) { 
                out[i][j] = 1;
            } else { 
                out[i][j] = 0; 
            }
        }
    }
}

//This is the function that does the work of looping over the binary image and doing the connected component labeling
int component_labeling(unsigned char **binary_image,int **label,int height,int width) {
    Queue q(height*width);
    Location loc;
    int segs = 1;

    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) { 
            label[i][j] = 0; 
        }
    }

    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) { 
            if (binary_image[i][j] == 1 && label[i][j] == 0) { 
                loc.col = i; 
                loc.row = j; 
                label[loc.col][loc.row] = segs; 
                q.push(loc); 
                while (!q.is_empty()) { 
                    loc = q.pop(); 
                    if (binary_image[loc.col+1][loc.row] == 1 && label[loc.col+1][loc.row] == 0) { 
                        Location north; 
                        north.col = loc.col+1; 
                        north.row = loc.row; 
                        label[north.col][north.row] = segs; 
                        q.push(north); 
                    }
                    if (binary_image[loc.col-1][loc.row] == 1 && label[loc.col-1][loc.row] == 0) { 
                        Location south; 
                        south.col = loc.col-1; 
                        south.row = loc.row; 
                        label[south.col][south.row] = segs; 
                        q.push(south); 
                    }
                    if (binary_image[loc.col][loc.row+1] == 1 && label[loc.col][loc.row+1] == 0) { 
                        Location east; 
                        east.col = loc.col; 
                        east.row = loc.row+1; 
                        label[east.col][east.row] = segs; 
                        q.push(east); 
                    }
                    if (binary_image[loc.col][loc.row-1] == 1 && label[loc.col][loc.row-1] == 0) { 
                        Location west; 
                        west.col = loc.col; 
                        west.row = loc.row-1; 
                        label[west.col][west.row] = segs; 
                        q.push(west); 
                    }
                }
                segs++; 
            }
        }
    }
    return segs; 
}    

//First make num_segments number of random colors to use for coloring the labeled parts of the image.
//Then loop over the labeled image using the label to index into random colors array.
//Set the rgb_pixel to the corresponding color, or set to black if the pixel was unlabeled.
#ifndef AUTOTEST
void label2RGB(int  **labeled_image, unsigned char ***rgb_image,int num_segments,int height,int width)
{   
    srand(time(0));  

    int** color = new int* [num_segments];
    for (int i = 0; i < num_segments; i++) { 
        color[i] = new int[RGB]; 
    }
    for (int i = 0; i < num_segments; i++) { 
        for (int j = 0; j < RGB; j++) {
            color[i][j] = (rand() % 255);   
        } 
    }

    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) { 
            for (int k = 0; k < RGB; k++) { 
                rgb_image[i][j][k] = 0; 
            }
        }
    }

    for (int k = 0; k < num_segments; k++) { 
        for (int i = 0; i < height; i++) { 
            for (int j = 0; j < width; j++) { 
                if (labeled_image[i][j] == k+1) { 
                    for (int m = 0; m < RGB; m++) { 
                        rgb_image[i][j][m] = (unsigned char) color[k][m]; 
                    } 
                } 
            }
        }
    } 
}
#endif

//function to delete all of the dynamic arrays created
void clean(unsigned char ***input,unsigned char **gray,unsigned char **binary, int **labeled_image,int height , int width) {
    if(input ) {
            //delete allocated input image array here
        for (int i = 0; i < height; i++) { 
            for (int j = 0; j < width; j++) { 
                delete[] input[i][j]; 
            }
            delete[] input[i]; 
        }
        delete[] input; 
    } if(gray) {
        //delete gray-scale image here
        for (int i = 0; i < height; i++) { 
            delete[] gray[i]; 
        }
        delete[] gray; 
    } if(binary) {
        //delete binary image array here
        for (int i = 0; i < height; i++) { 
            delete[] binary[i]; 
        }
        delete[] binary; 
    } if(labeled_image) {
        //delete labeled array here
        for (int i = 0; i < height; i++) { 
            delete[] labeled_image[i]; 
        }
        delete[] labeled_image; 
    }


}