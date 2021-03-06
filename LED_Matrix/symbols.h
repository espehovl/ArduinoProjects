#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#define test_width 10
#define test_height 10
static unsigned char test_bits[] = {
   0x33, 0x03, 0x01, 0x02, 0xcc, 0x00, 0x84, 0x00, 0x31, 0x02, 0x31, 0x02,
   0x84, 0x00, 0xcc, 0x00, 0x01, 0x02, 0x33, 0x03 };


#define timini_width 10
#define timini_height 10
static unsigned char timini_bits[] = {
   0x80, 0x03, 0xb0, 0x02, 0x80, 0x03, 0x3b, 0x02, 0xaa, 0x03, 0x2a, 0x00,
   0x2a, 0x00, 0x2a, 0x00, 0x3a, 0x00, 0x00, 0x00 };

#define error_width 10
#define error_height 10
static unsigned char error_bits[] = {
   0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
   0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00 };


#define wifi_width 10
#define wifi_height 10
static unsigned char wifi_bits[] = {
   0x03, 0x00, 0x1e, 0x00, 0x30, 0x00, 0x60, 0x00, 0xc7, 0x00, 0x8c, 0x01,
   0x18, 0x01, 0x33, 0x01, 0x26, 0x03, 0x25, 0x02 };


#define comms_width 10
#define comms_height 10
static unsigned char comms_bits[] = {
   0x48, 0x00, 0xc8, 0x00, 0xc8, 0x01, 0x48, 0x03, 0x48, 0x00, 0x48, 0x00,
   0x4b, 0x00, 0x4e, 0x00, 0x4c, 0x00, 0x48, 0x00 };


#define water_width 10
#define water_height 10
static unsigned char water_bits[] = {
   0x30, 0x00, 0x30, 0x00, 0xfe, 0x01, 0x00, 0x00, 0xaa, 0x00, 0xaa, 0x00,
   0x00, 0x00, 0x54, 0x01, 0x54, 0x01, 0x00, 0x00 };


#define heart_width 10
#define heart_height 10
static unsigned char heart_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0xce, 0x01, 0xff, 0x03, 0xfe, 0x01,
   0xfc, 0x00, 0x78, 0x00, 0x30, 0x00, 0x00, 0x00 };

#define one_width 10
#define one_height 10
static unsigned char one_bits[] = {
   0x00, 0x00, 0x30, 0x00, 0x38, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
   0x30, 0x00, 0x30, 0x00, 0x78, 0x00, 0x00, 0x00 };


#define two_width 10
#define two_height 10
static unsigned char two_bits[] = {
   0x00, 0x00, 0x78, 0x00, 0xcc, 0x00, 0x84, 0x00, 0xc0, 0x00, 0x70, 0x00,
   0x18, 0x00, 0x0c, 0x00, 0xfc, 0x00, 0x00, 0x00 };


#define three_width 10
#define three_height 10
static unsigned char three_bits[] = {
   0x00, 0x00, 0x78, 0x00, 0xcc, 0x00, 0x80, 0x00, 0x70, 0x00, 0xc0, 0x00,
   0x80, 0x00, 0xcc, 0x00, 0x78, 0x00, 0x00, 0x00 };


#define four_width 10
#define four_height 10
static unsigned char four_bits[] = {
   0x00, 0x00, 0x08, 0x00, 0x48, 0x00, 0x48, 0x00, 0x48, 0x00, 0x78, 0x00,
   0x40, 0x00, 0x40, 0x00, 0xf0, 0x00, 0x00, 0x00 };


#define five_width 10
#define five_height 10
static unsigned char five_bits[] = {
   0x00, 0x00, 0x7c, 0x00, 0x04, 0x00, 0x04, 0x00, 0x7c, 0x00, 0xc0, 0x00,
   0x80, 0x00, 0xcc, 0x00, 0x78, 0x00, 0x00, 0x00 };


#define six_width 10
#define six_height 10
static unsigned char six_bits[] = {
   0x00, 0x00, 0x78, 0x00, 0xcc, 0x00, 0x04, 0x00, 0x7c, 0x00, 0xcc, 0x00,
   0x84, 0x00, 0xcc, 0x00, 0x78, 0x00, 0x00, 0x00 };


#define seven_width 10
#define seven_height 10
static unsigned char seven_bits[] = {
   0x00, 0x00, 0xf8, 0x00, 0x80, 0x00, 0xc0, 0x00, 0x60, 0x00, 0x20, 0x00,
   0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00 };


#define eight_width 10
#define eight_height 10
static unsigned char eight_bits[] = {
   0x00, 0x00, 0x78, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0x78, 0x00, 0xcc, 0x00,
   0x84, 0x00, 0xcc, 0x00, 0x78, 0x00, 0x00, 0x00 };


#define nine_width 10
#define nine_height 10
static unsigned char nine_bits[] = {
   0x00, 0x00, 0x78, 0x00, 0xcc, 0x00, 0x84, 0x00, 0xcc, 0x00, 0xf8, 0x00,
   0x80, 0x00, 0xcc, 0x00, 0x78, 0x00, 0x00, 0x00 };


#define zero_width 10
#define zero_height 10
static unsigned char zero_bits[] = {
   0x00, 0x00, 0x78, 0x00, 0xcc, 0x00, 0x84, 0x00, 0x84, 0x00, 0x84, 0x00,
   0x84, 0x00, 0xcc, 0x00, 0x78, 0x00, 0x00, 0x00 };

#endif