#ifndef EEPROM_ADR_H
#define EEPROM_ADR_H

#define EA_DAC_GAIN_COMP_POS_VOL 0x00
#define EA_DAC_GAIN_COMP_NEG_VOL 0x04
#define EA_ADC_GAIN_COMP_POS_VOL 0x08
#define EA_ADC_GAIN_COMP_NEG_VOL 0x012

#define EA_DAC_GAIN_COMP_POS_CUR 0x016
#define EA_DAC_GAIN_COMP_NEG_CUR 0x020



#define EA_ADC_GAIN_COMP_POS_CUR 0x024
#define EA_ADC_GAIN_COMP_NEG_CUR 0x028

#define EA_DAC_ZERO_COMP_VOL 0x032
#define EA_DAC_ZERO_COMP_CUR 0x036

#define EA_ADC_ZERO_COMP_VOL 0x040
// TODO: put EA_ADC_ZERO_COMP_VOL2 here     


#define EA_ADC_ZERO_COMP_CUR 0x044
#define EA_ADC_ZERO_COMP_CUR2 0x048

#define EA_DAC_GAIN_COMP_POS_LIM 0x52
#define EA_DAC_GAIN_COMP_NEG_LIM 0x56

#define EA_DAC_GAIN_COMP_POS_VOL2 0x060
#define EA_DAC_GAIN_COMP_NEG_VOL2 0x064
#define EA_DAC_GAIN_COMP_POS_CUR2 0x068
#define EA_DAC_GAIN_COMP_NEG_CUR2 0x072


#define EA_ADC_GAIN_COMP_POS_VOL2 0x076
#define EA_ADC_GAIN_COMP_NEG_VOL2 0x080
#define EA_ADC_GAIN_COMP_POS_CUR2 0x084
#define EA_ADC_GAIN_COMP_NEG_CUR2 0x088

#define EA_DAC_ZERO_COMP_VOL2 0x092
#define EA_DAC_ZERO_COMP_CUR2 0x096

// WARNING:  No more addresses between. Must change some if we new addresses. Will mean incompatibilities...
//           Suggest moving nonlinearities to 0x200 and then recalibrate... 


#define EA_ADC_NONLINEAR_COMP_NR_VOL 0x100
#define EA_ADC_NONLINEAR_COMP_START_VOL 0x0104

//TODO: Put in right place 0x44. but need to shift other keys..
#define EA_ADC_ZERO_COMP_VOL2 0x0108

// Addresses for EEPROMAnything
#define EA_ANYTHING 0x200


#define EA_ADC_NONLINEAR_COMP_NR_CUR 0x500
#define EA_ADC_NONLINEAR_COMP_START_CUR 0x504

#endif