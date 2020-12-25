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
 * @defgroup ov5630_D65   Illumination Profile D65
 * @{
 *
 */
#ifndef __OV2715_D65_H__
#define __OV2715_D65_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define AWB_COLORMATRIX_ARRAY_SIZE_CIE_D65  2
#define AWB_LSCMATRIX_ARRAY_SIZE_CIE_D65    1

#define AWB_SATURATION_ARRAY_SIZE_CIE_D65   4
#define AWB_VIGNETTING_ARRAY_SIZE_CIE_D65   2

#define CC_OFFSET_SCALING_D65               12.0f

/*****************************************************************************/
/*!
 * CIE D65 Indoor Profile
 *  noon daylight, 6504K
 * This illumination is not tuned for this sensor correctly! This color profile
 * might not yield satisfying results.
 */
/*****************************************************************************/

// crosstalk matrix
const Isi3x3FloatMatrix_t  OV2715_XTalkCoeff_D65 =
{
    {
        1.58917f,  -0.37637f,  -0.21280f, 
       -0.24860f,   1.73159f,  -0.48299f, 
       -0.05440f,  -1.11859f,   2.17299f  
    }
};

// crosstalk offset matrix
const IsiXTalkFloatOffset_t OV2715_XTalkOffset_D65 =
{
    .fCtOffsetRed      = (-345.4375f / CC_OFFSET_SCALING_D65),
    .fCtOffsetGreen    = (-341.8750f / CC_OFFSET_SCALING_D65),
    .fCtOffsetBlue     = (-344.9375f / CC_OFFSET_SCALING_D65)
};

// gain matrix
const IsiComponentGain_t OV2715_CompGain_D65 =
{
    .fRed      = 1.35087f,
    .fGreenR   = 1.00000f,
    .fGreenB   = 1.00000f,
    .fBlue     = 1.52010f
};

// mean value of gaussian mixture model
const Isi2x1FloatMatrix_t OV2715_GaussMeanValue_D65 =
{
    {
        0.01173f,  0.05121f
    }
};

// inverse covariance matrix
const Isi2x2FloatMatrix_t OV2715_CovarianceMatrix_D65 =
{
    {
        542.95162f,  -151.77596f, 
       -151.77596f,  2105.99536f 
    }
};

// factor in gaussian mixture model
const IsiGaussFactor_t OV2715_GaussFactor_D65 =
{
    .fGaussFactor = 168.46516f
};

// thresholds for switching between MAP classification and interpolation
const Isi2x1FloatMatrix_t OV2715_Threshold_D65 =
{
    {
        1.0f, 1.0f // 1 = disabled
    }
};

// saturation curve
float afSaturationSensorGain_D65[AWB_SATURATION_ARRAY_SIZE_CIE_D65] =
{
    1.0f, 2.0f, 4.0f, 8.0f
};

float afSaturation_D65[AWB_SATURATION_ARRAY_SIZE_CIE_D65] =
{
    100.0f, 100.0f, 90.0f, 74.0f
};

const IsiSaturationCurve_t OV2715_SaturationCurve_D65 =
{
    .ArraySize      = AWB_SATURATION_ARRAY_SIZE_CIE_D65,
    .pSensorGain    = &afSaturationSensorGain_D65[0],
    .pSaturation    = &afSaturation_D65[0]
};

// saturation depended color conversion matrices
IsiSatCcMatrix_t OV2715_SatCcMatrix_D65[AWB_COLORMATRIX_ARRAY_SIZE_CIE_D65] =
{
    {
        .fSaturation    = 74.0f,
        .XTalkCoeff     =
        {
            {
                1.42555f,  -0.26715f,  -0.15840f, 
                0.10387f,   1.25090f,  -0.35477f, 
                0.24249f,  -0.82170f,   1.57921f  
            }
        }
    },
    {
        .fSaturation    = 100.0f,
        .XTalkCoeff     =
        {
            {
                 1.58917f,  -0.37637f,  -0.21280f, 
                -0.24860f,   1.73159f,  -0.48299f, 
                -0.05440f,  -1.11859f,   2.17299f  
            }
        }
    }
 };

const IsiCcMatrixTable_t OV2715_CcMatrixTable_D65 =
{
    .ArraySize          = AWB_COLORMATRIX_ARRAY_SIZE_CIE_D65,
    .pIsiSatCcMatrix    = &OV2715_SatCcMatrix_D65[0]
};

// saturation depended color conversion offset vectors
IsiSatCcOffset_t OV2715_SatCcOffset_D65[AWB_COLORMATRIX_ARRAY_SIZE_CIE_D65] =
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
            .fCtOffsetRed      = (-345.4375f / CC_OFFSET_SCALING_D65),
            .fCtOffsetGreen    = (-341.8750f / CC_OFFSET_SCALING_D65),
            .fCtOffsetBlue     = (-344.9375f / CC_OFFSET_SCALING_D65)
        }
    }
};

const IsiCcOffsetTable_t OV2715_CcOffsetTable_D65=
{
    .ArraySize          = AWB_COLORMATRIX_ARRAY_SIZE_CIE_D65,
    .pIsiSatCcOffset    = &OV2715_SatCcOffset_D65[0]
};

// vignetting curve
float afVignettingSensorGain_D65[AWB_VIGNETTING_ARRAY_SIZE_CIE_D65] =
{
    1.0f, 8.0f
};

float afVignetting_D65[AWB_VIGNETTING_ARRAY_SIZE_CIE_D65] =
{
    100.0f, 100.0f
};

const IsiVignettingCurve_t OV2715_VignettingCurve_D65 =
{
    .ArraySize      = AWB_VIGNETTING_ARRAY_SIZE_CIE_D65,
    .pSensorGain    = &afVignettingSensorGain_D65[0],
    .pVignetting    = &afVignetting_D65[0]
};

// vignetting dependend lsc matrices
IsiVignLscMatrix_t OV2715_VignLscMatrix_CIE_D65_1920x1080[AWB_LSCMATRIX_ARRAY_SIZE_CIE_D65] = 
{
    // array item 0
    {
       .fVignetting    = 100.0f,
       .LscMatrix      =
       {
           // ISI_COLOR_COMPONENT_RED
           {
               {
                    1156U, 1124U, 1111U, 1092U, 1073U, 1063U, 1054U, 1047U, 1037U, 1035U, 1030U, 1028U, 1031U, 1036U, 1042U, 1053U, 1066U,
                    1153U, 1131U, 1111U, 1096U, 1075U, 1067U, 1057U, 1049U, 1042U, 1037U, 1035U, 1029U, 1029U, 1034U, 1042U, 1056U, 1059U,
                    1147U, 1128U, 1110U, 1092U, 1077U, 1071U, 1060U, 1052U, 1044U, 1036U, 1031U, 1030U, 1028U, 1032U, 1037U, 1045U, 1057U,
                    1147U, 1126U, 1110U, 1090U, 1078U, 1069U, 1059U, 1054U, 1039U, 1037U, 1030U, 1030U, 1026U, 1028U, 1033U, 1049U, 1047U,
                    1149U, 1131U, 1110U, 1091U, 1079U, 1068U, 1067U, 1049U, 1041U, 1038U, 1032U, 1028U, 1026U, 1026U, 1029U, 1038U, 1044U,
                    1145U, 1131U, 1110U, 1098U, 1081U, 1073U, 1059U, 1049U, 1043U, 1033U, 1032U, 1033U, 1027U, 1025U, 1030U, 1041U, 1041U,
                    1156U, 1132U, 1111U, 1095U, 1085U, 1069U, 1061U, 1049U, 1036U, 1033U, 1031U, 1030U, 1024U, 1028U, 1030U, 1036U, 1042U,
                    1150U, 1133U, 1114U, 1097U, 1084U, 1070U, 1057U, 1043U, 1033U, 1032U, 1029U, 1028U, 1027U, 1025U, 1028U, 1038U, 1042U,
                    1155U, 1139U, 1115U, 1101U, 1085U, 1072U, 1061U, 1046U, 1029U, 1030U, 1030U, 1030U, 1025U, 1026U, 1031U, 1036U, 1042U,
                    1159U, 1139U, 1117U, 1101U, 1086U, 1076U, 1061U, 1047U, 1032U, 1029U, 1030U, 1030U, 1029U, 1027U, 1026U, 1040U, 1035U,
                    1161U, 1143U, 1125U, 1104U, 1087U, 1076U, 1060U, 1049U, 1039U, 1033U, 1031U, 1029U, 1030U, 1028U, 1027U, 1037U, 1047U,
                    1167U, 1148U, 1123U, 1109U, 1090U, 1078U, 1064U, 1051U, 1042U, 1034U, 1033U, 1031U, 1030U, 1027U, 1031U, 1042U, 1047U,
                    1173U, 1152U, 1131U, 1108U, 1093U, 1080U, 1065U, 1055U, 1044U, 1037U, 1033U, 1033U, 1025U, 1030U, 1031U, 1044U, 1051U,
                    1177U, 1160U, 1135U, 1112U, 1096U, 1077U, 1070U, 1057U, 1049U, 1040U, 1035U, 1032U, 1033U, 1030U, 1037U, 1047U, 1053U,
                    1187U, 1164U, 1139U, 1115U, 1094U, 1085U, 1073U, 1063U, 1053U, 1044U, 1036U, 1035U, 1031U, 1031U, 1038U, 1054U, 1056U,
                    1193U, 1168U, 1142U, 1123U, 1104U, 1087U, 1073U, 1063U, 1051U, 1045U, 1038U, 1033U, 1035U, 1038U, 1045U, 1057U, 1064U,
                    1196U, 1169U, 1147U, 1127U, 1107U, 1089U, 1077U, 1062U, 1055U, 1049U, 1039U, 1035U, 1037U, 1046U, 1048U, 1062U, 1068U
               }
           }, 
    
           // ISI_COLOR_COMPONENT_GREENR
           {
               {
                    1122U, 1100U, 1092U, 1081U, 1069U, 1066U, 1060U, 1055U, 1052U, 1039U, 1036U, 1028U, 1024U, 1029U, 1036U, 1046U, 1057U,
                    1137U, 1114U, 1105U, 1091U, 1080U, 1081U, 1077U, 1072U, 1064U, 1058U, 1047U, 1039U, 1035U, 1038U, 1043U, 1060U, 1066U,
                    1132U, 1116U, 1100U, 1087U, 1083U, 1080U, 1080U, 1078U, 1069U, 1062U, 1052U, 1044U, 1038U, 1036U, 1042U, 1056U, 1059U,
                    1135U, 1113U, 1099U, 1089U, 1084U, 1081U, 1079U, 1079U, 1072U, 1063U, 1052U, 1047U, 1037U, 1037U, 1037U, 1047U, 1048U,
                    1138U, 1120U, 1102U, 1093U, 1084U, 1083U, 1086U, 1080U, 1077U, 1070U, 1060U, 1050U, 1042U, 1037U, 1037U, 1045U, 1047U,
                    1136U, 1122U, 1106U, 1093U, 1089U, 1086U, 1083U, 1079U, 1076U, 1071U, 1064U, 1057U, 1042U, 1037U, 1037U, 1043U, 1043U,
                    1142U, 1124U, 1106U, 1098U, 1091U, 1085U, 1082U, 1077U, 1074U, 1074U, 1066U, 1056U, 1045U, 1042U, 1036U, 1045U, 1046U,
                    1149U, 1127U, 1113U, 1102U, 1094U, 1091U, 1084U, 1080U, 1072U, 1073U, 1066U, 1056U, 1050U, 1040U, 1040U, 1046U, 1046U,
                    1149U, 1135U, 1118U, 1104U, 1097U, 1093U, 1084U, 1081U, 1072U, 1068U, 1067U, 1059U, 1052U, 1048U, 1041U, 1045U, 1045U,
                    1160U, 1139U, 1119U, 1108U, 1099U, 1098U, 1089U, 1080U, 1073U, 1069U, 1062U, 1066U, 1055U, 1049U, 1044U, 1046U, 1046U,
                    1159U, 1147U, 1124U, 1112U, 1104U, 1096U, 1084U, 1077U, 1074U, 1070U, 1068U, 1064U, 1062U, 1049U, 1042U, 1046U, 1048U,
                    1167U, 1151U, 1132U, 1121U, 1103U, 1096U, 1090U, 1078U, 1075U, 1074U, 1069U, 1066U, 1058U, 1049U, 1047U, 1054U, 1051U,
                    1175U, 1158U, 1138U, 1120U, 1107U, 1096U, 1085U, 1081U, 1078U, 1074U, 1069U, 1064U, 1061U, 1057U, 1051U, 1060U, 1055U,
                    1186U, 1162U, 1140U, 1123U, 1105U, 1096U, 1091U, 1087U, 1080U, 1072U, 1067U, 1066U, 1061U, 1061U, 1058U, 1063U, 1063U,
                    1190U, 1170U, 1148U, 1122U, 1109U, 1105U, 1091U, 1088U, 1081U, 1075U, 1071U, 1068U, 1067U, 1063U, 1065U, 1072U, 1066U,
                    1202U, 1167U, 1144U, 1135U, 1119U, 1106U, 1096U, 1089U, 1083U, 1077U, 1073U, 1071U, 1068U, 1069U, 1072U, 1075U, 1072U,
                    1192U, 1169U, 1158U, 1139U, 1121U, 1110U, 1095U, 1086U, 1084U, 1082U, 1078U, 1075U, 1072U, 1079U, 1077U, 1078U, 1087U
               },
           },
    
           // ISI_COLOR_COMPONENT_GREENB
           {
               {
                    1124U, 1099U, 1094U, 1082U, 1070U, 1066U, 1061U, 1056U, 1052U, 1039U, 1036U, 1029U, 1024U, 1031U, 1033U, 1046U, 1056U, 
                    1140U, 1116U, 1104U, 1091U, 1081U, 1082U, 1077U, 1072U, 1062U, 1058U, 1045U, 1040U, 1034U, 1035U, 1042U, 1059U, 1067U,
                    1132U, 1117U, 1102U, 1089U, 1081U, 1081U, 1080U, 1078U, 1069U, 1061U, 1050U, 1042U, 1038U, 1036U, 1041U, 1054U, 1057U,
                    1139U, 1114U, 1101U, 1090U, 1083U, 1083U, 1079U, 1078U, 1072U, 1062U, 1053U, 1046U, 1038U, 1036U, 1037U, 1046U, 1049U,
                    1139U, 1121U, 1103U, 1094U, 1084U, 1084U, 1087U, 1080U, 1078U, 1069U, 1059U, 1049U, 1042U, 1036U, 1035U, 1045U, 1046U,
                    1139U, 1123U, 1107U, 1093U, 1089U, 1086U, 1082U, 1080U, 1075U, 1072U, 1064U, 1056U, 1042U, 1037U, 1037U, 1041U, 1044U,
                    1145U, 1126U, 1108U, 1098U, 1091U, 1086U, 1082U, 1078U, 1075U, 1073U, 1067U, 1055U, 1044U, 1042U, 1034U, 1044U, 1045U,
                    1150U, 1130U, 1115U, 1103U, 1094U, 1090U, 1084U, 1079U, 1072U, 1073U, 1065U, 1056U, 1049U, 1041U, 1040U, 1045U, 1046U,
                    1153U, 1136U, 1120U, 1105U, 1096U, 1094U, 1084U, 1082U, 1072U, 1067U, 1067U, 1059U, 1052U, 1046U, 1042U, 1045U, 1044U,
                    1162U, 1141U, 1120U, 1109U, 1100U, 1097U, 1089U, 1081U, 1071U, 1070U, 1061U, 1064U, 1055U, 1049U, 1043U, 1047U, 1046U,
                    1160U, 1149U, 1126U, 1113U, 1104U, 1096U, 1084U, 1078U, 1074U, 1070U, 1067U, 1065U, 1060U, 1048U, 1043U, 1047U, 1047U,
                    1171U, 1153U, 1132U, 1121U, 1104U, 1097U, 1090U, 1077U, 1076U, 1073U, 1068U, 1065U, 1059U, 1049U, 1046U, 1054U, 1050U,
                    1178U, 1159U, 1141U, 1120U, 1107U, 1096U, 1083U, 1082U, 1077U, 1074U, 1069U, 1064U, 1060U, 1054U, 1052U, 1060U, 1055U,
                    1188U, 1163U, 1143U, 1123U, 1106U, 1097U, 1092U, 1085U, 1081U, 1071U, 1066U, 1067U, 1062U, 1061U, 1058U, 1064U, 1061U,
                    1192U, 1170U, 1149U, 1122U, 1110U, 1104U, 1092U, 1088U, 1081U, 1074U, 1071U, 1068U, 1067U, 1062U, 1065U, 1071U, 1066U,
                    1204U, 1169U, 1145U, 1136U, 1118U, 1109U, 1095U, 1088U, 1084U, 1076U, 1072U, 1070U, 1068U, 1070U, 1072U, 1073U, 1072U,
                    1193U, 1167U, 1160U, 1139U, 1121U, 1109U, 1096U, 1086U, 1085U, 1082U, 1080U, 1073U, 1073U, 1078U, 1078U, 1078U, 1087U
               },
           },
    
           // ISI_COLOR_COMPONENT_BLUE
           {
               {
                    1080U, 1067U, 1058U, 1054U, 1047U, 1046U, 1045U, 1047U, 1046U, 1037U, 1030U, 1030U, 1028U, 1024U, 1032U, 1035U, 1047U,
                    1095U, 1081U, 1073U, 1067U, 1061U, 1062U, 1062U, 1064U, 1060U, 1056U, 1049U, 1046U, 1035U, 1042U, 1037U, 1048U, 1052U,
                    1097U, 1083U, 1070U, 1066U, 1066U, 1067U, 1069U, 1071U, 1066U, 1060U, 1052U, 1044U, 1040U, 1037U, 1038U, 1045U, 1046U,
                    1095U, 1082U, 1071U, 1068U, 1067U, 1069U, 1074U, 1073U, 1066U, 1063U, 1056U, 1047U, 1042U, 1037U, 1039U, 1039U, 1046U,
                    1093U, 1084U, 1073U, 1071U, 1066U, 1073U, 1077U, 1071U, 1071U, 1068U, 1061U, 1050U, 1045U, 1036U, 1037U, 1040U, 1038U,
                    1101U, 1087U, 1074U, 1071U, 1074U, 1076U, 1081U, 1071U, 1074U, 1070U, 1065U, 1058U, 1045U, 1039U, 1035U, 1041U, 1033U,
                    1101U, 1086U, 1075U, 1075U, 1075U, 1075U, 1072U, 1076U, 1066U, 1070U, 1065U, 1059U, 1046U, 1042U, 1033U, 1038U, 1044U,
                    1099U, 1093U, 1080U, 1074U, 1074U, 1077U, 1074U, 1071U, 1067U, 1069U, 1066U, 1057U, 1046U, 1040U, 1034U, 1047U, 1041U,
                    1104U, 1098U, 1084U, 1081U, 1079U, 1079U, 1079U, 1074U, 1063U, 1066U, 1059U, 1057U, 1050U, 1045U, 1042U, 1044U, 1039U,
                    1110U, 1099U, 1086U, 1082U, 1078U, 1082U, 1080U, 1073U, 1064U, 1063U, 1063U, 1063U, 1055U, 1048U, 1039U, 1048U, 1039U,
                    1111U, 1101U, 1091U, 1083U, 1079U, 1083U, 1075U, 1072U, 1065U, 1067U, 1066U, 1067U, 1056U, 1046U, 1040U, 1048U, 1043U,
                    1113U, 1105U, 1096U, 1086U, 1082U, 1081U, 1075U, 1066U, 1069U, 1067U, 1070U, 1063U, 1057U, 1049U, 1043U, 1048U, 1048U,
                    1118U, 1113U, 1097U, 1086U, 1077U, 1073U, 1072U, 1072U, 1070U, 1073U, 1065U, 1065U, 1055U, 1052U, 1046U, 1052U, 1047U,
                    1122U, 1111U, 1096U, 1083U, 1073U, 1072U, 1071U, 1079U, 1071U, 1071U, 1067U, 1066U, 1061U, 1054U, 1054U, 1056U, 1051U,
                    1121U, 1107U, 1094U, 1076U, 1076U, 1081U, 1080U, 1073U, 1075U, 1066U, 1066U, 1067U, 1061U, 1056U, 1056U, 1066U, 1053U,
                    1124U, 1103U, 1087U, 1088U, 1084U, 1084U, 1077U, 1082U, 1072U, 1075U, 1069U, 1070U, 1067U, 1065U, 1064U, 1069U, 1061U,
                    1093U, 1083U, 1091U, 1086U, 1076U, 1071U, 1069U, 1069U, 1070U, 1064U, 1068U, 1066U, 1066U, 1063U, 1067U, 1068U, 1071U
               },
           },
       },
    },
};

IsiLscMatrixTable_t OV2715_LscMatrixTable_CIE_D65_1920x1080 = 
{
    .ArraySize          = AWB_LSCMATRIX_ARRAY_SIZE_CIE_D65,
    .psIsiVignLscMatrix = &OV2715_VignLscMatrix_CIE_D65_1920x1080[0],
    .LscXGradTbl        = { 285U, 280U, 278U, 266U, 273U, 266U, 271U, 266U },
    .LscYGradTbl        = { 482U, 475U, 496U, 475U, 489U, 482U, 496U, 489U },
    .LscXSizeTbl        = { 115U, 117U, 118U, 123U, 120U, 123U, 121U, 123U },
    .LscYSizeTbl        = {  68U,  69U,  66U,  69U,  67U,  68U,  66U,  67U }
};



#ifdef __cplusplus
}
#endif

/* @} ov5630_D65 */

#endif /* __OV2715_D65_H__ */

