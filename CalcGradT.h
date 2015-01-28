#ifndef CALCGRADT_INCLUDE
#define CALCGRADT_INCLUDE

#include <stdio.h>
#include <kvs/UnstructuredVolumeObject>
#include <vector>
#include <math.h>

float LengthAandB2(float ax, float ay, float az, float bx, float by, float bz)
{
  float length = (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz);
  return(sqrt(length));
};

//2点間の温度勾配
float CalcGradTFrom2Points(kvs::UnstructuredVolumeObject* object, unsigned int index1, unsigned int index2, std::vector<float> *t)
{
  float dr = LengthAandB2(object->coords()[index1*3+0], object->coords()[index1*3+1], object->coords()[index1*3+2], object->coords()[index2*3+0], object->coords()[index2*3+1], object->coords()[index2*3+2]);

  float dt = t->at(index2) - t->at(index1);

  float viscosity = 1.0;

  float returnvalue = fabs(viscosity*dt/dr);

  return(returnvalue);
  
};


//objectはすべてのprism格子をconnectionにしている
//connectionは球の表面から並んでいて、外に広がっていると仮定する
//セルの最初の3つの番号は球の表面側であとの3つが外側と改定する
std::vector<float> CalcAbsGradT(kvs::UnstructuredVolumeObject* object, std::vector<float> t)
{
  std::cout << "Calculate Gradient T in Prism" << std::endl;
  std::vector<float> GradT(object->numberOfNodes(),0);

  std::vector<bool> calc_flag(object->numberOfNodes(), false);
  //計算済みの場合はcalc_flag.at(i) = true;

  for(unsigned int i=0;i<object->numberOfCells();i++){
    for(unsigned int k=0;k<3;k++){
      if(i*6+k+3>=object->numberOfCells()*6-1){
	break;
      }
      unsigned int index1 = object->connections()[i*6+k];
      unsigned int index2 = object->connections()[i*6+k+3];

      if(calc_flag.at(index1) == true){
      }else{
	GradT.at(index1) = CalcGradTFrom2Points(object, index1, index2, &t);
	calc_flag.at(index1) = true;
      }
    }
  }
  return(GradT);
};

#endif
