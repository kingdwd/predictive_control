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

static const solver_int32_default CASADI_PREFIX(s0)[] = {7, 1, 0, 7, 0, 1, 2, 3, 4, 5, 6};
#define s0 CASADI_PREFIX(s0)
static const solver_int32_default CASADI_PREFIX(s1)[] = {40, 1, 0, 40, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
#define s1 CASADI_PREFIX(s1)
static const solver_int32_default CASADI_PREFIX(s2)[] = {1, 1, 0, 1, 0};
#define s2 CASADI_PREFIX(s2)
static const solver_int32_default CASADI_PREFIX(s3)[] = {1, 7, 0, 1, 2, 3, 4, 5, 5, 6, 0, 0, 0, 0, 0, 0};
#define s3 CASADI_PREFIX(s3)
/* evaluate_stages */
solver_int32_default FORCESNLPsolver_model_12(const FORCESNLPsolver_float **arg, FORCESNLPsolver_float **res) 
{
    FORCESNLPsolver_float a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32,a33,a34,a35,a36,a37,a38,a39,a40,a41,a42,a43,a44,a45,a46,a47,a48,a49,a50,a51,a52,a53,a54,a55,a56,a57,a58,a59,a60,a61,a62,a63,a64,a65,a66,a67,a68,a69,a70,a71,a72,a73,a74,a75,a76,a77,a78,a79,a80,a81,a82,a83,a84,a85,a86,a87,a88,a89,a90;
    a0=3.;
    a1=arg[1] ? arg[1][4] : 0;
    a2=(a0*a1);
    a3=arg[0] ? arg[0][6] : 0;
    a4=arg[1] ? arg[1][16] : 0;
    a5=(a3-a4);
    a5=(a2*a5);
    a6=(a3-a4);
    a7=(a5*a6);
    a8=2.;
    a9=arg[1] ? arg[1][5] : 0;
    a10=(a8*a9);
    a11=(a3-a4);
    a11=(a10*a11);
    a7=(a7+a11);
    a11=arg[1] ? arg[1][6] : 0;
    a7=(a7+a11);
    a12=arg[1] ? arg[1][18] : 0;
    a12=(a3-a12);
    a13=1.0000000000000001e-01;
    a12=(a12/a13);
    a12=exp(a12);
    a13=1.;
    a14=(a13+a12);
    a7=(a7/a14);
    a15=(1./a14);
    a16=(a13-a15);
    a17=arg[1] ? arg[1][12] : 0;
    a18=(a0*a17);
    a19=arg[1] ? arg[1][17] : 0;
    a20=(a3-a19);
    a20=(a18*a20);
    a21=(a3-a19);
    a22=(a20*a21);
    a23=arg[1] ? arg[1][13] : 0;
    a24=(a8*a23);
    a25=(a3-a19);
    a25=(a24*a25);
    a22=(a22+a25);
    a25=arg[1] ? arg[1][14] : 0;
    a22=(a22+a25);
    a26=(a16*a22);
    a26=(a7+a26);
    a27=arg[1] ? arg[1][0] : 0;
    a28=(a0*a27);
    a29=(a3-a4);
    a29=(a28*a29);
    a30=(a3-a4);
    a31=(a29*a30);
    a32=arg[1] ? arg[1][1] : 0;
    a33=(a8*a32);
    a34=(a3-a4);
    a34=(a33*a34);
    a31=(a31+a34);
    a34=arg[1] ? arg[1][2] : 0;
    a31=(a31+a34);
    a31=(a31/a14);
    a35=(a13-a15);
    a36=arg[1] ? arg[1][8] : 0;
    a0=(a0*a36);
    a37=(a3-a19);
    a37=(a0*a37);
    a38=(a3-a19);
    a39=(a37*a38);
    a40=arg[1] ? arg[1][9] : 0;
    a8=(a8*a40);
    a41=(a3-a19);
    a41=(a8*a41);
    a39=(a39+a41);
    a41=arg[1] ? arg[1][10] : 0;
    a39=(a39+a41);
    a42=(a35*a39);
    a42=(a31+a42);
    a43=sq(a42);
    a44=sq(a26);
    a43=(a43+a44);
    a43=sqrt(a43);
    a44=(a26/a43);
    a45=(a3-a4);
    a45=(a27*a45);
    a46=(a3-a4);
    a47=(a45*a46);
    a48=(a3-a4);
    a49=(a47*a48);
    a50=(a3-a4);
    a50=(a32*a50);
    a51=(a3-a4);
    a52=(a50*a51);
    a49=(a49+a52);
    a52=(a3-a4);
    a52=(a34*a52);
    a49=(a49+a52);
    a52=arg[1] ? arg[1][3] : 0;
    a49=(a49+a52);
    a49=(a49/a14);
    a52=(a13-a15);
    a53=(a3-a19);
    a53=(a36*a53);
    a54=(a3-a19);
    a55=(a53*a54);
    a56=(a3-a19);
    a57=(a55*a56);
    a58=(a3-a19);
    a58=(a40*a58);
    a59=(a3-a19);
    a60=(a58*a59);
    a57=(a57+a60);
    a60=(a3-a19);
    a60=(a41*a60);
    a57=(a57+a60);
    a60=arg[1] ? arg[1][11] : 0;
    a57=(a57+a60);
    a60=(a52*a57);
    a60=(a49+a60);
    a61=arg[0] ? arg[0][3] : 0;
    a62=(a61-a60);
    a63=(a44*a62);
    a64=(a42/a43);
    a65=(a3-a4);
    a65=(a1*a65);
    a66=(a3-a4);
    a67=(a65*a66);
    a68=(a3-a4);
    a69=(a67*a68);
    a70=(a3-a4);
    a70=(a9*a70);
    a71=(a3-a4);
    a72=(a70*a71);
    a69=(a69+a72);
    a4=(a3-a4);
    a4=(a11*a4);
    a69=(a69+a4);
    a4=arg[1] ? arg[1][7] : 0;
    a69=(a69+a4);
    a69=(a69/a14);
    a13=(a13-a15);
    a4=(a3-a19);
    a4=(a17*a4);
    a72=(a3-a19);
    a73=(a4*a72);
    a74=(a3-a19);
    a75=(a73*a74);
    a76=(a3-a19);
    a76=(a23*a76);
    a77=(a3-a19);
    a78=(a76*a77);
    a75=(a75+a78);
    a3=(a3-a19);
    a3=(a25*a3);
    a75=(a75+a3);
    a3=arg[1] ? arg[1][15] : 0;
    a75=(a75+a3);
    a3=(a13*a75);
    a3=(a69+a3);
    a19=arg[0] ? arg[0][4] : 0;
    a78=(a19-a3);
    a79=(a64*a78);
    a63=(a63-a79);
    a79=arg[1] ? arg[1][19] : 0;
    a80=(a79*a63);
    a81=(a80*a63);
    a61=(a61-a60);
    a60=(a64*a61);
    a19=(a19-a3);
    a3=(a44*a19);
    a60=(a60+a3);
    a3=arg[1] ? arg[1][20] : 0;
    a82=(a3*a60);
    a83=(a82*a60);
    a81=(a81+a83);
    a83=arg[0] ? arg[0][0] : 0;
    a84=arg[1] ? arg[1][22] : 0;
    a85=(a83-a84);
    a86=arg[1] ? arg[1][25] : 0;
    a85=(a86*a85);
    a83=(a83-a84);
    a84=(a85*a83);
    a81=(a81+a84);
    a84=arg[1] ? arg[1][21] : 0;
    a87=arg[0] ? arg[0][1] : 0;
    a88=(a84*a87);
    a89=(a88*a87);
    a81=(a81+a89);
    a89=arg[0] ? arg[0][2] : 0;
    a90=sq(a89);
    a81=(a81+a90);
    if (res[0]!=0) res[0][0]=a81;
    a86=(a86*a83);
    a85=(a85+a86);
    if (res[1]!=0) res[1][0]=a85;
    a84=(a84*a87);
    a88=(a88+a84);
    if (res[1]!=0) res[1][1]=a88;
    a89=(a89+a89);
    if (res[1]!=0) res[1][2]=a89;
    a3=(a3*a60);
    a82=(a82+a3);
    a3=(a64*a82);
    a79=(a79*a63);
    a80=(a80+a79);
    a79=(a44*a80);
    a63=(a3+a79);
    if (res[1]!=0) res[1][3]=a63;
    a63=(a44*a82);
    a60=(a64*a80);
    a89=(a63-a60);
    if (res[1]!=0) res[1][4]=a89;
    a60=(a60-a63);
    a13=(a13*a60);
    a25=(a25*a13);
    a76=(a76*a13);
    a25=(a25+a76);
    a77=(a77*a13);
    a23=(a23*a77);
    a25=(a25+a23);
    a73=(a73*a13);
    a25=(a25+a73);
    a74=(a74*a13);
    a4=(a4*a74);
    a25=(a25+a4);
    a72=(a72*a74);
    a17=(a17*a72);
    a25=(a25+a17);
    a17=(a60/a14);
    a11=(a11*a17);
    a25=(a25+a11);
    a70=(a70*a17);
    a25=(a25+a70);
    a71=(a71*a17);
    a9=(a9*a71);
    a25=(a25+a9);
    a67=(a67*a17);
    a25=(a25+a67);
    a68=(a68*a17);
    a65=(a65*a68);
    a25=(a25+a65);
    a66=(a66*a68);
    a1=(a1*a66);
    a25=(a25+a1);
    a3=(a3+a79);
    a52=(a52*a3);
    a41=(a41*a52);
    a25=(a25-a41);
    a58=(a58*a52);
    a25=(a25-a58);
    a59=(a59*a52);
    a40=(a40*a59);
    a25=(a25-a40);
    a55=(a55*a52);
    a25=(a25-a55);
    a56=(a56*a52);
    a53=(a53*a56);
    a25=(a25-a53);
    a54=(a54*a56);
    a36=(a36*a54);
    a25=(a25-a36);
    a36=(a3/a14);
    a34=(a34*a36);
    a25=(a25-a34);
    a50=(a50*a36);
    a25=(a25-a50);
    a51=(a51*a36);
    a32=(a32*a51);
    a25=(a25-a32);
    a47=(a47*a36);
    a25=(a25-a47);
    a48=(a48*a36);
    a45=(a45*a48);
    a25=(a25-a45);
    a46=(a46*a48);
    a27=(a27*a46);
    a25=(a25-a27);
    a61=(a61*a82);
    a78=(a78*a80);
    a61=(a61-a78);
    a78=(a61/a43);
    a42=(a42+a42);
    a64=(a64/a43);
    a64=(a64*a61);
    a44=(a44/a43);
    a19=(a19*a82);
    a62=(a62*a80);
    a19=(a19+a62);
    a44=(a44*a19);
    a64=(a64+a44);
    a44=(a43+a43);
    a64=(a64/a44);
    a42=(a42*a64);
    a78=(a78-a42);
    a35=(a35*a78);
    a8=(a8*a35);
    a25=(a25+a8);
    a37=(a37*a35);
    a25=(a25+a37);
    a38=(a38*a35);
    a0=(a0*a38);
    a25=(a25+a0);
    a0=(a78/a14);
    a33=(a33*a0);
    a25=(a25+a33);
    a29=(a29*a0);
    a25=(a25+a29);
    a30=(a30*a0);
    a28=(a28*a30);
    a25=(a25+a28);
    a19=(a19/a43);
    a26=(a26+a26);
    a26=(a26*a64);
    a19=(a19-a26);
    a16=(a16*a19);
    a24=(a24*a16);
    a25=(a25+a24);
    a20=(a20*a16);
    a25=(a25+a20);
    a21=(a21*a16);
    a18=(a18*a21);
    a25=(a25+a18);
    a49=(a49/a14);
    a49=(a49*a3);
    a69=(a69/a14);
    a69=(a69*a60);
    a49=(a49-a69);
    a31=(a31/a14);
    a31=(a31*a78);
    a49=(a49-a31);
    a15=(a15/a14);
    a57=(a57*a3);
    a75=(a75*a60);
    a57=(a57-a75);
    a39=(a39*a78);
    a57=(a57-a39);
    a22=(a22*a19);
    a57=(a57-a22);
    a15=(a15*a57);
    a49=(a49-a15);
    a7=(a7/a14);
    a7=(a7*a19);
    a49=(a49-a7);
    a12=(a12*a49);
    a49=10.;
    a49=(a49*a12);
    a25=(a25+a49);
    a19=(a19/a14);
    a10=(a10*a19);
    a25=(a25+a10);
    a5=(a5*a19);
    a25=(a25+a5);
    a6=(a6*a19);
    a2=(a2*a6);
    a25=(a25+a2);
    if (res[1]!=0) res[1][5]=a25;
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
    if (sz_w) *sz_w = 91;
    return 0;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
