#include "ngspice.h"
#include <stdio.h>
#include "devdefs.h"
#include "cktdefs.h"
#include "hfetdefs.h"
#include "const.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"
/*
#define true 1
#define false 0
*/

//#define PHIB 0.5
double diode(double);

static void leak(double gmin, double vt, double v, double rs, double is1, 
				 double is2, double m1, double m2, double *il, double *gl);
                 
static void hfeta(HFETAmodel *model, HFETAinstance *here, CKTcircuit *ckt,
                  double vgs, double vds, double *cdrain, double *gm,
                  double *gds, double *capgs, double *capgd,
                  double *cgd, double *gmg, double *gmd,
                  double *cgs, double *ggs);
                 
void Pause(void);

int HFETAload(inModel, ckt)
GENmodel *inModel;
register CKTcircuit *ckt;
{
    register HFETAmodel *model = (HFETAmodel*)inModel;
    register HFETAinstance *here;
    double capgd;
    double capgs;
    double cd;
    double cdhat;
    double cdrain;
    double cdreq;
    double ceq;
    double ceqgd;
    double ceqgs;
    double cg;
    double cgd=0;
    double cgs=0;
    double cghat;
    double delvds;
    double delvgd;
    double delvgs;
    double delvgdpp=0;
    double delvgspp=0;
    double gds;
    double geq;
    double ggd=0;
    double ggs=0;
    double gm;
    double vcrit;
    double vds;
    double vgd;
    double vgs;
    double vgs1;
    double vgd1;
    double vds1;
    double xfact;
    double temp;
    double vt;
    double vgspp=0;
    double vgdpp=0;
    double cgspp=0;
    double cgdpp=0;
    double ggspp=0;
    double ggdpp=0;
    double gmg=0;
    double gmd=0;
    
    int inverse=FALSE;
    int icheck;
    int error;

    for( ; model != NULL; model = model->HFETAnextModel ) {
        for (here = model->HFETAinstances; here != NULL ;
                here=here->HFETAnextInstance) {
            vcrit = here->HFETAvcrit;
            vt  = CONSTKoverQ * here->HFETAtemp;
            icheck = 0;
            if( ckt->CKTmode & MODEINITSMSIG) {
                vgs = *(ckt->CKTstate0 + here->HFETAvgs);
                vgd = *(ckt->CKTstate0 + here->HFETAvgd);
                vgspp = *(ckt->CKTstate0 + here->HFETAvgspp);
                vgdpp = *(ckt->CKTstate0 + here->HFETAvgdpp);
            } else if (ckt->CKTmode & MODEINITTRAN) {
                vgs = *(ckt->CKTstate1 + here->HFETAvgs);
                vgd = *(ckt->CKTstate1 + here->HFETAvgd);
                vgspp = *(ckt->CKTstate1 + here->HFETAvgspp);
                vgdpp = *(ckt->CKTstate1 + here->HFETAvgdpp);
            } else if ( (ckt->CKTmode & MODEINITJCT) &&
                    (ckt->CKTmode & MODETRANOP) &&
                    (ckt->CKTmode & MODEUIC) ) {
                vds = model->HFETAtype*here->HFETAicVDS;
                vgs = model->HFETAtype*here->HFETAicVGS;
                vgd = vgs-vds;
                vgspp = vgs;
                vgdpp = vgd;
            } else if ( (ckt->CKTmode & MODEINITJCT) &&
                    (here->HFETAoff == 0)  ) {
                vgs = -1;
                vgd = -1;
                vgspp = 0;
                vgdpp = 0;
            } else if( (ckt->CKTmode & MODEINITJCT) ||
                    ((ckt->CKTmode & MODEINITFIX) && (here->HFETAoff))) {
                vgs = 0;
                vgd = 0;
                vgspp = 0;
                vgdpp = 0;
            } else {
#ifndef PREDICTOR
                if(ckt->CKTmode & MODEINITPRED) {
                    xfact = ckt->CKTdelta/ckt->CKTdeltaOld[2];
                    *(ckt->CKTstate0 + here->HFETAvgs) =
                            *(ckt->CKTstate1 + here->HFETAvgs);
                    vgs = (1+xfact) * *(ckt->CKTstate1 + here->HFETAvgs) -
                           xfact * *(ckt->CKTstate2 + here->HFETAvgs);
                    *(ckt->CKTstate0 + here->HFETAvgspp) = 
                            *(ckt->CKTstate1 + here->HFETAvgspp);
                    vgspp = (1+xfact) * *(ckt->CKTstate1 + here->HFETAvgspp) -
                           xfact * *(ckt->CKTstate2 + here->HFETAvgspp);
                    *(ckt->CKTstate0 + here->HFETAvgd) = 
                            *(ckt->CKTstate1 + here->HFETAvgd);
                    vgd = (1+xfact)* *(ckt->CKTstate1 + here->HFETAvgd) -
                           xfact * *(ckt->CKTstate2 + here->HFETAvgd);
                    *(ckt->CKTstate0 + here->HFETAvgdpp) = 
                            *(ckt->CKTstate1 + here->HFETAvgdpp);
                    vgdpp = (1+xfact) * *(ckt->CKTstate1 + here->HFETAvgdpp) -
                           xfact * *(ckt->CKTstate2 + here->HFETAvgdpp);
                    *(ckt->CKTstate0 + here->HFETAcg) = 
                            *(ckt->CKTstate1 + here->HFETAcg);
                    *(ckt->CKTstate0 + here->HFETAcd) = 
                            *(ckt->CKTstate1 + here->HFETAcd);
                    *(ckt->CKTstate0 + here->HFETAcgd) =
                            *(ckt->CKTstate1 + here->HFETAcgd);
                    *(ckt->CKTstate0 + here->HFETAcgs) =
                            *(ckt->CKTstate1 + here->HFETAcgs);
                    *(ckt->CKTstate0 + here->HFETAcgspp) =
                            *(ckt->CKTstate1 + here->HFETAcgspp);
                    *(ckt->CKTstate0 + here->HFETAcgdpp) =
                            *(ckt->CKTstate1 + here->HFETAcgdpp);
                    *(ckt->CKTstate0 + here->HFETAgm) =
                            *(ckt->CKTstate1 + here->HFETAgm);
                    *(ckt->CKTstate0 + here->HFETAgds) =
                            *(ckt->CKTstate1 + here->HFETAgds);
                    *(ckt->CKTstate0 + here->HFETAggs) =
                            *(ckt->CKTstate1 + here->HFETAggs);
                    *(ckt->CKTstate0 + here->HFETAggspp) =
                            *(ckt->CKTstate1 + here->HFETAggspp);
                    *(ckt->CKTstate0 + here->HFETAggd) =
                            *(ckt->CKTstate1 + here->HFETAggd);
                    *(ckt->CKTstate0 + here->HFETAggdpp) =
                            *(ckt->CKTstate1 + here->HFETAggdpp);
                    *(ckt->CKTstate0 + here->HFETAgmg) =
                            *(ckt->CKTstate1 + here->HFETAgmg);
                    *(ckt->CKTstate0 + here->HFETAgmd) =
                            *(ckt->CKTstate1 + here->HFETAgmd);        
                } else {
#endif /* PREDICTOR */
                    /*
                     *  compute new nonlinear branch voltages 
                     */
                    vgs = model->HFETAtype*
                        (*(ckt->CKTrhsOld+ here->HFETAgatePrimeNode)-
                        *(ckt->CKTrhsOld+ 
                        here->HFETAsourcePrimeNode));
                    vgd = model->HFETAtype*
                        (*(ckt->CKTrhsOld+here->HFETAgatePrimeNode)-
                        *(ckt->CKTrhsOld+
                        here->HFETAdrainPrimeNode));
                    vgspp = model->HFETAtype*
                        (*(ckt->CKTrhsOld+ here->HFETAgatePrimeNode)-
                        *(ckt->CKTrhsOld+ 
                        here->HFETAsourcePrmPrmNode));
                    vgdpp = model->HFETAtype*
                        (*(ckt->CKTrhsOld+ here->HFETAgatePrimeNode)-
                        *(ckt->CKTrhsOld+ 
                        here->HFETAdrainPrmPrmNode));
                        
#ifndef PREDICTOR
                }
#endif /* PREDICTOR */
                delvgs=vgs - *(ckt->CKTstate0 + here->HFETAvgs);
                delvgd=vgd - *(ckt->CKTstate0 + here->HFETAvgd);
                delvds=delvgs - delvgd;
                delvgspp=vgspp - *(ckt->CKTstate0 + here->HFETAvgspp);
                delvgdpp=vgdpp - *(ckt->CKTstate0 + here->HFETAvgdpp);
                cghat= *(ckt->CKTstate0 + here->HFETAcg) + 
                        *(ckt->CKTstate0 + here->HFETAgmg)*delvgs -
                        *(ckt->CKTstate0 + here->HFETAgmd)*delvds +                
                        *(ckt->CKTstate0 + here->HFETAggd)*delvgd +
                        *(ckt->CKTstate0 + here->HFETAggs)*delvgs +
                        *(ckt->CKTstate0 + here->HFETAggdpp)*delvgdpp +
                        *(ckt->CKTstate0 + here->HFETAggspp)*delvgspp;
                cdhat= *(ckt->CKTstate0 + here->HFETAcd) +
                        *(ckt->CKTstate0 + here->HFETAgm)*delvgs +
                        *(ckt->CKTstate0 + here->HFETAgds)*delvds -
                        *(ckt->CKTstate0 + here->HFETAggd)*delvgd -
                        (*(ckt->CKTstate0 + here->HFETAgmg)*delvgs -
                        *(ckt->CKTstate0 + here->HFETAgmd)*delvds);
                /*
                 *   bypass if solution has not changed 
                 */
                if((ckt->CKTbypass) &&
                    (!(ckt->CKTmode & MODEINITPRED)) &&
                    (fabs(delvgs) < ckt->CKTreltol*MAX(fabs(vgs),
                        fabs(*(ckt->CKTstate0 + here->HFETAvgs)))+
                        ckt->CKTvoltTol) )
                if ( (fabs(delvgd) < ckt->CKTreltol*MAX(fabs(vgd),
                        fabs(*(ckt->CKTstate0 + here->HFETAvgd)))+
                        ckt->CKTvoltTol))
                if ( (fabs(delvgspp) < ckt->CKTreltol*MAX(fabs(vgspp),
                        fabs(*(ckt->CKTstate0 + here->HFETAvgspp)))+
                        ckt->CKTvoltTol))
                if ( (fabs(delvgdpp) < ckt->CKTreltol*MAX(fabs(vgdpp),
                        fabs(*(ckt->CKTstate0 + here->HFETAvgdpp)))+
                        ckt->CKTvoltTol))
                if ( (fabs(cghat-*(ckt->CKTstate0 + here->HFETAcg)) 
                        < ckt->CKTreltol*MAX(fabs(cghat),
                        fabs(*(ckt->CKTstate0 + here->HFETAcg)))+
                        ckt->CKTabstol) ) if ( /* hack - expression too big */
                    (fabs(cdhat-*(ckt->CKTstate0 + here->HFETAcd))
                        < ckt->CKTreltol*MAX(fabs(cdhat),
                        fabs(*(ckt->CKTstate0 + here->HFETAcd)))+
                        ckt->CKTabstol) ) {

                    /* we can do a bypass */
                    vgs   = *(ckt->CKTstate0 + here->HFETAvgs);
                    vgd   = *(ckt->CKTstate0 + here->HFETAvgd);
                    vds   = vgs-vgd;
                    vgspp = *(ckt->CKTstate0 + here->HFETAvgspp);
                    vgdpp = *(ckt->CKTstate0 + here->HFETAvgdpp);
                    cg    = *(ckt->CKTstate0 + here->HFETAcg);
                    cd    = *(ckt->CKTstate0 + here->HFETAcd);
                    cgd   = *(ckt->CKTstate0 + here->HFETAcgd);
                    cgs   = *(ckt->CKTstate0 + here->HFETAcgs);
                    cgdpp = *(ckt->CKTstate0 + here->HFETAcgdpp);
                    cgspp = *(ckt->CKTstate0 + here->HFETAcgspp);
                    gm    = *(ckt->CKTstate0 + here->HFETAgm);
                    gds   = *(ckt->CKTstate0 + here->HFETAgds);
                    ggs   = *(ckt->CKTstate0 + here->HFETAggs);
                    ggd   = *(ckt->CKTstate0 + here->HFETAggd);
                    ggdpp = *(ckt->CKTstate0 + here->HFETAggdpp);
                    ggspp = *(ckt->CKTstate0 + here->HFETAggspp);
                    gmg   = *(ckt->CKTstate0 + here->HFETAgmg);
                    gmd   = *(ckt->CKTstate0 + here->HFETAgmd);
                    goto load;
                }
                /*
                 *  limit nonlinear branch voltages 
                 */
                vgs = DEVfetlim(vgs,*(ckt->CKTstate0 + here->HFETAvgs),TVTO);
                vgd = DEVfetlim(vgd,*(ckt->CKTstate0 + here->HFETAvgd),TVTO);
            }
            /*
             *   determine dc current and derivatives 
             */
            vds = vgs-vgd;
            if(model->HFETAgatemod == 0) {
              double arg;
              double earg;
              if(IS1S == 0 || IS2S == 0) {
                cgs = 0;
                ggs = 0;
              } else
                leak(ckt->CKTgmin,vt,vgs,RGS,IS1S,IS2S,M1S,M2S,&cgs,&ggs);
              arg = -vgs*DEL/vt;
              earg = exp(arg);
              cgs += GGRWL*vgs*earg;
              ggs += GGRWL*earg*(1-arg);  
              if(IS1D == 0 || IS2D == 0) {
                cgd = 0;
                ggd = 0;
              } else
                leak(ckt->CKTgmin,vt,vgd,RGD,IS1D,IS2D,M1D,M2D,&cgd,&ggd);
              arg = -vgd*DEL/vt;
              earg = exp(arg);
              cgd += GGRWL*vgd*earg;
              ggd += GGRWL*earg*(1-arg);  
            } else
              ggd = 0;
            if(vds < 0) {
              vds = -vds;
              inverse = TRUE;
            }
            hfeta(model,here,ckt,vds>0?vgs:vgd,vds,&cdrain,&gm,&gds,&capgs,&capgd,
                  &cgd,&gmg,&gmd,&cgs,&ggs);
            cg = cgs+cgd;
            if(inverse) {
              cdrain = -cdrain;
              vds = -vds;
              temp = capgs;
              capgs = capgd;
              capgd = temp;
            }
            /*
             *   compute equivalent drain current source 
             */
            cd = cdrain - cgd;
            if ( (ckt->CKTmode & (MODETRAN|MODEINITSMSIG)) ||
                    ((ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC)) ){
                /* 
                 *    charge storage elements 
                 */
                vgs1 = *(ckt->CKTstate1 + here->HFETAvgspp);
                vgd1 = *(ckt->CKTstate1 + here->HFETAvgdpp);
                vds1 = *(ckt->CKTstate1 + here->HFETAvgs)-
                       *(ckt->CKTstate1 + here->HFETAvgd);

                if(ckt->CKTmode & MODEINITTRAN) {
                    *(ckt->CKTstate1 + here->HFETAqgs) = capgs*vgspp;
                    *(ckt->CKTstate1 + here->HFETAqgd) = capgd*vgdpp;
                    *(ckt->CKTstate1 + here->HFETAqds) = CDS*vds;
                }
                *(ckt->CKTstate0+here->HFETAqgs) = *(ckt->CKTstate1 + here->HFETAqgs) +
                                                   capgs*(vgspp-vgs1);
                *(ckt->CKTstate0+here->HFETAqgd) = *(ckt->CKTstate1 + here->HFETAqgd) +
                                                   capgd*(vgdpp-vgd1);
                *(ckt->CKTstate0+here->HFETAqds) = *(ckt->CKTstate1 + here->HFETAqds) +
                                                   CDS*(vds-vds1);

                /*
                 *   store small-signal parameters 
                 */
                if( (!(ckt->CKTmode & MODETRANOP)) || 
                        (!(ckt->CKTmode & MODEUIC)) ) {
                    if(ckt->CKTmode & MODEINITSMSIG) {
                        *(ckt->CKTstate0 + here->HFETAqgs) = capgs;
                        *(ckt->CKTstate0 + here->HFETAqgd) = capgd;
                        *(ckt->CKTstate0 + here->HFETAqds) = CDS;
                        continue; /*go to 1000*/
                    }
                    /*
                     *   transient analysis 
                     */
                    if(ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->HFETAqgs) =
                                *(ckt->CKTstate0 + here->HFETAqgs);
                        *(ckt->CKTstate1 + here->HFETAqgd) =
                                *(ckt->CKTstate0 + here->HFETAqgd);
                        *(ckt->CKTstate1 + here->HFETAqds) =
                                *(ckt->CKTstate0 + here->HFETAqds);        
                    }
                    error = NIintegrate(ckt,&geq,&ceq,capgs,here->HFETAqgs);
                    if(error) return(error);
                    ggspp = geq;
                    cgspp = *(ckt->CKTstate0 + here->HFETAcqgs);
                    cg = cg + cgspp;
                    error = NIintegrate(ckt,&geq,&ceq,capgd,here->HFETAqgd);
                    if(error) return(error);
                    ggdpp = geq;
                    cgdpp = *(ckt->CKTstate0 + here->HFETAcqgd);
                    cg = cg + cgdpp;
                    cd = cd - cgdpp;
                    error = NIintegrate(ckt,&geq,&ceq,CDS,here->HFETAqds);
                    if(error) return(error);
                    gds += geq;
                    cd  += *(ckt->CKTstate0 + here->HFETAcqds);
                    if (ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->HFETAcqgs) =
                                *(ckt->CKTstate0 + here->HFETAcqgs);
                        *(ckt->CKTstate1 + here->HFETAcqgd) =
                                *(ckt->CKTstate0 + here->HFETAcqgd);
                        *(ckt->CKTstate1 + here->HFETAcqds) =
                                *(ckt->CKTstate0 + here->HFETAcqds);
                    }
                }
            }
            /*
             *  check convergence 
             */
            if( (!(ckt->CKTmode & MODEINITFIX)) | (!(ckt->CKTmode & MODEUIC))) {
                if( (icheck == 1) 
                        || (fabs(cghat-cg) >= ckt->CKTreltol*
                            MAX(fabs(cghat),fabs(cg))+ckt->CKTabstol) ||
                        (fabs(cdhat-cd) > ckt->CKTreltol*
                            MAX(fabs(cdhat),fabs(cd))+ckt->CKTabstol) 

                        ) {
                    ckt->CKTnoncon++;
                     ckt->CKTtroubleElt = (GENinstance *) here;
                }
            }
            *(ckt->CKTstate0 + here->HFETAvgs)   = vgs;
            *(ckt->CKTstate0 + here->HFETAvgd)   = vgd;
            *(ckt->CKTstate0 + here->HFETAvgspp) = vgspp;
            *(ckt->CKTstate0 + here->HFETAvgdpp) = vgdpp;
            *(ckt->CKTstate0 + here->HFETAcg)    = cg;
            *(ckt->CKTstate0 + here->HFETAcd)    = cd;
            *(ckt->CKTstate0 + here->HFETAcgd)   = cgd;
            *(ckt->CKTstate0 + here->HFETAcgs)   = cgs;
            *(ckt->CKTstate0 + here->HFETAcgspp) = cgspp;
            *(ckt->CKTstate0 + here->HFETAcgdpp) = cgdpp;
            *(ckt->CKTstate0 + here->HFETAgm)    = gm;
            *(ckt->CKTstate0 + here->HFETAgds)   = gds;
            *(ckt->CKTstate0 + here->HFETAggs)   = ggs;
            *(ckt->CKTstate0 + here->HFETAggd)   = ggd;
            *(ckt->CKTstate0 + here->HFETAggspp) = ggspp;
            *(ckt->CKTstate0 + here->HFETAggdpp) = ggdpp;
            *(ckt->CKTstate0 + here->HFETAgmg)   = gmg;
            *(ckt->CKTstate0 + here->HFETAgmd)   = gmd;            

            /*
             *    load current vector
             */
load:
            ceqgd = model->HFETAtype*(cgd+cgdpp-ggd*vgd-gmg*vgs-gmd*vds-ggdpp*vgdpp);
            ceqgs = model->HFETAtype*(cgs + cgspp - ggs*vgs - ggspp*vgspp);
            cdreq = model->HFETAtype*(cd + cgd + cgdpp - gds*vds - gm*vgs);
            *(ckt->CKTrhs + here->HFETAgatePrimeNode) += (-ceqgs-ceqgd);
            ceqgd = model->HFETAtype*(cgd-ggd*vgd-gmg*vgs-gmd*vds);
            *(ckt->CKTrhs + here->HFETAdrainPrimeNode) += (-cdreq+ceqgd);
            ceqgd = model->HFETAtype*(cgdpp-ggdpp*vgdpp);
            *(ckt->CKTrhs + here->HFETAdrainPrmPrmNode) += ceqgd;
            ceqgs = model->HFETAtype*(cgs-ggs*vgs);
            *(ckt->CKTrhs + here->HFETAsourcePrimeNode) += (cdreq+ceqgs);
            ceqgs = model->HFETAtype*(cgspp-ggspp*vgspp);
            *(ckt->CKTrhs + here->HFETAsourcePrmPrmNode) += ceqgs;
                    
            /*
             *    load y matrix 
             */

            *(here->HFETAdrainDrainPtr) += model->HFETAdrainConduct;
            *(here->HFETAsourceSourcePtr) += model->HFETAsourceConduct;
            *(here->HFETAgatePrimeGatePrimePtr) += (ggd+ggs+ggspp+ggdpp+gmg+model->HFETAgateConduct);
            *(here->HFETAdrainPrimeDrainPrimePtr) += (gds+ggd-gmd+model->HFETAdrainConduct+model->HFETAgf);
            *(here->HFETAsourcePrimeSourcePrimePtr) += (gds+gm+ggs+model->HFETAsourceConduct+model->HFETAgi);
            *(here->HFETAsourcePrmPrmSourcePrmPrmPtr) += (model->HFETAgi+ggspp);
            *(here->HFETAdrainPrmPrmDrainPrmPrmPtr) += (model->HFETAgf+ggdpp);
            *(here->HFETAdrainDrainPrimePtr) -= model->HFETAdrainConduct;
            *(here->HFETAdrainPrimeDrainPtr) -= model->HFETAdrainConduct;
            *(here->HFETAsourceSourcePrimePtr) -= model->HFETAsourceConduct;
            *(here->HFETAsourcePrimeSourcePtr) -= model->HFETAsourceConduct;
            *(here->HFETAgatePrimeDrainPrimePtr) += -ggd+gmd;
            *(here->HFETAdrainPrimeGatePrimePtr) += (gm-ggd-gmg);
            *(here->HFETAgatePrimeSourcePrimePtr) -= ggs+gmg+gmd;
            *(here->HFETAsourcePrimeGatePrimePtr) += (-ggs-gm);
            *(here->HFETAdrainPrimeSourcePrimePtr) += (-gds-gm+gmg+gmd);
            *(here->HFETAsourcePrimeDrainPrimePtr) -= gds;
            *(here->HFETAsourcePrimeSourcePrmPrmPtr) -= model->HFETAgi;            
            *(here->HFETAsourcePrmPrmSourcePrimePtr) -= model->HFETAgi;
            *(here->HFETAgatePrimeSourcePrmPrmPtr) -= ggspp;
            *(here->HFETAsourcePrmPrmGatePrimePtr) -= ggspp;
            *(here->HFETAdrainPrimeDrainPrmPrmPtr) -= model->HFETAgf;
            *(here->HFETAdrainPrmPrmDrainPrimePtr) -= model->HFETAgf;
            *(here->HFETAgatePrimeDrainPrmPrmPtr) -= ggdpp;
            *(here->HFETAdrainPrmPrmGatePrimePtr) -= ggdpp;
            *(here->HFETAgateGatePtr) += model->HFETAgateConduct;
            *(here->HFETAgateGatePrimePtr) -= model->HFETAgateConduct;
            *(here->HFETAgatePrimeGatePtr) -= model->HFETAgateConduct;
            

        }
    }
    return(OK);
}




static void leak(double gmin, double vt, double v, double rs, double is1, double is2,
                 double m1, double m2, double *il, double *gl)

{

  double vt1 = vt*m1;
  double vt2 = vt*m2;

  if(v > -10*vt1) {
    double dvdi0;
    double iaprox;
    double iaprox1;
    double iaprox2;
    double v0;
    double vteff = vt1 + vt2;
    double iseff = is2*pow((is1/is2),(m1/(m1+m2)));
    if(rs > 0) {
      double unorm = (v + rs*is1)/vt1 + log(rs*is1/vt1);
      iaprox1 = vt1*diode(unorm)/rs - is1;
      unorm = (v + rs*iseff)/vteff + log(rs*iseff/vteff);
      iaprox2 = vteff*diode(unorm)/rs - iseff;
    } else {
      iaprox1 = is1*(exp(v/vt1) - 1);
      iaprox2 = iseff*(exp(v/vteff) - 1);
    }
    if((iaprox1*iaprox2) != 0.0)
      iaprox = 1./(1./iaprox1 + 1./iaprox2);
    else
      iaprox = 0.5*(iaprox1 + iaprox2);
    
    dvdi0 = rs + vt1/(iaprox+is1) + vt2/(iaprox+is2);
    v0    = rs*iaprox;
    v0   += vt1*log(iaprox/is1 + 1) + vt2*log(iaprox/is2 + 1);
    //*il   = __max(-is1,iaprox + (v - v0)/dvdi0)*0.99999;
    *il   = MAX(-is1,iaprox + (v - v0)/dvdi0)*0.99999;
    *gl = 1./(rs + vt1/(*il+is1) + vt2/(*il+is2));
  } else {
    *gl = gmin;
    *il  = (*gl)*v-is1;
  }

}





static void hfeta(HFETAmodel *model, HFETAinstance *here, CKTcircuit *ckt,
                  double vgs, double vds, double *cdrain, double *gm,
                  double *gds, double *capgs, double *capgd,
                  double *cgd, double *gmg, double *gmd,
                  double *cgs, double *ggs)

{
           
  double vt;
  double vgt;
  double vgt0;
  double sigma;
  double vgte;
  double isat;
  double isatm;
  double ns;
  double nsm;
  double a;
  double b;
  double c;
  double d;
  double e;
  double f;
  double g;
  double h;
  double p;
  double q;
  double s;
  double t;
  double u;
  double nsc;
  double nsn;
  double temp;
  double etavth;
  double gch;
  double gchi;
  double gchim;
  double vsate;
  double vdse;
  double cg1;
  double cgc;
  double rt;
  double vl;
  double delidgch;
  double delgchgchi;
  double delgchins;
  double delnsnsm;
  double delnsmvgt;
  double delvgtevgt;
  double delidvsate;
  double delvsateisat;
  double delisatisatm;
  double delisatmvgte;
  double delisatmgchim;
  double delvsategch;
  double delidvds;
  double delvgtvgs;
  double delvsatevgt;

  vt     = CONSTKoverQ*TEMP;
  etavth = ETA*vt;
  vl     = VS/TMU*L;
  rt     = RSI+RDI;
  vgt0   = vgs - TVTO;
  s      = exp((vgt0-VSIGMAT)/VSIGMA);
  sigma  = SIGMA0/(1+s);
  vgt    = vgt0+sigma*vds;
  u      = 0.5*vgt/vt-1;
  t      = sqrt(model->HFETAdeltaSqr+u*u);
  vgte   = vt*(2+u+t);
  b      = exp(vgt/etavth);
  if(model->HFETAeta2Given && model->HFETAd2Given) {
    nsc    = N02*exp((vgt+TVTO-VT2)/(ETA2*vt));
    nsn    = 2*N0*log(1+0.5*b);
    nsm    = nsn*nsc/(nsn+nsc);
  } else {
    nsm = 2*N0*log(1+0.5*b);
  }
  if(nsm < 1.0e-38) {
    *cdrain = 0;
    *gm = 0.0;
    *gds = 0.0;
    *capgs = CF;
    *capgd = CF;
    goto cgd_calc;
  }
  c      = pow(nsm/NMAX,GAMMA);
  q      = pow(1+c,1.0/GAMMA);
  ns     = nsm/q;
  gchi   = GCHI0*ns;
  gch    = gchi/(1+gchi*rt);
  gchim  = GCHI0*nsm;
  h      = sqrt(1+2*gchim*RSI + vgte*vgte/(vl*vl));
  p      = 1+gchim*RSI+h;
  isatm  = gchim*vgte/p;
  g      = pow(isatm/IMAX,GAMMA);
  isat   = isatm/pow(1+g,1/GAMMA);
  vsate  = isat/gch;
  d      = pow(vds/vsate,M);
  e      = pow(1+d,1.0/M);
  delidgch      = vds*(1+TLAMBDA*vds)/e;
  *cdrain       = gch*delidgch;
  delidvsate    = (*cdrain)*d/vsate/(1+d);
  delidvds      = gch*(1+2*TLAMBDA*vds)/e-(*cdrain)*
                  pow(vds/vsate,M-1)/(vsate*(1+d));
  a             = 1+gchi*rt;
  delgchgchi    = 1.0/(a*a);
  delgchins     = GCHI0;
  delnsnsm      = ns/nsm*(1-c/(1+c));
  delvgtevgt    = 0.5*(1+u/t);
  delnsmvgt     = N0/etavth/(1.0/b + 0.5);
  if(model->HFETAeta2Given && model->HFETAd2Given)
    delnsmvgt     = nsc*(nsc*delnsmvgt+nsn*nsn/(ETA2*vt))/((nsc+nsn)*(nsc+nsn));
  delvsateisat  = 1.0/gch;
  delisatisatm  = isat/isatm*(1-g/(1+g));
  delisatmvgte  = gchim*(p - vgte*vgte/(vl*vl*h))/(p*p);
  delvsategch   = -vsate/gch;
  delisatmgchim = vgte*(p - gchim*RSI*(1+1.0/h))/(p*p);
  delvgtvgs     = 1-vds*SIGMA0/VSIGMA*s/((1+s)*(1+s));
  p             = delgchgchi*delgchins*delnsnsm*delnsmvgt;
  delvsatevgt   = (delvsateisat*delisatisatm*(delisatmvgte*delvgtevgt +
                  delisatmgchim*GCHI0*delnsmvgt)+delvsategch*p);
  g             = delidgch*p + delidvsate*delvsatevgt;
  *gm           = g*delvgtvgs;
  *gds          = delidvds + g*sigma;

  // Capacitance calculations
  temp          = ETA1*vt;
  cg1           = 1/(D1/EPSI+temp*exp(-(vgs-IN_VT1)/temp));
  cgc           = W*L*(CHARGE*delnsnsm*delnsmvgt*delvgtvgs+cg1);
  vdse          = vds*pow(1+pow(vds/vsate,MC),-1.0/MC);
  a             = (vsate-vdse)/(2*vsate-vdse);
  a             = a*a;
  temp          = 2.0/3.0;
  p             = PM + (1-PM)*exp(-vds/vsate);
  *capgs        = CF+2*temp*cgc*(1-a)/(1+p);
  a             = vsate/(2*vsate-vdse);
  a             = a*a;
  *capgd        = CF+2*p*temp*cgc*(1-a)/(1+p);
/*
  {
  char buf[128];
  FILE *fp;
  fp = fopen("d:\\temp\\debug.txt","at");
  sprintf(buf,"%f\t%f\t%e\t%e\n",vgs,vds,W*L*CHARGE*delnsnsm*delnsmvgt*delvgtvgs,cgc);
  fputs(buf,fp);
  fclose(fp);
  }
*/  
cgd_calc:

  if(model->HFETAgatemod != 0) {
    // Gate-drain current calculation
    double vkneet;
    double vmax;
    double td;
    double delcgdvgs;
    double delcgdtd;
    double deltdvdse;
    double deltdvkneet;
    double delvdsevmax;
    double delvdsevds;
    double dvdsevgs;
    double dvdsevds;
    double dtdvgs;
    double dtdvds;
                 
    vkneet = CK1*vsate+CK2;
    vmax   = CM1*vsate+CM2;
    a      = pow(vds/vmax,MT2);
    b      = pow(1+a,1/MT2);
    vdse   = vds/b;
    c      = pow(vdse/vkneet,MT1);
    d      = pow(1+c,1/MT1);
    td     = TEMP+TALPHA*vdse*vdse/d;
    e      = CONSTKoverQ*td*M2D;
    p      = PHIB/(CONSTboltz*td);
    f      = exp(-p);
    q      = (vgs-vdse)/e;
    g      = exp(q);
    h      = ISO*td*td*f*g;
    *cgd   = h - ISO*TEMP*TEMP*exp(-PHIB/(CONSTboltz*TEMP));
    delcgdvgs   = h/e;
    delcgdtd    = h*(p-q+2)/td;
    deltdvdse   = TALPHA*vdse*(2-c/(1+c))/d;
    deltdvkneet = (td-TEMP)*c/((1+c)*vkneet);
    delvdsevmax = vdse*a/((1+a)*vmax);
    delvdsevds  = (1-a/(1+a))/b;
    temp        = delvsatevgt*delvgtvgs;
    dvdsevgs    = delvdsevmax*CM1*temp;
    dtdvgs      = deltdvdse*dvdsevgs+deltdvkneet*CK1*temp;
    *gmg        = delcgdvgs+delcgdtd*dtdvgs;
    temp        = delvsatevgt*sigma;
    dvdsevds    = delvdsevds+delvdsevmax*CM1*temp;
    dtdvds      = deltdvdse*dvdsevds+deltdvkneet*CK1*temp;
    *gmd        = -delcgdvgs*dvdsevds+delcgdtd*dtdvds;
  } else {
    gmg = 0;
    gmd = 0;
  }  

  if(model->HFETAgatemod != 0) {  
    // Gate-source current calculation
    double evgs;
    double vtn = vt*M2S;
    double csat = ISO*TEMP*TEMP*exp(-PHIB/(CONSTboltz*TEMP));
    if (vgs <= -5*vt) {
      *ggs = -csat/vgs+ckt->CKTgmin;
      *cgs = (*ggs)*vgs;
    } else {
      evgs = exp(vgs/vtn);
      *ggs = csat*evgs/vtn+ckt->CKTgmin;
      *cgs = csat*(evgs-1)+ckt->CKTgmin*vgs;
    }
  }

  if(model->HFETAgatemod != 0 && (A1 != 0.0 || A2 != 0.0)) {
    // Correction current calculations  
    double vmax;
    double delvdsevmax;
    double delvdsevds;
    double dvdsevgs;
    double dvdsevds;
    vmax        = CM3*vsate;
    a           = pow(vds/vmax,MV1);
    b           = pow(1+a,1/MV1);
    vdse        = vds/b;
    delvdsevmax = vdse*a/((1+a)*vmax);
    delvdsevds  = (1-a/(1+a))/b;
    dvdsevgs    = delvdsevmax*CM3*delvsatevgt*delvgtvgs;
    dvdsevds    = delvdsevds+delvdsevmax*CM3*delvsatevgt*sigma;
    c           = vgte*vdse;
    d           = 1+A2*c;
    e           = vdse*delvgtevgt;
    f           = A2*(*cgd);
    *cdrain    += A1*(d*(*cgd) - (*cgs));
    *gds       += A1*(d*(*gmd)+f*(vgte*dvdsevds+e*sigma));
    *gm        += A1*(d*(*gmg)+f*(vgte*dvdsevgs+e*delvgtvgs) - (*ggs));
  }

}


double diode(double u)
{

#define U0 (-2.303)
#define A (2.221)
#define B (6.804)
#define C (1.685)
  	double it;
  	double ut;
  	double b;
  	double c;
  	double i;
  	double expu=exp(u);
  	
  	if(u <= U0)
    {
    	it = expu*(1-expu);
  	}else                                             
    {
    	b = 0.5*(u-U0);
    	it = u + A*exp((U0-u)/B) - log(b+sqrt(b*b + 0.25*C*C));
    }
  	
  	ut = it + log(it);
  	b = u-ut;
  	c = 1+it;
  	i = it*(1 + b/c + 0.5*b*b/c/c/c);
  	return(i);
}
