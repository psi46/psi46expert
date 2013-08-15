#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "Plane.h"


Plane::Plane(){
  init();
}


Plane::Plane(double x, double y, double z){
  init();
  fX0[0]=x;
  fX0[1]=y;
  fX0[2]=z;
}



void Plane::init(){
  for(int i=0; i<3; i++){
	 fX0[i]=0;
	 // nominal trafo
	 for(int j=0; j<3; j++){
		if(i==j){
		  fT [i][j]=1.;   // fT[*][i] = i-axis of local Frame in global coordinates
		  fQ [i][j]=1.;   // fQ[i][*} = i-axis of global Fram in local coordinates
		}else{
		  fT [i][j]=0.;
		  fQ [i][j]=0.;
		}
	 }
	 // shift/rotations
	 fDP [i]=0;
  }
}



void Plane::updateAngles(){
  // get the roation angles from the rotation matrix
  double theta=atan2(fT[0][1],fT[0][0]); // only if cosphi>0
  double sinphi=fT[0][2];
  double cosphi=fT[0][0]*cos(theta)+fT[0][1]*sin(theta);
  double phi=atan2(sinphi,cosphi);
  double psi=atan2(fT[1][2],fT[2][2]); // cosphi>0
  fDP[0]=psi;
  fDP[1]=phi;
  fDP[2]=theta;
  //printf("Plane::updateAngles> %10f  %10f  %10f \n",psi,phi,theta);
}


void Plane::rotate(double dpx, double dpy, double dpz){
  // construct rotation matrix (from see CST geometry note)
  // rotations first around z, then y, then x
  double R[3][3];
  double cospsi=cos(dpx);
  double sinpsi=sin(dpx);
  double cosphi=cos(dpy);
  double sinphi=sin(dpy);
  double costheta=cos(dpz);
  double sintheta=sin(dpz);
  R[0][0]= cosphi*costheta;
  R[0][1]= cosphi*sintheta;
  R[0][2]= sinphi;
  R[1][0]=-sinpsi*sinphi*costheta-cospsi*sintheta;
  R[1][1]=-sinpsi*sinphi*sintheta+cospsi*costheta;
  R[1][2]= sinpsi*cosphi;
  R[2][0]=-cospsi*sinphi*costheta+sinpsi*sintheta;
  R[2][1]=-cospsi*sinphi*sintheta-sinpsi*costheta;
  R[2][2] =cospsi*cosphi;
  
  // temporary copy of fT
  double T0[3][3];
  for(int i=0; i<3; i++){
	 for(int j=0; j<3; j++){
		T0[i][j]=fT[i][j];
	 }
  }
  // transform
  for(int i=0; i<3; i++){
	 for(int j=0; j<3; j++){
		fT[i][j]=0;
		for(int k=0; k<3; k++){
		  fT[i][j]+=R[i][k]*T0[k][j];
		}
	 }
  }
  // update inverse Trafo and angles
  invertTrafo();
  updateAngles();

}


void Plane::invertTrafo(){
  // fill the adjoint int fQ
  int p1[3]={1,0,0};
  int p2[3]={2,2,1};
  for(int i=0; i<3; i++){
	 for(int j=0; j<3; j++){
		fQ[i][j]=fT[p1[i]][p1[j]]*fT[p2[i]][p2[j]]-fT[p1[i]][p2[j]]*fT[p2[i]][p1[j]];
	 }
  }
  // get the determinant
  double det= 
	 fT[0][0]*fQ[0][0] - fT[0][1]*fQ[0][1] + fT[0][2]*fQ[0][2];
  //printf("Det = %10f\n",det);
  // dividing by the determinant yields the invers
  int sign=1;
  for(int i=0; i<3; i++){
	 for(int j=0; j<3; j++){
		fQ[i][j]=sign*fQ[i][j]/det;
		sign=-sign;
	 }
  }
  // should be equivalent to transposing here
}


void Plane::testTrafo(){
  for(int i=0; i<3; i++){
	 for(int j=0; j<3; j++){
		double g=0;
		for(int k=0; k<3; k++){
		  g+=fT[i][k]*fQ[k][j];
		}
		printf("%10f ",g);
	 }
	 printf("\n");
  }
}


void Plane::localToGlobal(double* xl, double* xg){
  for(int i=0; i<3; i++){
	 xg[i]=fX0[i];
	 for(int j=0; j<3; j++){
		xg[i]+=fT[i][j]*xl[j];
	 }
  }
}

void Plane::globalToLocal(double* xg, double* xl){
  for(int j=0; j<3; j++){
	 xl[j]=0;
	 for(int i=0; i<3; i++){
		xl[j]+=(xg[i]-fX0[i])*fT[i][j];
	 }
  }
}


void Plane::interceptGlobal(double* par, double* intercept){
  // intercept in global coordinates from track parameters
  double x0[3];
  x0[0]=par[0];
  x0[1]=par[1];
  x0[2]=0;
  double n[3];
  n[0]=par[2]/sqrt(1+par[2]*par[2]+par[3]*par[3]);
  n[1]=par[3]/sqrt(1+par[2]*par[2]+par[3]*par[3]);
  n[2]=     1/sqrt(1+par[2]*par[2]+par[3]*par[3]);
  interceptGlobal(x0,n,intercept);
}



void Plane::interceptGlobal(double* start, double* p, double* intercept){
  // intercept in global coordinates
  double a=0;
  double pn=0;
  for(int i=0; i<3; i++){
	 a+=fT[i][2]*(start[i]-fX0[i]);
	 pn+=fT[i][2]*p[i];
  }
  for(int i=0; i<3; i++){
	 intercept[i]=start[i]-a/pn*p[i];
  }
}

void Plane::interceptLocal(double* start, double* direction, double* intercept){
  // intercept in local coordinates
  double xglob[3];
  interceptGlobal(start, direction, xglob);
  globalToLocal(xglob, intercept);
}
