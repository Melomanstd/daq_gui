#ifndef DEFINES
#define DEFINES

enum
{
    MODE_NO_MEASURING,
    MODE_SINGLESHOT_MEASURING,
    MODE_BLOCK_MEASURING
};

enum
{
    STATE_OFF,
    STATE_ON
};

enum
{
    CHANNEL_0,
    CHANNEL_1
};

class ModeParameters
{
public:
    ModeParameters() :
        mode(MODE_NO_MEASURING),
        measuringInterval(2),
        blockSize(2),
        displayedInterval(10)
    {}

    int mode;
    int measuringInterval;
    int blockSize;
    int displayedInterval;
};

#endif // DEFINES

