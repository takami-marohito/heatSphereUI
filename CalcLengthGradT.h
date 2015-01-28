#ifndef CALCLENGTHGRADT_INCLUDE
#define CALCLENGTHGRADT_INCLUDE

#include <stdio.h>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/Vector3>
#include <vector>

float LengthAandB3(float ax, float ay, float az, float bx, float by, float bz)
{
  float length = (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz);
  return(sqrt(length));
};


//prism 22.35 - 24.557
std::vector<float> CalcLengthGradT(kvs::UnstructuredVolumeObject *object, std::vector<unsigned int> surface_node, std::vector<float> GradT)
{

  std::cout << "Calculate Length GradT start" << std::endl;

  FILE *fp;
  fp = fopen("./lineconnection","r");

  std::vector<float> LengthGradT(object->numberOfNodes(),15);
  std::vector<std::vector<unsigned int>> lineconnection(surface_node.size());

  unsigned int tmp;
  fscanf(fp,"%u",&tmp);

  for(unsigned int i=0;i<surface_node.size();i++){
    unsigned int num;
    fscanf(fp,"%u",&num);
    for(unsigned int j=0;j<num;j++){
      unsigned int connection;
      fscanf(fp,"%u",&connection);
      lineconnection.at(i).push_back(connection);
    }
  }

  for(unsigned int i=0;i<surface_node.size();i++){
    unsigned int index = surface_node.at(i);
    float max_grad = GradT.at(index);
    unsigned int length = 0;
    unsigned int index2=index;

    for(unsigned int j=0;j<lineconnection.at(i).size();j++){
      if(max_grad < GradT.at(lineconnection.at(i).at(j))){
	index2 = lineconnection.at(i).at(j);
	max_grad = GradT.at(lineconnection.at(i).at(j));
	length = j;
      }
    }
    LengthGradT.at(surface_node.at(i)) = LengthAandB3(object->coords()[index2*3+0], object->coords()[index2*3+1], object->coords()[index2*3+2], 0, 0, 0);
  }
  return(LengthGradT);
};

#endif

