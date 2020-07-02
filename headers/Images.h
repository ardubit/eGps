// Do not declare functions and objects here. It's only for PROGMEM
// PROGMEM store date into Flash memory

#ifndef IMAGES_H
#define IMAGES_H

#include <Arduino.h>
#include "Debug.h"
#include <pgmspace.h>

#define IMG_H 8
#define IMG_W 8
#define TIMES 3

// Full, half and low battery
PROGMEM const unsigned char BATTERY[][8] = {
    /* 8, 8, */
    {B11111000,
     B11111000,
     B11111110,
     B11111110,
     B11111110,
     B11111000,
     B11111000,
     B00000000},
    {/* 8, 8, */
     B11111000,
     B11001000,
     B11101110,
     B11110010,
     B11111110,
     B11111000,
     B11111000,
     B00000000},
    {/* 8, 8, */
     B11111000,
     B10001000,
     B11001110,
     B11100010,
     B11111110,
     B11111000,
     B11111000,
     B00000000},
    {/* 8, 8, */
     B11111000,
     B10001000,
     B10001110,
     B11000010,
     B11101110,
     B11111000,
     B11111000,
     B00000000},
    {/* 8, 8, */
     B11111000,
     B10001000,
     B10001110,
     B10000010,
     B11001110,
     B11101000,
     B11111000,
     B00000000},
    {/* 8, 8, */
     B11111000,
     B10001000,
     B10001110,
     B10000010,
     B10001110,
     B10001000,
     B11111000,
     B00000000},
    {/* 8, 8, */
     B10101000,
     B00000000,
     B10101010,
     B00000000,
     B10101010,
     B00000000,
     B10101000,
     B00000000}};

// Running man
PROGMEM const unsigned char RUNNING_MAN_RIGHT[][8] = {
    {/* 8, 8, */
     B00011000,
     B01011000,
     B10110010,
     B00111100,
     B01010000,
     B11001000,
     B10001100,
     B00000000},
    {/* 8, 8, */
     B00011000,
     B00011000,
     B01110000,
     B01111000,
     B00111000,
     B00110000,
     B00100000,
     B00000000},
    {/* 8, 8, */
     B00011000,
     B00011000,
     B01110000,
     B10111010,
     B00111100,
     B01101000,
     B01001100,
     B00000000}};

// Victory man
PROGMEM const unsigned char VICTORY_MAN[][8] = {
    /* 8, 8, */
    {B00110000,
     B00110010,
     B00011110,
     B11111100,
     B00111000,
     B00011000,
     B00011000,
     B00000000},
    {/* 8, 8, */
     B00110010,
     B00110010,
     B10011100,
     B01111100,
     B00111000,
     B00011000,
     B00011000,
     B00000000},
    {/* 8, 8, */
     B00110010,
     B00110010,
     B10011100,
     B01111100,
     B00111000,
     B00011000,
     B00011000,
     B00000000}};

// Satellite
PROGMEM const unsigned char SATELLITE[] = {
    /* 8, 8, */
    B10000010,
    B10010010,
    B10111010,
    B11111110,
    B10111010,
    B10010010,
    B10000010,
    B00000000};

// Earth
PROGMEM const unsigned char EARTH[] = {
    /* 8, 8, */
    B00111000,
    B01000100,
    B10001110,
    B10011110,
    B10000010,
    B01110100,
    B00111000,
    B00000000};

// Recording
PROGMEM const unsigned char REC[][8] = {
    {/* 8, 8, */
     B00111000,
     B01111100,
     B11111110,
     B11111110,
     B11111110,
     B01111100,
     B00111000,
     B00000000},
    {/* 8, 8, */
     B00111000,
     B01000100,
     B10000010,
     B10000010,
     B10000010,
     B01000100,
     B00111000,
     B00000000}};

// Target
PROGMEM const unsigned char TARGET[] = {
    /* 8, 8, */
    B00111000,
    B01010100,
    B10010010,
    B11111110,
    B10010010,
    B01010100,
    B00111000,
    B00000000};

// Lock
PROGMEM const unsigned char LOCK[] = {
    /* 8, 8, */
    B00111000,
    B01000100,
    B01000100,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B00000000};

// Flag
PROGMEM const unsigned char FLAG[] = {
    /* 8, 8, */
    B11111110,
    B11111100,
    B11111000,
    B11111100,
    B11111110,
    B10000000,
    B10000000,
    B00000000};

// Arrow up
PROGMEM const unsigned char ARROW_UP[] = {
    /* 8, 8, */
    B00010000,
    B00111000,
    B01010100,
    B10010010,
    B00010000,
    B11111110,
    B11111110,
    B00000000};

// Arrow right
PROGMEM const unsigned char ARROW_RIGHT[] = {
    /* 8, 8, */
    B11010000,
    B11001000,
    B11000100,
    B11111110,
    B11000100,
    B11001000,
    B11010000,
    B00000000};

// Arrow down
PROGMEM const unsigned char ARROW_DOWN[] = {
    /* 8, 8, */
    B11111110,
    B11111110,
    B00010000,
    B10010010,
    B01010100,
    B00111000,
    B00010000,
    B00000000};

// Arrow left
PROGMEM const unsigned char ARROW_LEFT[] = {
    /* 8, 8, */
    B00010110,
    B00100110,
    B01000110,
    B11111110,
    B01000110,
    B00100110,
    B00010110,
    B00000000};

// Pointer
PROGMEM const unsigned char POINTER[] = {
    /* 8, 8, */
    B00010000,
    B00010000,
    B00111000,
    B00111000,
    B01111100,
    B01111100,
    B11111110,
    B00000000};

// Error
PROGMEM const unsigned char STATUS_ERR[] = {
    /* 8, 8, */
    B10000010,
    B11000110,
    B01101100,
    B00111000,
    B01101100,
    B11000110,
    B10000010,
    B00000000};

// Error
PROGMEM const unsigned char STATUS_QUESTION[] = {
    /* 8, 8, */
    B11111110,
    B11000110,
    B10111010,
    B11111010,
    B11110110,
    B11101110,
    B11101110,
    B00000000};

// Ok
PROGMEM const unsigned char STATUS_OK[] = {
    /* 8, 8, */
    B00000000,
    B00000010,
    B00000110,
    B10001100,
    B11011000,
    B01110000,
    B00100000,
    B00000000};

// WiFi router
PROGMEM const unsigned char WIFI_ROUTER[] = {
    /* 8, 8, */
    B00000000,
    B10010010,
    B01010100,
    B00111000,
    B11111110,
    B11111110,
    B11111110,
    B00000000};

// Speed
PROGMEM const unsigned char SPEED[] = {
    /* 8, 8, */
    B01110000,
    B10111000,
    B01011100,
    B10101110,
    B01011100,
    B10111000,
    B01110000,
    B00000000};

// Tempo
PROGMEM const unsigned char TEMPO[] = {
    /* 8, 8, */
    B01010000,
    B10100000,
    B01010000,
    B10100000,
    B11111000,
    B11111100,
    B11111110,
    B00000000};

// Latitude
PROGMEM const unsigned char LAT[] = {
    /* 8, 8, */
    B00000000,
    B00101000,
    B01000100,
    B11111110,
    B01000100,
    B00101000,
    B00000000,
    B00000000};

// Longitude
PROGMEM const unsigned char LNG[] = {
    /* 8, 8, */
    B00010000,
    B00111000,
    B01010100,
    B00010000,
    B01010100,
    B00111000,
    B00010000,
    B00000000};

// Time
PROGMEM const unsigned char TIME[] = {
    /* 8, 8, */
    B11111110,
    B11010110,
    B10010010,
    B10011010,
    B10000010,
    B11000110,
    B11111110,
    B00000000};

// Distance
PROGMEM const unsigned char DISTANCE[] = {
    /* 8, 8, */
    B11110000,
    B11100000,
    B11100000,
    B10010010,
    B00001110,
    B00001110,
    B00011110,
    B00000000};

// Date
PROGMEM const unsigned char DATE[] = {
    /* 8, 8, */
    B01000100,
    B11111110,
    B11111110,
    B10000010,
    B10111010,
    B10000010,
    B11111110,
    B00000000};

// File
PROGMEM const unsigned char FILES[] = {
    /* 8, 8, */
    B11111000,
    B11100100,
    B11100010,
    B11100010,
    B11111110,
    B11111110,
    B11111110,
    B00000000};

// Altitude
PROGMEM const unsigned char ALTITUDE[] = {
    /* 8, 8, */
    B00000100,
    B00001110,
    B00000100,
    B00010000,
    B00101000,
    B01111100,
    B11111110,
    B00000000};

/* 
    {
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B11111110,
    B00000000};
*/

#endif