#ifndef CALCSECONDINVARIANT_INCLUDE
#define CALCSECONDINVARIANT_INCLUDE


#include <stdio.h>
#include <kvs/UnstructuredVolumeObject>
#include <vector>
#include <algorithm>

#include <math.h>

float LengthAandB(float ax, float ay, float az, float bx, float by, float bz)
{
  float length = (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz);
  return(sqrt(length));
};

float CalcLength(float u, float v, float w, float x, float y, float z ){
  return (sqrt( (u-x)*(u-x)+(v-y)*(v-y)+(w-z)*(w-z) ));
};


float CalcSecondInvariantFromTensor(float v_tensor00,float v_tensor01, float v_tensor02, float v_tensor10, float v_tensor11, float v_tensor12, float v_tensor20, float v_tensor21, float v_tensor22){
  return(  v_tensor01*v_tensor10 + v_tensor12*v_tensor21 + v_tensor20*v_tensor02 - v_tensor00*v_tensor11 - v_tensor11*v_tensor22 - v_tensor22*v_tensor00 );
};


std::vector<float> CalcSecondInvariant_Prism(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w)
{
  std::cout << "Calculate Second Invariant at the surface start" << std::endl;

  std::vector<std::vector<unsigned int>> Pconnection(object->numberOfNodes());


  //
  float max_x=0;
  float max_y=0;
  float max_z=0;

  for(unsigned int i=0;i<object->numberOfCells();i++){
    for(unsigned int j=0;j<6;j++){
      unsigned int index1 = object->connections()[i*6+j];
      for(unsigned int k=0;k<6;k++){
	unsigned int index2 = object->connections()[i*6+k];
	int count=0;
	for(unsigned int l=0;l<Pconnection.at(index1).size();l++){
	  if(index2==Pconnection.at(index1).at(l)){
	    break;
	  }
	  count++;
	}
	if(count==Pconnection.at(index1).size() && index1!=index2){
	  Pconnection.at(index1).push_back(index2);
	}
      }
    }
  }
  ///ここまででPconnection.at(index)には
  ///index番目の頂点を持つprismの全ての点が入る（重複なし）
  int count2=0;
  for(unsigned int i=0;i<object->numberOfNodes();i++){
    if(Pconnection.at(i).size()==0){
      count2++;
    }
  }
  std::vector<float> SecInv(object->numberOfNodes(), 0);
  std::vector<float> sum_length(object->numberOfNodes(), 0);
  std::vector<float> sum_v_tensor(object->numberOfNodes()*9,0);


  for(unsigned int i=0;i<object->numberOfNodes();i++){
    for(unsigned int j=0;j<Pconnection.at(i).size();j++){
      unsigned int index1=i;
      unsigned int index2=Pconnection.at(index1).at(j);

      float x = object->coords()[index1*3+0];
      float y = object->coords()[index1*3+1];
      float z = object->coords()[index1*3+2];
      
      float u = object->coords()[index2*3+0];
      float v = object->coords()[index2*3+1];
      float w = object->coords()[index2*3+2];

      float length = CalcLength(x,y,z,u,v,w );

      float v_tensor00, v_tensor01, v_tensor02, v_tensor10, v_tensor11, v_tensor12, v_tensor20, v_tensor21, v_tensor22;

      v_tensor00 = (vel_u.at(index1)-vel_u.at(index2))/(x-u);
      v_tensor10 = (vel_u.at(index1)-vel_u.at(index2))/(y-v);
      v_tensor20 = (vel_u.at(index1)-vel_u.at(index2))/(z-w);
      v_tensor01 = (vel_v.at(index1)-vel_v.at(index2))/(x-u);
      v_tensor11 = (vel_v.at(index1)-vel_v.at(index2))/(y-v);
      v_tensor21 = (vel_v.at(index1)-vel_v.at(index2))/(z-w);
      v_tensor02 = (vel_w.at(index1)-vel_w.at(index2))/(x-u);
      v_tensor12 = (vel_w.at(index1)-vel_w.at(index2))/(y-v);
      v_tensor22 = (vel_w.at(index1)-vel_w.at(index2))/(z-w);
      
      

      if(fabs(x-u)<0.01||fabs(y-v)<0.01||fabs(z-w)<0.01){
      //if(x-u<0.001||y-v<0.001||z-w<0.001){
      }else if(isinf(v_tensor00) || isinf(v_tensor01) || isinf(v_tensor02) || isinf(v_tensor10) || isinf(v_tensor11) || isinf(v_tensor12) || isinf(v_tensor20) || isinf(v_tensor21) || isinf(v_tensor22)){
      }else if(NAN == v_tensor00 || NAN == v_tensor01 || NAN == v_tensor02 || NAN == v_tensor10 || NAN == v_tensor20 || NAN == v_tensor11 || NAN == v_tensor12 || NAN == v_tensor22 || NAN == v_tensor21){
      }else if(-NAN == v_tensor00 || -NAN == v_tensor01 || -NAN == v_tensor02 || -NAN == v_tensor10 || -NAN == v_tensor20 || -NAN == v_tensor11 || -NAN == v_tensor12 || -NAN == v_tensor22 || -NAN == v_tensor21){
      }else{
	sum_v_tensor.at(index1*9+0) += 1.0/length*v_tensor00;
	sum_v_tensor.at(index1*9+1) += 1.0/length*v_tensor01;
	sum_v_tensor.at(index1*9+2) += 1.0/length*v_tensor02;
	sum_v_tensor.at(index1*9+3) += 1.0/length*v_tensor10;
	sum_v_tensor.at(index1*9+4) += 1.0/length*v_tensor11;
	sum_v_tensor.at(index1*9+5) += 1.0/length*v_tensor12;
	sum_v_tensor.at(index1*9+6) += 1.0/length*v_tensor20;
	sum_v_tensor.at(index1*9+7) += 1.0/length*v_tensor21;
	sum_v_tensor.at(index1*9+8) += 1.0/length*v_tensor22;
	sum_length.at(index1) += 1.0/length;
      }
    }
  }

  int count = 0;
  for(unsigned int i=0;i<object->numberOfNodes();i++){
    if(sum_length.at(i)==0){
      count++;
      SecInv.at(i)=0;
    }else{
      SecInv.at(i) = CalcSecondInvariantFromTensor(sum_v_tensor.at(i*9+0)/sum_length.at(i),sum_v_tensor.at(i*9+1)/sum_length.at(i),sum_v_tensor.at(i*9+2)/sum_length.at(i),sum_v_tensor.at(i*9+3)/sum_length.at(i),sum_v_tensor.at(i*9+4)/sum_length.at(i),sum_v_tensor.at(i*9+5)/sum_length.at(i),sum_v_tensor.at(i*9+6)/sum_length.at(i),sum_v_tensor.at(i*9+7)/sum_length.at(i),sum_v_tensor.at(i*9+8)/sum_length.at(i));
    }
    if(isnan(SecInv.at(i)) ){
      SecInv.at(i) = 0;
    }
  }
  std::vector<std::vector<unsigned int>>().swap(Pconnection);
  std::vector<float>().swap(sum_v_tensor);
  std::vector<float>().swap(sum_length);
  //printf("no count is %d %d\n",count, count2);
  return(SecInv);
};


#endif
