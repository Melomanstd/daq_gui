#ifndef DEFINES
#define DEFINES

#define MAXIMUM_PLOT_SAMPLES 1000

enum
{
    MODE_NO_MEASURING,
    MODE_SINGLESHOT_MEASURING,
    MODE_BLOCK_MEASURING,
    MODE_HF_MEASURING
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
        displayedInterval(10),
        channelZeroState(STATE_OFF),
        channelOneState(STATE_OFF),
        scaningInterval(0),
        samplingInterval(0)
    {}

    int mode;
    int measuringInterval;
    int blockSize;
    int displayedInterval;
    unsigned char channelZeroState;
    unsigned char channelOneState;
    int scaningInterval;
    int samplingInterval;
};

#endif // DEFINES

