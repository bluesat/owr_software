#include <string>
#include <cstdint>
#include <vector>

struct gpsData {
    uint16_t time;
    int32_t latitude; // lat * 10000 - to avoid floats
    int32_t longitude; // lon * 10000 - to avoid floats
    uint16_t numSatelites;
    int16_t altitude;
    uint16_t fixValid;
} __attribute__((packed));
typedef struct gpsData GPSData;


struct status {
    bool roverOk;
    double batteryVoltage;
    GPSData gpsData;
};
	
class Bluetongue {
	private:
		void comm(bool forBattery, void *message, int message_len, void *resp, 
				int resp_len);
		int port_fd;
        fd_set uart_set;
        struct timeval timeout;
	
	public:
		Bluetongue(const char* port);
		~Bluetongue();
		struct status update(double leftMotor, double rightMotor, int armTop,
                int armBottom, double armRotate);
};

