
#include "ngspice.h"
#include <stdio.h>
#include "ifsim.h"
#include "hfet2defs.h"
#include "sperror.h"
#include "suffix.h"


int HFET2param(param, value, inst, select)
int param;
IFvalue *value;
GENinstance *inst;
IFvalue *select;
{
  
  HFET2instance *here = (HFET2instance*)inst;
  switch(param) {
    case HFET2_LENGTH:
      L = value->rValue;
      here->HFET2lengthGiven = TRUE;
      break;
    case HFET2_IC_VDS:
      here->HFET2icVDS = value->rValue;
      here->HFET2icVDSGiven = TRUE;
      break;
    case HFET2_IC_VGS:
      here->HFET2icVGS = value->rValue;
      here->HFET2icVGSGiven = TRUE;
      break;
    case HFET2_OFF:
      here->HFET2off = value->iValue;
      break;
    case HFET2_IC:
      switch(value->v.numValue) {
        case 2:
          here->HFET2icVGS = *(value->v.vec.rVec+1);
          here->HFET2icVGSGiven = TRUE;
        case 1:
          here->HFET2icVDS = *(value->v.vec.rVec);
          here->HFET2icVDSGiven = TRUE;
          break;
        default:
          return(E_BADPARM);
      }
      break;
    case HFET2_TEMP:
      TEMP = value->rValue+CONSTCtoK;
      here->HFET2tempGiven = TRUE;
      break;
    case HFET2_WIDTH:
      W = value->rValue;
      here->HFET2widthGiven = TRUE;
      break;
    default:
      return(E_BADPARM);
  }
  return(OK);
  
}
