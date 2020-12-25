/******************************************************************************
 *
 * Copyright 2010, Dream Chip Technologies GmbH. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Dream Chip Technologies GmbH, Steinriede 10, 30827 Garbsen / Berenbostel,
 * Germany
 *
 *****************************************************************************/
/**
 * @file isi_iss.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup ov5630_a   Illumination Profile A
 * @{
 *
 */
#ifndef __OV2715_A_H__
#define __OV2715_A_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define AWB_COLORMATRIX_ARRAY_SIZE_CIE_A    2
#define AWB_LSCMATRIX_ARRAY_SIZE_CIE_A      1

#define AWB_SATURATION_ARRAY_SIZE_CIE_A     4
#define AWB_VIGNETTING_ARRAY_SIZE_CIE_A     2

#define CC_OFFSET_SCALING_A                 4.0f

/*****************************************************************************/
/**
 * CIE PROFILE A:
 *
 *  incandescent/tungsten, 2856K
 */
/*****************************************************************************/
// crosstalk matrix
const Isi3x3FloatMatrix_t  OV2715_XTalkCoeff_CIE_A =
{
    {
          1.50000f,   0.34771f,  -0.84772f, 
         -0.44487f,   1.82856f,  -0.38369f, 
         -0.93434f,  -1.63154f,   3.56588f  
    }
};

// crosstalk offset matrix
const IsiXTalkFloatOffset_t OV2715_XTalkOffset_CIE_A =
{
    .fCtOffsetRed   = (-129.5f      / CC_OFFSET_SCALING_A),
    .fCtOffsetGreen = (-119.5f      / CC_OFFSET_SCALING_A),
    .fCtOffsetBlue  = (-124.4375f   / CC_OFFSET_SCALING_A)
};

// gain matrix
const IsiComponentGain_t OV2715_CompGain_CIE_A =
{
    .fRed      = 1.00000f,
    .fGreenR   = 1.33216f,
    .fGreenB   = 1.33216f,
    .fBlue     = 2.97503f
};

// mean value of gaussian mixture model
const Isi2x1FloatMatrix_t OV2715_GaussMeanValue_CIE_A =
{
    {
        -0.17832f,  -0.05394f
    }
};

// inverse covariance matrix
const Isi2x2FloatMatrix_t OV2715_CovarianceMatrix_CIE_A =
{
    {
        3822.65008f,  -3102.33290f, 
       -3102.33290f,  3994.42863f 
    }
};

// factor in gaussian mixture model
const IsiGaussFactor_t OV2715_GaussFactor_CIE_A =
 {
    .fGaussFactor = 378.13397f
 };

// thresholds for switching between MAP classification and interpolation
const Isi2x1FloatMatrix_t OV2715_Threshold_CIE_A =
{
    {
        1.00000f,  1.00000f
    }
};

// saturation curve for A profile
float afSaturationSensorGain_CIE_A[AWB_SATURATION_ARRAY_SIZE_CIE_A] =
{
    1.0f, 2.0f, 4.0f, 8.0f
};

float afSaturation_CIE_A[AWB_SATURATION_ARRAY_SIZE_CIE_A] =
{
    100.0f, 100.0f, 90.0f, 74.0f
};

const IsiSaturationCurve_t OV2715_SaturationCurve_CIE_A =
{
    .ArraySize      = AWB_SATURATION_ARRAY_SIZE_CIE_A,
    .pSensorGain    = &afSaturationSensorGain_CIE_A[0],
    .pSaturation    = &afSaturation_CIE_A[0]
};

// saturation depended color conversion matrices
IsiSatCcMatrix_t OV2715_SatCcMatrix_CIE_A[AWB_COLORMATRIX_ARRAY_SIZE_CIE_A] =
{
    {
        .fSaturation    = 74.0f,
        .XTalkCoeff     =
        {
            {
                 1.36408f,   0.25306f,  -0.61714f, 
                -0.03062f,   1.29231f,  -0.26170f, 
                -0.36604f,  -1.20086f,   2.56691f  
            }
        }
    },
    {
        .fSaturation    = 100.0f,
        .XTalkCoeff     =
        {
            {
                 1.50000f,   0.34771f,  -0.84772f, 
                -0.44487f,   1.82856f,  -0.38369f, 
                -0.93434f,  -1.63154f,   3.56588f  
            }
        }
    }
};

const IsiCcMatrixTable_t OV2715_CcMatrixTable_CIE_A =
{
    .ArraySize          = AWB_COLORMATRIX_ARRAY_SIZE_CIE_A,
    .pIsiSatCcMatrix    = &OV2715_SatCcMatrix_CIE_A[0]
};

// saturation depended color conversion offset vectors
IsiSatCcOffset_t OV2715_SatCcOffset_CIE_A[AWB_COLORMATRIX_ARRAY_SIZE_CIE_A] =
{
    {
        .fSaturation    = 74.0f,
        .CcOffset       =
        {
            .fCtOffsetRed   = 0.0f,
            .fCtOffsetGreen = 0.0f,
            .fCtOffsetBlue  = 0.0f
        }
    },
    {
        .fSaturation    = 100.0f,
        .CcOffset       =
        {
            .fCtOffsetRed   = (-129.5f      / CC_OFFSET_SCALING_A),
            .fCtOffsetGreen = (-119.5f      / CC_OFFSET_SCALING_A),
            .fCtOffsetBlue  = (-124.4375f   / CC_OFFSET_SCALING_A)
        }
    }
};

const IsiCcOffsetTable_t OV2715_CcOffsetTable_CIE_A =
{
    .ArraySize          = AWB_COLORMATRIX_ARRAY_SIZE_CIE_A,
    .pIsiSatCcOffset    = &OV2715_SatCcOffset_CIE_A[0]
};

// vignetting curve
float afVignettingSensorGain_CIE_A[AWB_VIGNETTING_ARRAY_SIZE_CIE_A] =
{
    1.0f, 8.0f
};

float afVignetting_CIE_A[AWB_VIGNETTING_ARRAY_SIZE_CIE_A] =
{
    100.0f, 100.0f
};

const IsiVignettingCurve_t OV2715_VignettingCurve_CIE_A =
{
    .ArraySize      = AWB_VIGNETTING_ARRAY_SIZE_CIE_A,
    .pSensorGain    = &afVignettingSensorGain_CIE_A[0],
    .pVignetting    = &afVignetting_CIE_A[0]
};



// vignetting dependend lsc matrices ( 1080pXX  1920x1080 )
IsiVignLscMatrix_t OV2715_VignLscMatrix_CIE_A_1920x1080[AWB_LSCMATRIX_ARRAY_SIZE_CIE_A] = 
{
    // array item 0
    {
       .fVignetting    = 100.0f,
       .LscMatrix      =
       {
           // ISI_COLOR_COMPONENT_RED
           {
               {
                    1190U, 1160U, 1138U, 1120U, 1097U, 1085U, 1077U, 1068U, 1060U, 1055U, 1053U, 1054U, 1061U, 1066U, 1079U, 1086U, 1104U,
                    1187U, 1156U, 1137U, 1114U, 1094U, 1083U, 1075U, 1065U, 1059U, 1052U, 1051U, 1053U, 1053U, 1062U, 1060U, 1097U, 1091U,
                    1177U, 1153U, 1130U, 1112U, 1093U, 1082U, 1074U, 1065U, 1057U, 1051U, 1049U, 1049U, 1048U, 1053U, 1071U, 1073U, 1088U,
                    1174U, 1146U, 1125U, 1107U, 1090U, 1077U, 1070U, 1062U, 1053U, 1047U, 1043U, 1043U, 1044U, 1049U, 1054U, 1073U, 1073U,
                    1166U, 1146U, 1125U, 1105U, 1088U, 1077U, 1070U, 1056U, 1050U, 1045U, 1042U, 1041U, 1041U, 1042U, 1050U, 1061U, 1071U,
                    1171U, 1145U, 1123U, 1104U, 1089U, 1075U, 1066U, 1055U, 1045U, 1041U, 1039U, 1042U, 1040U, 1039U, 1046U, 1060U, 1064U,
                    1170U, 1143U, 1122U, 1103U, 1087U, 1075U, 1065U, 1048U, 1040U, 1036U, 1038U, 1036U, 1035U, 1041U, 1041U, 1054U, 1063U,
                    1167U, 1144U, 1122U, 1101U, 1086U, 1074U, 1059U, 1045U, 1027U, 1031U, 1033U, 1035U, 1034U, 1036U, 1040U, 1050U, 1060U,
                    1170U, 1144U, 1121U, 1102U, 1086U, 1074U, 1059U, 1042U, 1026U, 1029U, 1030U, 1034U, 1032U, 1034U, 1039U, 1050U, 1053U,
                    1171U, 1143U, 1120U, 1104U, 1086U, 1072U, 1056U, 1042U, 1028U, 1027U, 1028U, 1032U, 1031U, 1034U, 1034U, 1047U, 1047U,
                    1168U, 1146U, 1120U, 1103U, 1084U, 1072U, 1057U, 1043U, 1034U, 1030U, 1030U, 1030U, 1031U, 1030U, 1034U, 1045U, 1054U,
                    1173U, 1147U, 1122U, 1103U, 1086U, 1072U, 1057U, 1045U, 1034U, 1029U, 1031U, 1030U, 1029U, 1029U, 1034U, 1046U, 1057U,
                    1174U, 1148U, 1124U, 1103U, 1088U, 1068U, 1056U, 1044U, 1038U, 1031U, 1028U, 1029U, 1027U, 1030U, 1033U, 1046U, 1056U,
                    1180U, 1156U, 1125U, 1107U, 1086U, 1070U, 1058U, 1047U, 1038U, 1031U, 1029U, 1027U, 1029U, 1029U, 1035U, 1049U, 1054U,
                    1196U, 1140U, 1140U, 1102U, 1087U, 1072U, 1060U, 1046U, 1037U, 1031U, 1027U, 1027U, 1025U, 1029U, 1036U, 1051U, 1056U,
                    1184U, 1173U, 1122U, 1113U, 1088U, 1072U, 1057U, 1045U, 1036U, 1027U, 1024U, 1025U, 1025U, 1032U, 1040U, 1051U, 1061U,
                    1195U, 1154U, 1143U, 1113U, 1095U, 1071U, 1057U, 1045U, 1037U, 1031U, 1025U, 1025U, 1030U, 1038U, 1042U, 1057U, 1071U
               },
           }, 
    
           // ISI_COLOR_COMPONENT_GREENR
           {
               {
                    1130U, 1104U, 1096U, 1082U, 1066U, 1062U, 1056U, 1055U, 1047U, 1036U, 1034U, 1028U, 1029U, 1033U, 1045U, 1051U, 1068U, 
                    1137U, 1111U, 1098U, 1084U, 1072U, 1069U, 1068U, 1063U, 1056U, 1049U, 1040U, 1036U, 1035U, 1041U, 1036U, 1071U, 1068U,
                    1134U, 1108U, 1096U, 1083U, 1072U, 1070U, 1069U, 1065U, 1061U, 1051U, 1042U, 1036U, 1034U, 1033U, 1049U, 1048U, 1063U,
                    1131U, 1107U, 1092U, 1080U, 1072U, 1068U, 1067U, 1065U, 1061U, 1052U, 1044U, 1036U, 1033U, 1034U, 1036U, 1050U, 1049U,
                    1135U, 1109U, 1093U, 1082U, 1070U, 1069U, 1070U, 1065U, 1063U, 1056U, 1047U, 1039U, 1034U, 1030U, 1033U, 1039U, 1046U,
                    1135U, 1112U, 1094U, 1083U, 1075U, 1070U, 1068U, 1066U, 1060U, 1055U, 1049U, 1045U, 1034U, 1029U, 1029U, 1037U, 1041U,
                    1136U, 1112U, 1095U, 1081U, 1075U, 1071U, 1065U, 1060U, 1058U, 1055U, 1049U, 1042U, 1032U, 1029U, 1027U, 1034U, 1038U,
                    1139U, 1115U, 1096U, 1087U, 1077U, 1072U, 1065U, 1059U, 1050U, 1053U, 1047U, 1043U, 1032U, 1027U, 1027U, 1035U, 1040U,
                    1141U, 1121U, 1102U, 1085U, 1077U, 1073U, 1067U, 1058U, 1050U, 1047U, 1046U, 1041U, 1035U, 1028U, 1029U, 1034U, 1033U,
                    1148U, 1122U, 1102U, 1090U, 1080U, 1076U, 1064U, 1058U, 1048U, 1046U, 1043U, 1045U, 1039U, 1033U, 1024U, 1034U, 1029U,
                    1151U, 1129U, 1106U, 1094U, 1083U, 1075U, 1065U, 1056U, 1049U, 1047U, 1047U, 1046U, 1040U, 1029U, 1026U, 1031U, 1034U,
                    1155U, 1133U, 1113U, 1100U, 1083U, 1071U, 1063U, 1051U, 1049U, 1049U, 1048U, 1042U, 1040U, 1033U, 1026U, 1035U, 1035U,
                    1163U, 1138U, 1117U, 1097U, 1082U, 1070U, 1058U, 1053U, 1051U, 1047U, 1042U, 1041U, 1037U, 1032U, 1030U, 1036U, 1035U,
                    1171U, 1146U, 1118U, 1101U, 1081U, 1067U, 1062U, 1056U, 1048U, 1043U, 1039U, 1039U, 1038U, 1037U, 1033U, 1039U, 1039U,
                    1182U, 1130U, 1132U, 1095U, 1084U, 1074U, 1063U, 1055U, 1046U, 1038U, 1038U, 1039U, 1040U, 1037U, 1041U, 1045U, 1041U,
                    1178U, 1161U, 1111U, 1112U, 1090U, 1077U, 1063U, 1054U, 1042U, 1037U, 1037U, 1039U, 1039U, 1042U, 1044U, 1044U, 1048U,
                    1182U, 1141U, 1142U, 1116U, 1097U, 1079U, 1063U, 1054U, 1050U, 1045U, 1043U, 1041U, 1044U, 1051U, 1049U, 1052U, 1060U
               }
           },
    
           // ISI_COLOR_COMPONENT_GREENB
           {
               {
                    1132U, 1108U, 1098U, 1084U, 1066U, 1064U, 1056U, 1055U, 1046U, 1035U, 1031U, 1026U, 1028U, 1033U, 1042U, 1049U, 1067U,
                    1143U, 1116U, 1101U, 1087U, 1074U, 1069U, 1068U, 1062U, 1055U, 1048U, 1039U, 1035U, 1033U, 1038U, 1034U, 1069U, 1065U,
                    1140U, 1112U, 1099U, 1085U, 1074U, 1072U, 1068U, 1064U, 1061U, 1050U, 1040U, 1035U, 1033U, 1031U, 1047U, 1046U, 1064U,
                    1135U, 1112U, 1096U, 1082U, 1073U, 1069U, 1067U, 1064U, 1060U, 1051U, 1043U, 1036U, 1031U, 1032U, 1036U, 1049U, 1047U,
                    1139U, 1113U, 1096U, 1084U, 1071U, 1070U, 1071U, 1065U, 1062U, 1056U, 1046U, 1037U, 1033U, 1028U, 1030U, 1038U, 1046U,
                    1140U, 1116U, 1096U, 1085U, 1076U, 1072U, 1069U, 1065U, 1060U, 1054U, 1048U, 1045U, 1033U, 1028U, 1029U, 1035U, 1040U,
                    1142U, 1115U, 1099U, 1084U, 1078U, 1072U, 1065U, 1060U, 1057U, 1056U, 1049U, 1041U, 1032U, 1027U, 1026U, 1033U, 1039U,
                    1146U, 1119U, 1100U, 1089U, 1078U, 1075U, 1066U, 1060U, 1049U, 1053U, 1046U, 1043U, 1032U, 1026U, 1027U, 1035U, 1039U,
                    1147U, 1125U, 1105U, 1089U, 1078U, 1074U, 1067U, 1059U, 1049U, 1046U, 1045U, 1041U, 1033U, 1028U, 1027U, 1034U, 1033U,
                    1154U, 1126U, 1106U, 1092U, 1083U, 1078U, 1064U, 1059U, 1047U, 1046U, 1043U, 1044U, 1037U, 1032U, 1024U, 1033U, 1029U,
                    1156U, 1133U, 1110U, 1097U, 1084U, 1076U, 1065U, 1055U, 1049U, 1047U, 1046U, 1045U, 1040U, 1029U, 1026U, 1030U, 1035U,
                    1161U, 1137U, 1116U, 1102U, 1085U, 1072U, 1064U, 1051U, 1048U, 1049U, 1047U, 1042U, 1038U, 1031U, 1025U, 1036U, 1033U,
                    1167U, 1142U, 1120U, 1099U, 1084U, 1072U, 1058U, 1053U, 1050U, 1046U, 1042U, 1040U, 1036U, 1031U, 1030U, 1035U, 1037U,
                    1176U, 1149U, 1123U, 1102U, 1083U, 1069U, 1062U, 1056U, 1049U, 1043U, 1037U, 1039U, 1037U, 1036U, 1034U, 1039U, 1039U,
                    1188U, 1134U, 1135U, 1098U, 1085U, 1075U, 1063U, 1054U, 1047U, 1039U, 1037U, 1038U, 1038U, 1037U, 1040U, 1045U, 1041U,
                    1181U, 1165U, 1113U, 1114U, 1090U, 1077U, 1063U, 1053U, 1042U, 1036U, 1036U, 1038U, 1038U, 1041U, 1044U, 1044U, 1047U,
                    1189U, 1141U, 1144U, 1116U, 1098U, 1080U, 1063U, 1054U, 1048U, 1044U, 1042U, 1041U, 1043U, 1049U, 1049U, 1052U, 1057U
               }
           },
    
           // ISI_COLOR_COMPONENT_BLUE
           {
               {
                    1090U, 1068U, 1063U, 1053U, 1045U, 1044U, 1040U, 1044U, 1042U, 1040U, 1033U, 1036U, 1037U, 1038U, 1048U, 1051U, 1066U,
                    1101U, 1077U, 1066U, 1060U, 1049U, 1053U, 1052U, 1052U, 1053U, 1048U, 1044U, 1041U, 1039U, 1046U, 1038U, 1069U, 1063U,
                    1096U, 1076U, 1064U, 1053U, 1057U, 1051U, 1054U, 1054U, 1054U, 1053U, 1047U, 1041U, 1042U, 1039U, 1054U, 1044U, 1063U,
                    1096U, 1071U, 1064U, 1055U, 1053U, 1054U, 1056U, 1055U, 1055U, 1055U, 1047U, 1045U, 1036U, 1043U, 1041U, 1049U, 1045U,
                    1093U, 1071U, 1059U, 1054U, 1051U, 1055U, 1062U, 1058U, 1056U, 1054U, 1051U, 1044U, 1043U, 1037U, 1037U, 1040U, 1045U,
                    1092U, 1075U, 1060U, 1053U, 1058U, 1054U, 1060U, 1055U, 1054U, 1057U, 1053U, 1047U, 1039U, 1036U, 1033U, 1035U, 1041U,
                    1096U, 1072U, 1062U, 1057U, 1057U, 1057U, 1058U, 1054U, 1048U, 1054U, 1052U, 1044U, 1039U, 1029U, 1030U, 1035U, 1039U,
                    1092U, 1078U, 1066U, 1058U, 1054U, 1063U, 1053U, 1052U, 1039U, 1051U, 1048U, 1041U, 1036U, 1029U, 1031U, 1039U, 1040U,
                    1095U, 1080U, 1063U, 1061U, 1055U, 1056U, 1057U, 1050U, 1038U, 1043U, 1046U, 1041U, 1037U, 1029U, 1032U, 1037U, 1036U,
                    1102U, 1084U, 1066U, 1059U, 1058U, 1062U, 1056U, 1048U, 1039U, 1044U, 1042U, 1045U, 1041U, 1034U, 1028U, 1040U, 1032U,
                    1096U, 1089U, 1067U, 1064U, 1062U, 1056U, 1054U, 1046U, 1040U, 1040U, 1050U, 1048U, 1042U, 1036U, 1025U, 1034U, 1038U,
                    1104U, 1086U, 1074U, 1062U, 1057U, 1054U, 1052U, 1037U, 1039U, 1046U, 1047U, 1048U, 1041U, 1034U, 1024U, 1035U, 1035U,
                    1099U, 1092U, 1076U, 1060U, 1051U, 1042U, 1039U, 1040U, 1042U, 1040U, 1043U, 1041U, 1040U, 1030U, 1028U, 1034U, 1034U,
                    1108U, 1088U, 1072U, 1061U, 1046U, 1042U, 1044U, 1044U, 1040U, 1038U, 1038U, 1044U, 1037U, 1034U, 1035U, 1036U, 1039U,
                    1112U, 1064U, 1078U, 1046U, 1046U, 1046U, 1043U, 1042U, 1035U, 1033U, 1038U, 1040U, 1038U, 1036U, 1032U, 1040U, 1039U,
                    1099U, 1084U, 1042U, 1063U, 1056U, 1048U, 1047U, 1041U, 1034U, 1034U, 1037U, 1039U, 1040U, 1038U, 1037U, 1041U, 1039U,
                    1088U, 1054U, 1079U, 1065U, 1056U, 1050U, 1041U, 1045U, 1042U, 1039U, 1042U, 1045U, 1047U, 1050U, 1052U, 1047U, 1053U
               },
           },
       },
    },
};

IsiLscMatrixTable_t OV2715_LscMatrixTable_CIE_A_1920x1080 = 
{
    .ArraySize          = AWB_LSCMATRIX_ARRAY_SIZE_CIE_A,
    .psIsiVignLscMatrix = &OV2715_VignLscMatrix_CIE_A_1920x1080[0],
    .LscXGradTbl        = { 285U, 280U, 278U, 266U, 273U, 266U, 271U, 266U },
    .LscYGradTbl        = { 482U, 475U, 496U, 475U, 489U, 482U, 496U, 489U },
    .LscXSizeTbl        = { 115U, 117U, 118U, 123U, 120U, 123U, 121U, 123U },
    .LscYSizeTbl        = {  68U,  69U,  66U,  69U,  67U,  68U,  66U,  67U }
};



#ifdef __cplusplus
}
#endif

/* @} ov5630_a */

#endif /* __OV2715_A_H__ */

