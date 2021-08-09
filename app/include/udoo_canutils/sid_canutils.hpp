#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#include <socketcan_wrapper/socketcan_wrapper.hpp>

class SID_Interface
{
private:
    SocketCAN_Wrapper _mCanWrapper;
    std::string _mUpperStr, _mLowerStr;
    std::mutex _mStringMutex;
    std::atomic<bool> _mActive;
    std::thread _mTextThread;

    void _mSendTextSpa( std::string _aUpperMsg, std::string _aLowerMsg );
    void _mTextFunc();
public:
    SID_Interface( const std::string& _aIfaceName, float _mTimeoutSec = -1.0F )
        : _mCanWrapper( _aIfaceName, _mTimeoutSec ),
          _mUpperStr( "            " ),
          _mLowerStr( "            " ),
          _mActive( true ),
          _mTextThread( [&](){ _mTextFunc(); } )
    {}
    ~SID_Interface()
    {
        _mSendTextSpa( "            ", "            " );
        std::cout << "destructor" << std::endl;
        _mActive.store( false );
        _mTextThread.join();
    }

    void set_upper_str( std::string _aUpperStr );
    void set_lower_str( std::string _aUpperStr );
};
