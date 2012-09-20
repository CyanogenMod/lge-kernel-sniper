//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2011.07.26] - CAM : from justin froyo
/*
 * drivers/media/video/mt9v113_regs.h
 *
 * Secondary Camera Property (mt9v113 Sensor).
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#define MT9V113_MIN_BRIGHTNESS 0
#define MT9V113_MAX_BRIGHTNESS 12
#define MT9V113_BRIGHTNESS_STEP 1
#define MT9V113_DEF_BRIGHTNESS 7

#define MT9V113_MIN_FLICKER 0
#define MT9V113_MAX_FLICKER 1
#define MT9V113_FLICKER_STEP 1
#define MT9V113_DEF_FLICKER 0

#define MT9V113_MIN_COLORFX 0 
#define MT9V113_MAX_COLORFX 3
#define MT9V113_COLORFX_STEP 1
#define MT9V113_DEF_COLORFX 0

#define MT9V113_MIN_WB 0
#define MT9V113_MAX_WB 3
#define MT9V113_WB_STEP 1
#define MT9V113_DEF_WB 0

#define MT9V113_MIN_MIRROR 0
#define MT9V113_MAX_MIRROR 3
#define MT9V113_MIRROR_STEP 1
#define MT9V113_DEF_MIRROR 0

#define MT9V113_MIN_METERING 0
#define MT9V113_MAX_METERING 1
#define MT9V113_METERING_STEP 1
#define MT9V113_DEF_METERING 0

const static struct mt9v113_reg pll_settings[] =
{
//VERSION 2

//***********************************************************************/
//Module Company: LGIT
//Date:2008.06.27
//MCLK:24.543 MHz
//PCLK:28.122 MHz
//Flicker : auto
//Preview Max 30 fps:fps
//Capture Max fps:14.645 fps
//Sensor: MICRON SoC MT9V113
//Slave Addr: 0x78
//preview size: VGA
//Mirror mode: 
//Customer: LGElectronics
//***********************************************************************/
// 1. AWB MIN POS 20  MAX POS 7F 
// 2. GAMMA black 2 gamma 0.35 contrast 1.3 
// 3. AWB PIXEL COUNT 0xA00
// 4. AE POSITION 변경  (BOTTOM INCLUDE)
// 5. AE SPEED 재조정 1->3
// 6. RESET SEQUENCE 원복
// 7. Low Light saturation 1 80->43   saturation2 10->00  
// 8. black level 2 -> 6, low light gain F0 -> 120, Edge 3 -> 4, KG-R 82 -> 84
//[MCLK 24Mhz  PCLK 24Mhz  640x480 22fps]
{0x0014, 0xB04B, I2C_16BIT},
{0x0014, 0xB049, I2C_16BIT},
{0x0010, 0x0110, I2C_16BIT}, // PLL_DIVIDERS = 1335
{0x0012, 0x0000, I2C_16BIT}, // PLL_P_DIVIDERS
{0x0014, 0x244B, I2C_16BIT},
{0xFFFE, 0x000A, I2C_16BIT}, // DELAY=10
{0x0014, 0x304B, I2C_16BIT},
{0xFFFE, 0x000A, I2C_16BIT}, // DELAY=10
{0x0014, 0xB04A, I2C_16BIT},

//{0x0016, 0x02DF, I2C_16BIT}, // PCLK Inverter KJK
{0x001E, 0x0707, I2C_16BIT}, //606 //PLEW KJK

{0x098C, 0xA111, I2C_16BIT}, // Bad Frame
{0x0990, 0x000A, I2C_16BIT}, //

{0x098C, 0x2703, I2C_16BIT}, // Output Width (A)
{0x0990, 0x0280, I2C_16BIT}, //      = 640
{0x098C, 0x2705, I2C_16BIT}, // Output Height(A)
{0x0990, 0x01E0, I2C_16BIT}, //      = 480
{0x098C, 0x2707, I2C_16BIT}, // Output Width (B)
{0x0990, 0x0280, I2C_16BIT}, //      = 640
{0x098C, 0x2709, I2C_16BIT}, // Output Height(B)
{0x0990, 0x01E0, I2C_16BIT}, //      = 480
{0x098C, 0x270D, I2C_16BIT}, // Row Start    (A)
{0x0990, 0x0000, I2C_16BIT}, //      = 0
{0x098C, 0x270F, I2C_16BIT}, // Column Start (A)
{0x0990, 0x0000, I2C_16BIT}, //      = 0
{0x098C, 0x2711, I2C_16BIT}, // Row End      (A)
{0x0990, 0x01E7, I2C_16BIT}, //      = 487
{0x098C, 0x2713, I2C_16BIT}, // Column End   (A)
{0x0990, 0x0287, I2C_16BIT}, //      = 647
{0x098C, 0x2715, I2C_16BIT}, // Row Speed    (A)
{0x0990, 0x0001, I2C_16BIT}, //      = 1
{0x098C, 0x2717, I2C_16BIT}, // Read Mode    (A)***
{0x0990, 0x0026, I2C_16BIT}, // default(0x0024) KJK 180 degree	Change by MMS 0x27->0x26
{0x098C, 0x2719, I2C_16BIT}, // sensor_fine_correction (A)
{0x0990, 0x001A, I2C_16BIT}, //      = 26
{0x098C, 0x271B, I2C_16BIT}, // sensor_fine_IT_min (A)
{0x0990, 0x006B, I2C_16BIT}, //      = 107
{0x098C, 0x271D, I2C_16BIT}, // sensor_fine_IT_max_margin (A)
{0x0990, 0x006B, I2C_16BIT}, //      = 107
//[Normal mode : support 10~22fps]
{0x098C, 0x271F, I2C_16BIT}, // Frame Lines  (A)
{0x0990, 0x01FB, I2C_16BIT}, //      = 556
{0x098C, 0x2721, I2C_16BIT}, // Line Length  (A)
{0x0990, 0x0433, I2C_16BIT}, // Line Length  (A)

//[MIN. frame rate]
{0x098C, 0xA20C, I2C_16BIT}, // MCU_ADDRESS
//{0x0990, 0x000A, I2C_16BIT}, // AE_MAX_INDEX (12FPS ~:0x000A)(10FPS~:0x000C),(7.5FPS~:0x0010)
{0x0990, 0x000C, I2C_16BIT},

{0x098C, 0x2723, I2C_16BIT}, // Row Start    (B)
{0x0990, 0x0000, I2C_16BIT}, //      = 0
{0x098C, 0x2725, I2C_16BIT}, // Column Start (B)
{0x0990, 0x0000, I2C_16BIT}, //      = 0
{0x098C, 0x2727, I2C_16BIT}, // Row End (B)
{0x0990, 0x01E7, I2C_16BIT}, //      = 487
{0x098C, 0x2729, I2C_16BIT}, // Column End   (B)
{0x0990, 0x0287, I2C_16BIT}, //      = 647
{0x098C, 0x272B, I2C_16BIT}, // Row Speed    (B)
{0x0990, 0x0001, I2C_16BIT}, //      = 1
{0x098C, 0x272D, I2C_16BIT}, // Read Mode    (B)***
{0x0990, 0x0026, I2C_16BIT}, // default(0x0024) KJK 180 degree	Change by MMS 0x27->0x26
{0x098C, 0x272F, I2C_16BIT}, // sensor_fine_correction (B)
{0x0990, 0x001A, I2C_16BIT}, //      = 26
{0x098C, 0x2731, I2C_16BIT}, // sensor_fine_IT_min (B)
{0x0990, 0x006B, I2C_16BIT}, //      = 107
{0x098C, 0x2733, I2C_16BIT}, // sensor_fine_IT_max_margin (B) 
{0x0990, 0x006B, I2C_16BIT}, //      = 107                    
{0x098C, 0x2735, I2C_16BIT}, // Frame Lines (B)               
{0x0990, 0x01FB, I2C_16BIT}, //      = 1140                   
{0x098C, 0x2737, I2C_16BIT}, // Line Length (B)               
{0x0990, 0x0433, I2C_16BIT}, //      = 842                    
{0x098C, 0x2739, I2C_16BIT}, // Crop_X0 (A)                   
{0x0990, 0x0000, I2C_16BIT}, //      = 0                      
{0x098C, 0x273B, I2C_16BIT}, // Crop_X1 (A)                   
{0x0990, 0x027F, I2C_16BIT}, //      = 639                    
{0x098C, 0x273D, I2C_16BIT}, // Crop_Y0 (A)                   
{0x0990, 0x0000, I2C_16BIT}, //      = 0                      
{0x098C, 0x273F, I2C_16BIT}, // Crop_Y1 (A)                   
{0x0990, 0x01DF, I2C_16BIT}, //      = 479                    
{0x098C, 0x2747, I2C_16BIT}, // Crop_X0 (B)                   
{0x0990, 0x0000, I2C_16BIT}, //      = 0                      
{0x098C, 0x2749, I2C_16BIT}, // Crop_X1 (B)                   
{0x0990, 0x027F, I2C_16BIT}, //      = 639                    
{0x098C, 0x274B, I2C_16BIT}, // Crop_Y0 (B)                   
{0x0990, 0x0000, I2C_16BIT}, //      = 0                      
{0x098C, 0x274D, I2C_16BIT}, // Crop_Y1 (B)                   
{0x0990, 0x01DF, I2C_16BIT}, //      = 479                    
{0x098C, 0x222D, I2C_16BIT}, // R9 Step                       
{0x0990, 0x005D, I2C_16BIT}, //      = 139                    
{0x098C, 0xA408, I2C_16BIT}, // search_f1_50                  
{0x0990, 0x001E, I2C_16BIT}, //      = 33                     
{0x098C, 0xA409, I2C_16BIT}, // search_f2_50                  
{0x0990, 0x0020, I2C_16BIT}, //      = 36                     
{0x098C, 0xA40A, I2C_16BIT}, // search_f1_60                  
{0x0990, 0x0024, I2C_16BIT}, //      = 40                     
{0x098C, 0xA40B, I2C_16BIT}, // search_f2_60                  
{0x0990, 0x0026, I2C_16BIT}, //      = 43                     
{0x098C, 0x2411, I2C_16BIT}, // R9_Step_60_A                  
{0x0990, 0x005D, I2C_16BIT}, //      = 139                    
{0x098C, 0x2413, I2C_16BIT}, // R9_Step_50_A                  
{0x0990, 0x0070, I2C_16BIT}, //      = 167                    
{0x098C, 0x2415, I2C_16BIT}, // R9_Step_60_B                  
{0x0990, 0x005D, I2C_16BIT}, //      = 139                    
{0x098C, 0x2417, I2C_16BIT}, // R9_Step_50_B                  
{0x0990, 0x0070, I2C_16BIT}, //   = 167                       
{0x098C, 0xA404, I2C_16BIT}, // FD Mode                        
{0x0990, 0x0010, I2C_16BIT}, //      = 16                     
{0x098C, 0xA40D, I2C_16BIT}, // Stat_min                      
{0x0990, 0x0002, I2C_16BIT}, //      = 2                      
{0x098C, 0xA40E, I2C_16BIT}, // Stat_max                       
{0x0990, 0x0003, I2C_16BIT}, //  = 3                          
{0x098C, 0xA410, I2C_16BIT}, // Min_amplitude                 
{0x0990, 0x000A, I2C_16BIT}, //  = 10            

//[Lens Correction 85]                     
{0x3658, 0x0130, I2C_16BIT}, //  P_RD_P0Q0    
{0x365A, 0x030D, I2C_16BIT}, //  P_RD_P0Q1    
{0x365C, 0x6B92, I2C_16BIT}, //  P_RD_P0Q2    
{0x365E, 0xE62E, I2C_16BIT}, //  P_RD_P0Q3    
{0x3660, 0x53B4, I2C_16BIT}, //  P_RD_P0Q4    
{0x3680, 0x1AED, I2C_16BIT}, //  P_RD_P1Q0    
{0x3682, 0x2A6A, I2C_16BIT}, //  P_RD_P1Q1    
{0x3684, 0xBA2B, I2C_16BIT}, //  P_RD_P1Q2    
{0x3686, 0x392F, I2C_16BIT}, //  P_RD_P1Q3    
{0x3688, 0xAD53, I2C_16BIT}, //  P_RD_P1Q4    
{0x36A8, 0x1453, I2C_16BIT}, //  P_RD_P2Q0    
{0x36AA, 0xC78D, I2C_16BIT}, //  P_RD_P2Q1    
{0x36AC, 0x4C35, I2C_16BIT}, //  P_RD_P2Q2    
{0x36AE, 0x7791, I2C_16BIT}, //  P_RD_P2Q3    
{0x36B0, 0x6797, I2C_16BIT}, //  P_RD_P2Q4    
{0x36D0, 0xE3F0, I2C_16BIT}, //  P_RD_P3Q0    
{0x36D2, 0xF8F0, I2C_16BIT}, //  P_RD_P3Q1    
{0x36D4, 0x5934, I2C_16BIT}, //  P_RD_P3Q2    
{0x36D6, 0x6AF2, I2C_16BIT}, //  P_RD_P3Q3    
{0x36D8, 0xFC98, I2C_16BIT}, //  P_RD_P3Q4    
{0x36F8, 0x2EB4, I2C_16BIT}, //  P_RD_P4Q0    
{0x36FA, 0x9972, I2C_16BIT}, //  P_RD_P4Q1    
{0x36FC, 0x5E33, I2C_16BIT}, //  P_RD_P4Q2    
{0x36FE, 0x07D8, I2C_16BIT}, //  P_RD_P4Q3    
{0x3700, 0x51DC, I2C_16BIT}, //  P_RD_P4Q4    
{0x364E, 0x0170, I2C_16BIT}, //  P_GR_P0Q0    
{0x3650, 0x570C, I2C_16BIT}, //  P_GR_P0Q1    
{0x3652, 0x6652, I2C_16BIT}, //  P_GR_P0Q2    
{0x3654, 0xB5B0, I2C_16BIT}, //  P_GR_P0Q3    
{0x3656, 0x3274, I2C_16BIT}, //  P_GR_P0Q4    
{0x3676, 0x214D, I2C_16BIT}, //  P_GR_P1Q0    
{0x3678, 0x3D46, I2C_16BIT}, //  P_GR_P1Q1    
{0x367A, 0x218F, I2C_16BIT}, //  P_GR_P1Q2    
{0x367C, 0x13D0, I2C_16BIT}, //  P_GR_P1Q3    
{0x367E, 0x8C34, I2C_16BIT}, //  P_GR_P1Q4    
{0x369E, 0x1253, I2C_16BIT}, //  P_GR_P2Q0    
{0x36A0, 0x98B0, I2C_16BIT}, //  P_GR_P2Q1    
{0x36A2, 0x3275, I2C_16BIT}, //  P_GR_P2Q2    
{0x36A4, 0x4FD0, I2C_16BIT}, //  P_GR_P2Q3    
{0x36A6, 0x0896, I2C_16BIT}, //  P_GR_P2Q4    
{0x36C6, 0x82D0, I2C_16BIT}, //  P_GR_P3Q0    
{0x36C8, 0x8292, I2C_16BIT}, //  P_GR_P3Q1    
{0x36CA, 0x0CD5, I2C_16BIT}, //  P_GR_P3Q2    
{0x36CC, 0x2632, I2C_16BIT}, //  P_GR_P3Q3    
{0x36CE, 0xC659, I2C_16BIT}, //  P_GR_P3Q4    
{0x36EE, 0x2374, I2C_16BIT}, //  P_GR_P4Q0    
{0x36F0, 0xDC32, I2C_16BIT}, //  P_GR_P4Q1    
{0x36F2, 0xCBF7, I2C_16BIT}, //  P_GR_P4Q2    
{0x36F4, 0x1918, I2C_16BIT}, //  P_GR_P4Q3    
{0x36F6, 0x675C, I2C_16BIT}, //  P_GR_P4Q4    
{0x3662, 0x00D0, I2C_16BIT}, //  P_BL_P0Q0    
{0x3664, 0x67AC, I2C_16BIT}, //  P_BL_P0Q1    
{0x3666, 0x6CD2, I2C_16BIT}, //  P_BL_P0Q2    
{0x3668, 0xB350, I2C_16BIT}, //  P_BL_P0Q3    
{0x366A, 0x0CD4, I2C_16BIT}, //  P_BL_P0Q4    
{0x368A, 0x400C, I2C_16BIT}, //  P_BL_P1Q0    
{0x368C, 0x07ED, I2C_16BIT}, //  P_BL_P1Q1    
{0x368E, 0x57CF, I2C_16BIT}, //  P_BL_P1Q2    
{0x3690, 0x3C50, I2C_16BIT}, //  P_BL_P1Q3    
{0x3692, 0xD654, I2C_16BIT}, //  P_BL_P1Q4    
{0x36B2, 0x0213, I2C_16BIT}, //  P_BL_P2Q0    
{0x36B4, 0x2CCF, I2C_16BIT}, //  P_BL_P2Q1    
{0x36B6, 0x1636, I2C_16BIT}, //  P_BL_P2Q2    
{0x36B8, 0x8EB5, I2C_16BIT}, //  P_BL_P2Q3    
{0x36BA, 0xF338, I2C_16BIT}, //  P_BL_P2Q4    
{0x36DA, 0xD310, I2C_16BIT}, //  P_BL_P3Q0    
{0x36DC, 0x22B0, I2C_16BIT}, //  P_BL_P3Q1    
{0x36DE, 0x3273, I2C_16BIT}, //  P_BL_P3Q2    
{0x36E0, 0x9BF6, I2C_16BIT}, //  P_BL_P3Q3    
{0x36E2, 0x82D8, I2C_16BIT}, //  P_BL_P3Q4    
{0x3702, 0x4854, I2C_16BIT}, //  P_BL_P4Q0    
{0x3704, 0x9AD5, I2C_16BIT}, //  P_BL_P4Q1    
{0x3706, 0xD519, I2C_16BIT}, //  P_BL_P4Q2    
{0x3708, 0x5D99, I2C_16BIT}, //  P_BL_P4Q3    
{0x370A, 0x685D, I2C_16BIT}, //  P_BL_P4Q4    
{0x366C, 0x00D0, I2C_16BIT}, //  P_GB_P0Q0    
{0x366E, 0x498C, I2C_16BIT}, //  P_GB_P0Q1    
{0x3670, 0x6B32, I2C_16BIT}, //  P_GB_P0Q2    
{0x3672, 0xA910, I2C_16BIT}, //  P_GB_P0Q3    
{0x3674, 0x3614, I2C_16BIT}, //  P_GB_P0Q4    
{0x3694, 0x3FAC, I2C_16BIT}, //  P_GB_P1Q0    
{0x3696, 0xB68A, I2C_16BIT}, //  P_GB_P1Q1    
{0x3698, 0x0FB0, I2C_16BIT}, //  P_GB_P1Q2    
{0x369A, 0x5E70, I2C_16BIT}, //  P_GB_P1Q3    
{0x369C, 0xD6B4, I2C_16BIT}, //  P_GB_P1Q4    
{0x36BC, 0x09F3, I2C_16BIT}, //  P_GB_P2Q0    
{0x36BE, 0xB9F1, I2C_16BIT}, //  P_GB_P2Q1    
{0x36C0, 0x3C55, I2C_16BIT}, //  P_GB_P2Q2    
{0x36C2, 0x2035, I2C_16BIT}, //  P_GB_P2Q3    
{0x36C4, 0x5B55, I2C_16BIT}, //  P_GB_P2Q4    
{0x36E4, 0x9450, I2C_16BIT}, //  P_GB_P3Q0    
{0x36E6, 0xF171, I2C_16BIT}, //  P_GB_P3Q1    
{0x36E8, 0x16D4, I2C_16BIT}, //  P_GB_P3Q2    
{0x36EA, 0x8234, I2C_16BIT}, //  P_GB_P3Q3    
{0x36EC, 0x98F9, I2C_16BIT}, //  P_GB_P3Q4    
{0x370C, 0x3854, I2C_16BIT}, //  P_GB_P4Q0    
{0x370E, 0x3754, I2C_16BIT}, //  P_GB_P4Q1    
{0x3710, 0xBED7, I2C_16BIT}, //  P_GB_P4Q2    
{0x3712, 0xF557, I2C_16BIT}, //  P_GB_P4Q3    
{0x3714, 0x4B7C, I2C_16BIT}, //  P_GB_P4Q4    
{0x3644, 0x0148, I2C_16BIT}, //  POLY_ORIGIN_C
{0x3642, 0x00F0, I2C_16BIT}, //  POLY_ORIGIN_R         
{0x3210, 0x09B8, I2C_16BIT}, //  COLOR_PIPELINE_CONTROL

//awb_and_ccm
{0x098C, 0x2306, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_0]
{0x0990, 0x0133, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2308, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_1]
{0x0990, 0xFFC4, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x230A, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_2]
{0x0990, 0x0014, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x230C, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_3]
{0x0990, 0xFF64, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x230E, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_4]
{0x0990, 0x01E3, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2310, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_5]
{0x0990, 0xFFB2, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2312, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_6]
{0x0990, 0xFF9A, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2314, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_7]
{0x0990, 0xFEDB, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2316, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_8]
{0x0990, 0x0213, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2318, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_9]
{0x0990, 0x001C, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x231A, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_L_10]
{0x0990, 0x003A, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x231C, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_0]
{0x0990, 0x0064, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x231E, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_1]
{0x0990, 0xFF7D, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2320, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_2]
{0x0990, 0xFFFF, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2322, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_3]
{0x0990, 0x001A, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2324, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_4]
{0x0990, 0xFF94, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2326, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_5]
{0x0990, 0x0048, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2328, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_6]
{0x0990, 0x001B, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x232A, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_7]
{0x0990, 0x0166, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x232C, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_8]
{0x0990, 0xFEE3, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x232E, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_9]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2330, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_RL_10]
{0x0990, 0xFFDC, I2C_16BIT}, // MCU_DATA_0             
{0x098C, 0xA348, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0008, I2C_16BIT}, // AWB_GAIN_BUFFER_SPEED      
{0x098C, 0xA349, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0002, I2C_16BIT}, // AWB_JUMP_DIVISOR           
{0x098C, 0xA34A, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0059, I2C_16BIT}, // AWB_GAINMIN_R               
{0x098C, 0xA34B, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x00E6, I2C_16BIT}, // AWB_GAINMAX_R
{0x098C, 0xA34C, I2C_16BIT}, // MCU_ADDRESS      
{0x0990, 0x0059, I2C_16BIT}, // AWB_GAINMIN_B                        
{0x098C, 0xA34D, I2C_16BIT}, // MCU_ADDRESS     
{0x0990, 0x00A6, I2C_16BIT}, // AWB_GAINMAX_B
{0x098C, 0xA34E, I2C_16BIT}, // MCU_ADDRESS     
{0x0990, 0x00C7, I2C_16BIT}, // AWB_GAIN_R
{0x098C, 0xA34F, I2C_16BIT}, // MCU_ADDRESS     
{0x0990, 0x0080, I2C_16BIT}, // AWB_GAIN_G
{0x098C, 0xA350, I2C_16BIT}, // MCU_ADDRESS     
{0x0990, 0x0080, I2C_16BIT}, // AWB_GAIN_B     
   
//[AWB MIN POS CHANGE BY DANIEL]
{0x098C, 0xA351, I2C_16BIT}, // MCU_ADDRESS [AWB_CCM_POSITION_MIN]
{0x0990, 0x0020, I2C_16BIT}, // MCU_DATA_0   
{0x098C, 0xA352, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x007F, I2C_16BIT}, // AWB_CCM_POSITION_MAX
{0x098C, 0xA353, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0040, I2C_16BIT}, // AWB_CCM_POSITION
{0x098C, 0xA354, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0060, I2C_16BIT}, // AWB_SATURATION             
{0x098C, 0xA355, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0001, I2C_16BIT}, // AWB_MODE                   
{0x098C, 0xA35D, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0078, I2C_16BIT}, // AWB_STEADY_BGAIN_OUT_MIN   
{0x098C, 0xA35E, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0086, I2C_16BIT}, // AWB_STEADY_BGAIN_OUT_MAX   
{0x098C, 0xA35F, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x007E, I2C_16BIT}, // AWB_STEADY_BGAIN_IN_MIN    
{0x098C, 0xA360, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0082, I2C_16BIT}, // AWB_STEADY_BGAIN_IN_MAX
{0x098C, 0xA302, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0000, I2C_16BIT}, // AWB_WINDOW_POS             
{0x098C, 0xA303, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x00EF, I2C_16BIT}, // AWB_WINDOW_SIZE          

//전체 색 조정(RED higher)
{0x098C, 0xA364, I2C_16BIT}, // MCU_ADDRESS [AWB_64]
{0x0990, 0x00E4, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xA365, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0000, I2C_16BIT}, // AWB_X0 <-0x0010
{0x098C, 0xA366, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0080, I2C_16BIT}, // AWB_KR_L
{0x098C, 0xA367, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0080, I2C_16BIT}, // AWB_KG_L
{0x098C, 0xA368, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0080, I2C_16BIT}, // AWB_KB_L
{0x098C, 0xA369, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0083, I2C_16BIT}, //7F // 8A,  // AWB_KR_R <-0x0082
{0x098C, 0xA36A, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0084, I2C_16BIT}, // 82,  // AWB_KG_R
{0x098C, 0xA36B, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x0082, I2C_16BIT}, // 82,  // AWB_KB_R 

//[Gamma]
{0x098C, 0xAB37, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
{0x0990, 0x0001, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2B38, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMASTARTMORPH]
{0x0990, 0x1000, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2B3A, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMASTOPMORPH]
{0x0990, 0x2000, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB3C, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_0]
{0x0990, 0x0000, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB3D, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_1]
{0x0990, 0x000A, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB3E, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_2]
{0x0990, 0x001C, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB3F, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_3]
{0x0990, 0x0036, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB40, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_4]
{0x0990, 0x0056, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB41, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_5]
{0x0990, 0x0070, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB42, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_6]
{0x0990, 0x0086, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB43, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_7]
{0x0990, 0x0099, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB44, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_8]
{0x0990, 0x00A9, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB45, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_9]
{0x0990, 0x00B6, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB46, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_10]
{0x0990, 0x00C1, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB47, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_11]
{0x0990, 0x00CC, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB48, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_12]
{0x0990, 0x00D5, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB49, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_13]
{0x0990, 0x00DE, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB4A, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_14]
{0x0990, 0x00E5, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB4B, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_15]
{0x0990, 0x00EC, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB4C, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_16]
{0x0990, 0x00F3, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB4D, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_17]
{0x0990, 0x00F9, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xAB4E, I2C_16BIT}, // MCU_ADDRESS [HG_GAMMA_TABLE_A_18]
{0x0990, 0x00FF, I2C_16BIT}, // MCU_DATA_0                               

//[LL(Low Light) setting & NR(Noise Reduction)] 
{0x098C, 0xAB1F, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x00C6, I2C_16BIT}, // RESERVED_HG_1F                         
{0x098C, 0xAB20, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0060, I2C_16BIT}, // RESERVED_HG_20(maximum saturation)(080731) 80->43                        
{0x098C, 0xAB21, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x001F, I2C_16BIT}, // RESERVED_HG_21

//Origin
/*
{0x098C, 0xAB22, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0006, I2C_16BIT}, // 4,  //3,  // RESERVED_HG_22                         
{0x098C, 0xAB23, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0006, I2C_16BIT}, // 5,  // RESERVED_HG_23                         
*/

//sharpen and noise down
{0x098C, 0xAB22, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0004, I2C_16BIT}, //6 // 4,  //3,  // RESERVED_HG_22                         
{0x098C, 0xAB23, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0005, I2C_16BIT}, //6 // 5,  // RESERVED_HG_23    


{0x098C, 0xAB24, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0030, I2C_16BIT}, // RESERVED_HG_24(minimum saturation)<-0x0030 (080731) 10->00                       
{0x098C, 0xAB25, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0035, I2C_16BIT}, // RESERVED_HG_25(noise filter)<-0x0014                      
{0x098C, 0xAB26, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0000, I2C_16BIT}, // RESERVED_HG_26                         
{0x098C, 0xAB27, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0006, I2C_16BIT}, // RESERVED_HG_27                         
{0x098C, 0x2B28, I2C_16BIT}, // MCU_ADDRESS 
{0x0990, 0x1800, I2C_16BIT}, // HG_LL_BRIGHTNESSSTART <-0x1388                         
{0x098C, 0x2B2A, I2C_16BIT}, // MCU_ADDRESS  
{0x0990, 0x3000, I2C_16BIT}, // HG_LL_BRIGHTNESSSTOP <-0x4E20                        
{0x098C, 0xAB2C, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0006, I2C_16BIT}, // RESERVED_HG_2C                       
{0x098C, 0xAB2D, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x000A, I2C_16BIT}, // RESERVED_HG_2D                         
{0x098C, 0xAB2E, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0006, I2C_16BIT}, // RESERVED_HG_2E                         
{0x098C, 0xAB2F, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0006, I2C_16BIT}, // RESERVED_HG_2F                       
{0x098C, 0xAB30, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x001E, I2C_16BIT}, // RESERVED_HG_30                         
{0x098C, 0xAB31, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x000E, I2C_16BIT}, // RESERVED_HG_31                        
{0x098C, 0xAB32, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x001E, I2C_16BIT}, // RESERVED_HG_32                    
{0x098C, 0xAB33, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x001E, I2C_16BIT}, // RESERVED_HG_33                      
{0x098C, 0xAB34, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0008, I2C_16BIT}, // RESERVED_HG_34                     
{0x098C, 0xAB35, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x0080, I2C_16BIT}, // RESERVED_HG_35

//[AE WINDOW SIZE POS CHANGE-CENTER]window wider 080827
{0x098C, 0xA202, I2C_16BIT}, // MCU_ADDRESS [AE_WINDOW_POS]  (080731) AE window change
{0x0990, 0x0021, I2C_16BIT}, // MCU_DATA_0, 0x0043
{0x098C, 0xA203, I2C_16BIT}, // MCU_ADDRESS [AE_WINDOW_SIZE]
{0x0990, 0x00dd, I2C_16BIT}, // MCU_DATA_0, 0x00B9
                                                          
//ae_settings
{0x098C, 0xA11D, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0002, I2C_16BIT}, // SEQ_PREVIEW_1_AE                                                                                                                 
{0x098C, 0xA208, I2C_16BIT},                              
{0x0990, 0x0003, I2C_16BIT}, // 080723  AE speed 조정  0x0004  (080731 speed 재조정 1->3)
{0x098C, 0xA209, I2C_16BIT},                              
{0x0990, 0x0002, I2C_16BIT},                              
{0x098C, 0xA20A, I2C_16BIT},                              
{0x0990, 0x001F, I2C_16BIT},                              
{0x098C, 0xA216, I2C_16BIT},                              
{0x0990, 0x003A, I2C_16BIT},
	
//drt_off
{0x098C, 0xA244, I2C_16BIT}, // MCU_ADDRESS                
{0x0990, 0x0008, I2C_16BIT}, // RESERVED_AE_44      

//base_target
{0x098C, 0xA24F, I2C_16BIT}, // MCU_ADDRESS  
{0x0990, 0x0042, I2C_16BIT}, // AE_BASETARGET
{0x098C, 0xA207, I2C_16BIT}, // MCU_ADDRESS                                
{0x0990, 0x0006, I2C_16BIT}, // AE_GATE              Change to 0xA->0x06 from Sensor           
{0x098C, 0xA20D, I2C_16BIT}, // MCU_ADDRESS                               
{0x0990, 0x0020, I2C_16BIT}, // AE_MinVirtGain(minimum allowed virtual gain)                       
{0x098C, 0xA20E, I2C_16BIT}, // MCU_ADDRESS  // 080723 저조도 Gain
{0x0990, 0x0080, I2C_16BIT}, // a0->80  AE_MaxVirtGain(maximum allowed virtual gain)
{0x098C, 0xAB04, I2C_16BIT}, // MCU_ADDRESS                       
{0x0990, 0x0014, I2C_16BIT},                              
{0x098C, 0x2361, I2C_16BIT}, // protect the WB hunting                               
{0x0990, 0x0a00, I2C_16BIT}, // <-0x00X0                          
{0x3244, 0x0307, I2C_16BIT}, //0x0310(3.28)
{0x098C, 0x2212, I2C_16BIT}, // MCU_ADDRESS
{0x0990, 0x0120, I2C_16BIT}, //F0,  // RESERVED_AE_12(default:0x0080)
                                                                                                       
//[Edge]                                                                                              
{0x326C, 0x1305, I2C_16BIT},

// KJK. Format Conversion
{0x098C, 0x2755, I2C_16BIT}, // MCU_ADDRESS [MODE_OUTPUT_FORMAT_A] Preview / Default : CbYCrY
{0x0990, 0x0002, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2757, I2C_16BIT}, // MCU_ADDRESS [MODE_OUTPUT_FORMAT_B] Capture / Default : CbYCrY
{0x0990, 0x0002, I2C_16BIT}, // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg VT_core_settings[]=
{
//[lolight brightness]
{0x098C, 0x2212, I2C_16BIT}, // MCU_ADDRESS
{0x0990, 0x0200, I2C_16BIT}, // 120 //F0, // RESERVED_AE_12(default:0x0080)
{0x098C, 0xA20E, I2C_16BIT}, // MCU_ADDRESS  // 080723 저조도 Gain
{0x0990, 0x00B0, I2C_16BIT}, // a0->80  AE_MaxVirtGain(maximum allowed virtual gain)

//[15Fps Fixed] Real 14.7
{0x098C, 0x271F, I2C_16BIT}, // Frame Lines (A) 15FPS
{0x0990, 0x01FB, I2C_16BIT}, //      = 556                    
{0x098C, 0x2721, I2C_16BIT}, // Line Length (A) 15FPS       
{0x0990, 0x0629, I2C_16BIT}, //      = 842
{0x098C, 0xA20B, I2C_16BIT}, // MCU_ADDRESS [AE_MIN_INDEX]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xA20C, I2C_16BIT}, // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

// 15FPS Fixed, No Flip & Mirror
const static struct mt9v113_reg VR_core_settings[] =
{

//{0x098C, 0x2717, I2C_16BIT}, // Read Mode (A)***		//Removed by MMS
//{0x0990, 0x0024, I2C_16BIT}, // default(0x0024)

//{0x098C, 0x272D, I2C_16BIT}, // Read Mode (B)***              
//{0x0990, 0x0024, I2C_16BIT}, // default(0x0024)      

//[15Fps Fixed]
{0x098C, 0x271F, I2C_16BIT}, // Frame Lines (A) 15FPS
{0x0990, 0x01FB, I2C_16BIT}, //      = 556                    
{0x098C, 0x2721, I2C_16BIT}, // Line Length (A) 15FPS       
{0x0990, 0x0629, I2C_16BIT}, //      = 842
{0x098C, 0xA20B, I2C_16BIT}, // MCU_ADDRESS [AE_MIN_INDEX]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xA20C, I2C_16BIT}, // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Normal_core_settings[]={

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2011.09.22] - CAM
// No Flip & Mirror
//{0x098C, 0x2717, I2C_16BIT}, // Read Mode (A)***		//Removed by MMS
//{0x0990, 0x0024, I2C_16BIT}, // default(0x0024)

//{0x098C, 0x272D, I2C_16BIT}, // Read Mode (B)***              
//{0x0990, 0x0024, I2C_16BIT}, // default(0x0024)   
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2011.09.22] - CAM

//[Normal mode : support 10~22fps]
{0x098C, 0x271F , I2C_16BIT}, // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x01FB , I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2721 , I2C_16BIT}, // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x0433 , I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xA20C , I2C_16BIT}, // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x000C , I2C_16BIT}, // 0x000A // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

/////////////////////////////
//       BrightNess
/////////////////////////////
const static struct mt9v113_reg Brightness1[]={
//CAMERA_BRIGHTNESS_MINUS_6
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0010, I2C_16BIT},

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
  
const static struct mt9v113_reg Brightness2[]={
//CAMERA_BRIGHTNESS_MINUS_5
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0018, I2C_16BIT},  // MMS 0727 : 0x0021->0x001E

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Brightness3[]={
//CAMERA_BRIGHTNESS_MINUS_4
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0020, I2C_16BIT},  // MMS 0727 : 0x0028->0x0022

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Brightness4[]={
//CAMERA_BRIGHTNESS_MINUS_3
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0028, I2C_16BIT},  // MMS 0727 : 0x002F->0x0026

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Brightness5[]={ 
//CAMERA_BRIGHTNESS_MINUS_2
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0030, I2C_16BIT},  // MMS 0727 : 0x0035->0x002A

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Brightness6[]={
//CAMERA_BRIGHTNESS_MINUS_1
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0038, I2C_16BIT},  // MMS 0727 : 0x0035->0x002E

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness7[]={
//CAMERA_BRIGHTNESS_NORMAL
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0042, I2C_16BIT},  // MMS 0727 : 0x0042->0x0032

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness8[]={
//CAMERA_BRIGHTNESS_PLUS_1
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x004A, I2C_16BIT},  // MMS 0727 : 0x0049->0x0036

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness9[]={
//CAMERA_BRIGHTNESS_PLUS_2
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0052, I2C_16BIT},  // MMS 0727 : 0x0051->0x003A

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness10[]={
//CAMERA_BRIGHTNESS_PLUS_3
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x005A, I2C_16BIT},  // MMS 0727 : 0x0059->0x003E

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness11[]={
//CAMERA_BRIGHTNESS_PLUS_4
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0062, I2C_16BIT},  // MMS 0727 : 0x0061->0x0042

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness12[]={
//CAMERA_BRIGHTNESS_PLUS_5
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x006A, I2C_16BIT},  // MMS 0727 : 0x0069->0x0046

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Brightness13[]={
//CAMERA_BRIGHTNESS_PLUS_6
{0x098C, 0xA24F, I2C_16BIT},
{0x0990, 0x0072, I2C_16BIT},  // MMS 0727 : 0x0070->0x004A

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

////////////////////////////////
// Color Mode
///////////////////////////////
const static struct mt9v113_reg Color_Normal[]={
//CAMERA_EFFECT_OFF                    
{0x098C, 0x2759, I2C_16BIT}, // MCU_ADDRESS
{0x0990, 0x6440, I2C_16BIT}, // MODE_SPEC_EFFECTS_A  
{0x098C, 0x275B, I2C_16BIT}, // MCU_ADDRESS  
{0x0990, 0x6440, I2C_16BIT}, // MODE_SPEC_EFFECTS_B  

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Color_Mono[]={
//CAMERA_EFFECT_MONO
{0x098C, 0x2759, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x6441, I2C_16BIT}, // MODE_SPEC_EFFECTS_A
{0x098C, 0x275B, I2C_16BIT}, // MCU_ADDRESS        
{0x0990, 0x6441, I2C_16BIT}, // MODE_SPEC_EFFECTS_B

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
const static struct mt9v113_reg Color_Sepia[]={
//CAMERA_EFFECT_SEPIA
{0x098C, 0x2763, I2C_16BIT}, // MCU_ADDRESS             
{0x0990, 0xE814, I2C_16BIT}, //<-0xF414                 
{0x098C, 0x2759, I2C_16BIT}, // MCU_ADDRESS             
{0x0990, 0x6442, I2C_16BIT}, // MODE_SPEC_EFFECTS_A     
{0x098C, 0x275B, I2C_16BIT}, // MCU_ADDRESS             
{0x0990, 0x6442, I2C_16BIT}, // MODE_SPEC_EFFECTS_B     

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Color_Negative[]={
//CAMERA_EFFECT_NEGATIVE
{0x098C, 0x2759, I2C_16BIT}, // MCU_ADDRESS         
{0x0990, 0x6443, I2C_16BIT}, // MODE_SPEC_EFFECTS_A 
{0x098C, 0x275B, I2C_16BIT}, // MCU_ADDRESS         
{0x0990, 0x6443, I2C_16BIT}, // MODE_SPEC_EFFECTS_B 

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Color_Solarize[]={
//CAMERA_EFFECT_Solarization
{0x098C, 0x2759, I2C_16BIT},  // MCU_ADDRESS [MODE_SPEC_EFFECTS_A]
{0x0990, 0x4E44, I2C_16BIT},  // MCU_DATA_0
{0x098C, 0x275B, I2C_16BIT},  // MCU_ADDRESS [MODE_SPEC_EFFECTS_B]
{0x0990, 0x4E44, I2C_16BIT},  // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};


const static struct mt9v113_reg AWB_auto[]={
//CAMERA_PARAM_BALANCE_AUTO:
{0x098C, 0xA102, I2C_16BIT},  // MCU_ADDRESS
{0x0990, 0x000F, I2C_16BIT},  // Mode(AWB/Flicker/AE driver enable)
{0x098C, 0xA34A, I2C_16BIT},  // MCU_ADDRESS      
{0x0990, 0x0059, I2C_16BIT},  // AWB_GAINMIN_R                        
{0x098C, 0xA34B, I2C_16BIT},  // MCU_ADDRESS       
{0x0990, 0x00E6, I2C_16BIT},  // AWB_GAINMAX_R                        
{0x098C, 0xA34C, I2C_16BIT},  // MCU_ADDRESS      
{0x0990, 0x0059, I2C_16BIT},  // AWB_GAINMIN_B                        
{0x098C, 0xA34D, I2C_16BIT},  // MCU_ADDRESS     
{0x0990, 0x00A6, I2C_16BIT},  // AWB_GAINMAX_B                        
{0x098C, 0xA351, I2C_16BIT},  // MCU_ADDRESS 
{0x0990, 0x0020, I2C_16BIT},  // AWB_CCM_POSITION_MIN                       
{0x098C, 0xA352, I2C_16BIT},  // MCU_ADDRESS
{0x0990, 0x007F, I2C_16BIT},  // AWB_CCM_POSITION_MAX
{0x098C, 0xA365, I2C_16BIT},  // MCU_ADDRESS 
{0x0990, 0x0000, I2C_16BIT},  // MCU_DATA_0, [AWB_X0]
{0x098C, 0xA369, I2C_16BIT},  // MCU_ADDRESS 
{0x0990, 0x0083, I2C_16BIT},  // 8A // MCU_DATA_0, [AWB_KR_R]
{0x098C, 0xA36B, I2C_16BIT},  // MCU_ADDRESS 
{0x0990, 0x0082, I2C_16BIT},  // MCU_DATA_0, [AWB_KB_R]

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg AWB_sun[]={
//CAMERA_PARAM_BALANCE_SUN: 6500K
/* AWB off */
{0x098C, 0xA102, I2C_16BIT},   // MCU_ADDRESS
{0x0990, 0x000B, I2C_16BIT},   // Mode(AWB disable)   
{0x098C, 0xA34A, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MIN]    
{0x0990, 0x00B0, I2C_16BIT},   // MCU_DATA_0   
{0x098C, 0xA34B, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MAX]    
{0x0990, 0x00B0, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA34C, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMIN_B]    
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA34D, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMAX_B]   
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA351, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MIN]    
{0x0990, 0x004B, I2C_16BIT},   // MCU_DATA_0   
{0x098C, 0xA352, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MAX]  
{0x0990, 0x004B, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA353, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION]    
{0x0990, 0x004B, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA369, I2C_16BIT},   // MCU_ADDRESS [AWB_KR_R]    
{0x0990, 0x00C8, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA36B, I2C_16BIT},   // MCU_ADDRESS [AWB_KB_R]    
{0x0990, 0x007F, I2C_16BIT},   // AWB_CCM_POSITION 

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};                                   
                                     
const static struct mt9v113_reg AWB_glow[]={
//CAMERA_PARAM_BALANCE_GLOW: 2850K
/* AWB off */

{0x098C, 0xA102, I2C_16BIT},   // MCU_ADDRESS
{0x0990, 0x000B, I2C_16BIT},   // Mode(AWB disable)
{0x098C, 0xA34A, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MIN]    
{0x0990, 0x0090, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA34B, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MAX]    
{0x0990, 0x0090, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA34C, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMIN_B]    
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA34D, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMAX_B]    
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA351, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MIN]    
{0x0990, 0x0000, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA352, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MAX]    
{0x0990, 0x0000, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA353, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION]    
{0x0990, 0x0000, I2C_16BIT},   // MCU_DATA_0   
{0x098C, 0xA369, I2C_16BIT},   // MCU_ADDRESS [AWB_KR_R]    
{0x0990, 0x00D4, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA36B, I2C_16BIT},   // MCU_ADDRESS [AWB_KB_R]    
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0    

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};                                   
                                     
const static struct mt9v113_reg AWB_fluorescent[]={
//CAMERA_PARAM_BALANCE_FLUORESCENT:  4500K
/* AWB off */

{0x098C, 0xA102, I2C_16BIT},   // MCU_ADDRESS
{0x0990, 0x000B, I2C_16BIT},   // Mode(AWB disable) 
{0x098C, 0xA34A, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MIN]
{0x0990, 0x0092, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA34B, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MAX]
{0x0990, 0x0092, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA34C, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMIN_B]
{0x0990, 0x0096, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA34D, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMAX_B]
{0x0990, 0x0096, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA351, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MIN]
{0x0990, 0x001F, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA352, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MAX]
{0x0990, 0x001F, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA353, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION]
{0x0990, 0x001F, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA369, I2C_16BIT},   // MCU_ADDRESS [AWB_KR_R]
{0x0990, 0x00D4, I2C_16BIT},   // MCU_DATA_0
{0x098C, 0xA36B, I2C_16BIT},   // MCU_ADDRESS [AWB_KB_R]
{0x0990, 0x005D, I2C_16BIT},   // MCU_DATA_0    

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};                                   
                                     
const static struct mt9v113_reg AWB_cloudy[]={
//CAMERA_PARAM_BALANCE_CLOUDY: 7500K
/* AWB off */

{0x098C, 0xA102, I2C_16BIT},   // MCU_ADDRESS
{0x0990, 0x000B, I2C_16BIT},   // Mode(AWB disable) 
{0x098C, 0xA34A, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MIN]    
{0x0990, 0x00D0, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA34B, I2C_16BIT},   // MCU_ADDRESS [AWB_GAIN_MAX]    
{0x0990, 0x00D0, I2C_16BIT},   // MCU_DATA_0   
{0x098C, 0xA34C, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMIN_B]    
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA34D, I2C_16BIT},   // MCU_ADDRESS [AWB_GAINMAX_B]    
{0x0990, 0x0080, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA351, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MIN]   
{0x0990, 0x004B, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA352, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION_MAX]    
{0x0990, 0x004B, I2C_16BIT},   // MCU_DATA_0   
{0x098C, 0xA353, I2C_16BIT},   // MCU_ADDRESS [AWB_CCM_POSITION]    
{0x0990, 0x004B, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA369, I2C_16BIT},   // MCU_ADDRESS [AWB_KR_R]    
{0x0990, 0x00C8, I2C_16BIT},   // MCU_DATA_0    
{0x098C, 0xA36B, I2C_16BIT},   // MCU_ADDRESS [AWB_KB_R]    
{0x0990, 0x0083, I2C_16BIT},   // MCU_DATA_0     

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};    

const static struct mt9v113_reg Mirror_Normal[]={
// Default
{0x098C, 0x2717, I2C_16BIT},
{0x0990, 0x0024, I2C_16BIT},
{0x098C, 0x272D, I2C_16BIT},
{0x0990, 0x0024, I2C_16BIT},
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Horizontal_Mirror[]={
// Horizontal mirror
{0x098C, 0x2717, I2C_16BIT},
{0x0990, 0x0026, I2C_16BIT},
{0x098C, 0x272D, I2C_16BIT},
{0x0990, 0x0026, I2C_16BIT},
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Vertical_Flip[]={
// Vertical Flip
{0x098C, 0x2717, I2C_16BIT},
{0x0990, 0x0025, I2C_16BIT},
{0x098C, 0x272D, I2C_16BIT},
{0x0990, 0x0025, I2C_16BIT},
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Mirror_and_Flip[]={
// Horizontal mirror/Vertical Flip
{0x098C, 0x2717, I2C_16BIT},
{0x0990, 0x0027, I2C_16BIT},
{0x098C, 0x272D, I2C_16BIT},
{0x0990, 0x0027, I2C_16BIT},
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};



const static struct mt9v113_reg Night_mode_on[]={
//[Normal Night Mode : 7.5 ~ 22 Fps]
{0x098C, 0x271F, I2C_16BIT},  // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x01FB, I2C_16BIT},  // MCU_DATA_0
{0x098C, 0x2721, I2C_16BIT},  // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x0433, I2C_16BIT},  // MCU_DATA_0
{0x098C, 0xA20C, I2C_16BIT},  // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x0010, I2C_16BIT},  // 0x000F // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Night_mode_off[]={
//[Normal mode : support 10~22fps, Just FPS Only] Real 10 ~ 23
{0x098C, 0x271F , I2C_16BIT}, // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x01FB , I2C_16BIT}, // MCU_DATA_0
{0x098C, 0x2721 , I2C_16BIT}, // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x0433 , I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xA20C , I2C_16BIT}, // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x000C , I2C_16BIT}, // 0x000A // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};




const static struct mt9v113_reg VT_Night_mode_on[]={
//[VT Night Mode : 7.5FpsFixxed]
{0x098C, 0x271F, I2C_16BIT},  // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x01FB, I2C_16BIT},  // MCU_DATA_0
{0x098C, 0x2721, I2C_16BIT},  // MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
{0x0990, 0x0C53, I2C_16BIT},  // 0x093E // MCU_DATA_0
{0x098C, 0xA20B, I2C_16BIT},  // MCU_ADDRESS [AE_MIN_INDEX]
{0x0990, 0x0004, I2C_16BIT},  // MCU_DATA_0
{0x098C, 0xA20C, I2C_16BIT},  // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x0004, I2C_16BIT},  // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg VT_Night_mode_off[]={
//[15Fps Fixed, Just FPS Only] Real 14.7
{0x098C, 0x271F, I2C_16BIT}, // Frame Lines (A) 15FPS
{0x0990, 0x01FB, I2C_16BIT}, //      = 556                    
{0x098C, 0x2721, I2C_16BIT}, // Line Length (A) 15FPS       
{0x0990, 0x0629, I2C_16BIT}, //      = 842
{0x098C, 0xA20B, I2C_16BIT}, // MCU_ADDRESS [AE_MIN_INDEX]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0
{0x098C, 0xA20C, I2C_16BIT}, // MCU_ADDRESS [AE_MAX_INDEX]
{0x0990, 0x0004, I2C_16BIT}, // MCU_DATA_0

{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};




///// Not Used. /////

const static struct mt9v113_reg Exposure_mode_multi[]={
//AE MODE
//*Multi_default                                                              
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};         

const static struct mt9v113_reg Exposure_mode_cen[]={
//AE MODE
//*Center                                                              
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Flicker_60Hz[]={
//1. Auto Flicker 60Hz Start
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg Flicker_50Hz[]={
//1. Auto Flicker 50Hz Start
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg set_preview_vga_config[] =
{
//Insert by MMS	[START]
{0x098C,  0x2739, I2C_16BIT},
{0x0990,  0x0000, I2C_16BIT},
{0x098C,  0x273B, I2C_16BIT},
{0x0990,  0x027F, I2C_16BIT},
{0x098C,  0x273D, I2C_16BIT},
{0x0990,  0x0000, I2C_16BIT},
{0x098C,  0x273F, I2C_16BIT},
{0x0990,  0x01DF, I2C_16BIT},
{0x098C,  0x2703, I2C_16BIT},
{0x0990,  0x0280, I2C_16BIT},
{0x098C,  0x2705, I2C_16BIT},
{0x0990,  0x01E0, I2C_16BIT},
{0x098C,  0xA103, I2C_16BIT},
{0x0990,  0x0005, I2C_16BIT},
//Insert by MMS	[END]
//=================================================================================================
//  Set Preview Config
//=================================================================================================
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg set_preview_qvga_config[] =
{
//=================================================================================================
//  Set Preview Config
//=================================================================================================
//Insert by MMS	[START]
{0x098C,  0x2739, I2C_16BIT},
{0x0990,  0x0000, I2C_16BIT},
{0x098C,  0x273B, I2C_16BIT},
{0x0990,  0x027F, I2C_16BIT},
{0x098C,  0x273D, I2C_16BIT},
{0x0990,  0x0000, I2C_16BIT},
{0x098C,  0x273F, I2C_16BIT},
{0x0990,  0x01DF, I2C_16BIT},
{0x098C,  0x2703, I2C_16BIT},
{0x0990,  0x0140, I2C_16BIT},
{0x098C,  0x2705, I2C_16BIT},
{0x0990,  0x00F0, I2C_16BIT},
{0x098C,  0xA103, I2C_16BIT},
{0x0990,  0x0005, I2C_16BIT},
//Insert by MMS	[END]
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};

const static struct mt9v113_reg set_preview_qcif_config[] =
{
//Insert by MMS	[START]
{0x098C,  0x2739, I2C_16BIT},
{0x0990,  0x0000, I2C_16BIT},
{0x098C,  0x273B, I2C_16BIT},
{0x0990,  0x027F, I2C_16BIT},
{0x098C,  0x273D, I2C_16BIT},
{0x0990,  0x0000, I2C_16BIT},
{0x098C,  0x273F, I2C_16BIT},
{0x0990,  0x01DF, I2C_16BIT},
{0x098C,  0x2703, I2C_16BIT},
{0x0990,  0x0140, I2C_16BIT},
{0x098C,  0x2705, I2C_16BIT},
{0x0990,  0x0090, I2C_16BIT},
{0x098C,  0xA103, I2C_16BIT},
{0x0990,  0x0005, I2C_16BIT},
//Insert by MMS	[END]
{I2C_REG_TERM, I2C_VAL_TERM, I2C_LEN_TERM}
};
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr [2011.07.26] - CAM : from justin froyo