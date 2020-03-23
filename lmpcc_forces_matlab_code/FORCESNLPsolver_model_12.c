/* This function was automatically generated by CasADi */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CODEGEN_PREFIX
#define NAMESPACE_CONCAT(NS, ID) _NAMESPACE_CONCAT(NS, ID)
#define _NAMESPACE_CONCAT(NS, ID) NS ## ID
#define CASADI_PREFIX(ID) NAMESPACE_CONCAT(CODEGEN_PREFIX, ID)
#else /* CODEGEN_PREFIX */
#define CASADI_PREFIX(ID) FORCESNLPsolver_model_12_ ## ID
#endif /* CODEGEN_PREFIX */

#include <math.h>

#include "FORCESNLPsolver/include/FORCESNLPsolver.h"

#define PRINTF printf
FORCESNLPsolver_float CASADI_PREFIX(sq)(FORCESNLPsolver_float x) { return x*x;}
#define sq(x) CASADI_PREFIX(sq)(x)

FORCESNLPsolver_float CASADI_PREFIX(sign)(FORCESNLPsolver_float x) { return x<0 ? -1 : x>0 ? 1 : x;}
#define sign(x) CASADI_PREFIX(sign)(x)

static const solver_int32_default CASADI_PREFIX(s0)[] = {6, 1, 0, 6, 0, 1, 2, 3, 4, 5};
#define s0 CASADI_PREFIX(s0)
static const solver_int32_default CASADI_PREFIX(s1)[] = {30, 1, 0, 30, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
#define s1 CASADI_PREFIX(s1)
static const solver_int32_default CASADI_PREFIX(s2)[] = {1, 1, 0, 1, 0};
#define s2 CASADI_PREFIX(s2)
static const solver_int32_default CASADI_PREFIX(s3)[] = {1, 6, 0, 1, 2, 3, 4, 5, 5, 0, 0, 0, 0, 0};
#define s3 CASADI_PREFIX(s3)
/* evaluate_stages */
solver_int32_default FORCESNLPsolver_model_12(const FORCESNLPsolver_float **arg, FORCESNLPsolver_float **res) 
{
    FORCESNLPsolver_float a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16;
    a0=arg[0] ? arg[0][3] : 0;
    a1=arg[1] ? arg[1][4] : 0;
    a0=(a0-a1);
    a1=arg[1] ? arg[1][0] : 0;
    a2=(a1*a0);
    a3=(a2*a0);
    a4=arg[0] ? arg[0][4] : 0;
    a5=arg[1] ? arg[1][5] : 0;
    a4=(a4-a5);
    a5=arg[1] ? arg[1][1] : 0;
    a6=(a5*a4);
    a7=(a6*a4);
    a3=(a3+a7);
    a7=arg[1] ? arg[1][3] : 0;
    a8=arg[0] ? arg[0][1] : 0;
    a9=(a7*a8);
    a10=(a9*a8);
    a3=(a3+a10);
    a10=10.;
    a11=arg[0] ? arg[0][2] : 0;
    a12=(a10*a11);
    a13=(a12*a11);
    a3=(a3+a13);
    a13=arg[1] ? arg[1][2] : 0;
    a14=arg[0] ? arg[0][0] : 0;
    a15=(a13*a14);
    a16=(a15*a14);
    a3=(a3+a16);
    if (res[0]!=0) res[0][0]=a3;
    a13=(a13*a14);
    a15=(a15+a13);
    if (res[1]!=0) res[1][0]=a15;
    a7=(a7*a8);
    a9=(a9+a7);
    if (res[1]!=0) res[1][1]=a9;
    a10=(a10*a11);
    a12=(a12+a10);
    if (res[1]!=0) res[1][2]=a12;
    a1=(a1*a0);
    a2=(a2+a1);
    if (res[1]!=0) res[1][3]=a2;
    a5=(a5*a4);
    a6=(a6+a5);
    if (res[1]!=0) res[1][4]=a6;
    return 0;
}

solver_int32_default FORCESNLPsolver_model_12_init(solver_int32_default *f_type, solver_int32_default *n_in, solver_int32_default *n_out, solver_int32_default *sz_arg, solver_int32_default *sz_res) 
{
    *f_type = 1;
    *n_in = 2;
    *n_out = 2;
    *sz_arg = 2;
    *sz_res = 2;
    return 0;
}

solver_int32_default FORCESNLPsolver_model_12_sparsity(solver_int32_default i, solver_int32_default *nrow, solver_int32_default *ncol, const solver_int32_default **colind, const solver_int32_default **row) 
{
    const solver_int32_default *s;
    switch (i) 
    {
      case 0:
        s = s0;
        break;
      case 1:
        s = s1;
        break;
      case 2:
        s = s2;
        break;
      case 3:
        s = s3;
        break;
      default:
        return 1;
    }
    
    *nrow = s[0];
    *ncol = s[1];
    *colind = s + 2;
    *row = s + 2 + (*ncol + 1);
    return 0;
}

solver_int32_default FORCESNLPsolver_model_12_work(solver_int32_default *sz_iw, solver_int32_default *sz_w) 
{
    if (sz_iw) *sz_iw = 0;
    if (sz_w) *sz_w = 17;
    return 0;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
