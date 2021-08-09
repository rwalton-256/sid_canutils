/**
 * @file sid_canutils.cpp
 * @author Ryan Walton
 * @brief 
 * @date 2021-08-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <cstring>
#include <assert.h>
#include <iostream>

#include <socketcan_wrapper/can_debug.hpp>
#include <udoo_canutils/sid_canutils.hpp>

union text_order_byte_t
{
    struct
    {
        uint8_t ord0:1;
        uint8_t ord1:1;
        uint8_t ord2:1;
        uint8_t res1:3;
        uint8_t new_msg:1;
        uint8_t res0:1;
    } bits;
    uint8_t raw;
};

union text_row_byte_t
{
    struct
    {
        uint8_t row0:1;
        uint8_t row1:1;
        uint8_t res:5;
        uint8_t changed:1;
    } bits;
    uint8_t raw;
};

union text_msg_t
{
    struct
    {
    	text_order_byte_t order;
	    uint8_t res;
    	text_row_byte_t row;
	    uint8_t letters[5];
    } bytes;
    uint8_t raw[8];
};

/**
 * @brief Manipulates text msg data type according to flags
 * 
 * @param msg_ptr Pointer to destination msg data
 * @param new_msg Flag indicating if this is the first message in a series of messages
 * @param msg_order Order of messages in series, value should be n for the first message and 0 for the last message
 * @param top_row Flag indicating if the message is to print to the top or bottom row of the SID
 * @param str Pointer to cstring characters to add to message
 * @param len Number of characters to add to message
 */
static void populate_text_msg( text_msg_t* msg_ptr, bool new_msg, uint8_t msg_order, bool top_row, const char* str, size_t len )
{
    msg_ptr->bytes.order.bits.new_msg = new_msg;

    msg_ptr->bytes.order.raw |= msg_order;

    msg_ptr->bytes.res = 0x96;

    msg_ptr->bytes.row.bits.changed = 1;

    if( top_row )
    {
        msg_ptr->bytes.row.bits.row0 = 1;
    }
    else
    {
        msg_ptr->bytes.row.bits.row1 = 1;
    }

    assert( len <= 5 );

    memcpy( msg_ptr->bytes.letters, str, len );
}

/**
 * @brief Sends upper and lower strings to SID as if they were coming from the
 * Saab Parking Assist module. Calling this function at, for example, 5Hz, will
 * allow the message to exist perpetually on the SID
 * 
 * @param _aUpperMsg String to print to upper SID line
 * @param _aLowerMsg String to print to lower SID line
 */
void SID_Interface::_mSendTextSpa( std::string _aUpperMsg, std::string _aLowerMsg )
{
    assert( _aUpperMsg.size() == 12 );
    assert( _aLowerMsg.size() == 12 );

    can_frame frames[6];
    memset( frames, 0, sizeof( frames ) );

    for( uint8_t i=0; i<6; i++ )
    {
        frames[i].can_id = 0x337;
        frames[i].can_dlc = 8;
    }

    populate_text_msg( (text_msg_t*)frames[0].data,  true, 0x5,  true, _aUpperMsg.c_str(),      5 );
    populate_text_msg( (text_msg_t*)frames[1].data, false, 0x4,  true, _aUpperMsg.c_str() + 5,  5 );
    populate_text_msg( (text_msg_t*)frames[2].data, false, 0x3,  true, _aUpperMsg.c_str() + 10, 2 );
    populate_text_msg( (text_msg_t*)frames[3].data, false, 0x2, false, _aLowerMsg.c_str(),      5 );
    populate_text_msg( (text_msg_t*)frames[4].data, false, 0x1, false, _aLowerMsg.c_str() + 5,  5 );
    populate_text_msg( (text_msg_t*)frames[5].data, false, 0x0, false, _aLowerMsg.c_str() + 10, 2 );

    can_frame term_frame =
    {
        .can_id = 0x357,
        .can_dlc = 8,
        .data = { 0x1f, 0x01, 0x05, 0x12, 0x00, 0x00, 0x00, 0x00 }
    };

    for( uint8_t i=0; i<6; i++ )
    {
        _mCanWrapper.send_frame( frames[i] );
    }
    _mCanWrapper.send_frame( term_frame );
}

/**
 * @brief Function executed as standalone thread, which prints the strings continuously so
 * they stay visible on the SID
 */
void SID_Interface::_mTextFunc()
{
    std::string upper, lower;

    while( _mActive.load() )
    {
        {
            std::unique_lock<std::mutex> lock( _mStringMutex );

            upper = _mUpperStr;
            lower = _mLowerStr;
        }

        _mSendTextSpa( upper, lower );

        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
    }
}

/**
 * @brief Sets the string to be displayed on the upper line of the SID
 * 
 * @param _aUpperStr 
 */
void SID_Interface::set_upper_str( std::string _aUpperStr )
{
    while( _aUpperStr.size() < 12 )
    {
        _aUpperStr.append( " " );
    }

    _aUpperStr = std::string( _aUpperStr.c_str(), 12 );

    std::unique_lock<std::mutex> lock( _mStringMutex );

    _mUpperStr = _aUpperStr;
}

/**
 * @brief Sets the string to be displayed on the lower line of the SID
 * 
 * @param _aLowerStr 
 */
void SID_Interface::set_lower_str( std::string _aLowerStr )
{
    while( _aLowerStr.size() < 12 )
    {
        _aLowerStr.append( " " );
    }

    _aLowerStr = std::string( _aLowerStr.c_str(), 12 );

    std::unique_lock<std::mutex> lock( _mStringMutex );

    _mLowerStr = _aLowerStr;
}
