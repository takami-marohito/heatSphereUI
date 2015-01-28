#ifndef CALCLENGTHSEI_INCLUDE
#define CALCLENGTHSEI_INCLUDE

#include <stdio.h>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/Vector3>
#include <vector>

float LengthAandB4(float ax, float ay, float az, float bx, float by, float bz)
{
  float length = (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz);
  return(sqrt(length));
};


//prism 22.35 - 24.557
std::vector<float> CalcLengthSecondInvariant(kvs::UnstructuredVolumeObject *object, std::vector<unsigned int> surface_node, std::vector<float> Sei, float target_sei)
{
  int count = 0;
  std::cout << "Calculate Length SecondInvariant start" << std::endl;

  FILE *fp;
  fp = fopen("./lineconnection","r");

  std::vector<float> LengthSei(object->numberOfNodes(),0);
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
    unsigned int index1 = surface_node.at(i);
    unsigned int index2 = index1;
    unsigned int target_index = index1;
    unsigned int length = 0;
    bool flag = false;
    for(unsigned int j=1;j<lineconnection.at(i).size();j++){
      index1 = index2;
      index2 = lineconnection.at(i).at(j);
      if((Sei.at(index1) > target_sei&&Sei.at(index2) < target_sei) || (Sei.at(index1) < target_sei&&Sei.at(index2) > target_sei) ){
	length = j;
	flag = true;
	break;
      }
    }
    //printf("%u %u %u ",i,index1,index2);
    if(flag == false){
      if(Sei.at(index1) < target_sei){
	//printf("a\n");
	LengthSei.at(surface_node.at(i)) = 0;
      }
      if(Sei.at(index1) > target_sei){
	//printf("b\n");
	LengthSei.at(surface_node.at(i)) = 22.5;
      }
    }else{
      float length1 = LengthAandB4(object->coords()[index1*3+0], object->coords()[index1*3+1], object->coords()[index1*3+2], 0, 0, 0);
      float length2 = LengthAandB4(object->coords()[index2*3+0], object->coords()[index2*3+1], object->coords()[index2*3+2], 0, 0, 0);
      //printf("%f %f",length1, length2);
      float float_Sei = fabs(target_sei-Sei.at(index1)) / fabs(Sei.at(index2)-Sei.at(index1));
      //printf("c\n");
      LengthSei.at(surface_node.at(i)) = length1 + float_Sei*(length2-length1);
      count++;
      //LengthSei.at(surface_node.at(i)) = length1 + float_Sei*(length2-length1);
    }
  }
  //printf("%d %f finish\n",count, max_length);
  return(LengthSei);
};

#endif

