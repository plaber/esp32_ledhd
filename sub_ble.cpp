#include "sub_ble.h"
#include "conf.h"
#include "sub_btn.h"
#include "sub_udp.h"

#ifdef USEBLE

#define TaskStackSize 1024
#include "BLEDevice.h"

#define ServerName  "VR BOX"
 
typedef void (*NotifyCallback)(BLERemoteCharacteristic*, uint8_t*, size_t, bool);

static BLEUUID serviceUUID("00001812-0000-1000-8000-00805f9b34fb");
static BLEUUID ReportCharUUID("00002A4D-0000-1000-8000-00805f9b34fb");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLEAdvertisedDevice* myDevice;

std::map<std::uint16_t, BLERemoteCharacteristic*> *pmapbh;
std::map<std::uint16_t, BLERemoteCharacteristic*> :: iterator itrbh;

TaskHandle_t HandleCh = NULL;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)
{
	for (int i = 0; i < length; i++)
		Serial.printf("%02X ", pData[i]);
	String ans = "";
	if (pData[0] == 0x40) {state.loop = true; ans = get_answ("go","1");}
	if (pData[0] == 0x80) ans = get_answ("stp","1");
	if (pData[0] == 0x10) ans = get_answ("brgn","m");
	if (pData[0] == 0x01) ans = get_answ("brgn","p");
	if (pData[0] == 0x08) ans = get_answ("modp","m");
	if (pData[0] == 0x02) ans = get_answ("modp","p");
	Serial.println(ans);
}

class MyClientCallback : public BLEClientCallbacks
{
	void onConnect(BLEClient* pclient)
	{
		Serial.println("onConnect event");
	}
	void onDisconnect(BLEClient* pclient)
	{
		Serial.println("onDisconnect event");
		connected = false;
	}
};

bool setupCharacteristics(BLERemoteService* pRemoteService, NotifyCallback pNotifyCallback)
{
	char bfr[80];
	pmapbh = pRemoteService->getCharacteristicsByHandle();
	for (itrbh = pmapbh->begin(); itrbh != pmapbh->end(); itrbh++)
	{
		BLEUUID x = itrbh->second->getUUID();
		Serial.print("Characteristic UUID: ");
		Serial.println(x.toString().c_str());
		if (ReportCharUUID.equals(itrbh->second->getUUID()))
		{
			Serial.println("Found a report characteristic");
			if (itrbh->second->canNotify())
			{
				Serial.println("Can notify");
				itrbh->second->registerForNotify(pNotifyCallback);
				sprintf(bfr, "Callback registered for: Handle: 0x%08X, %d", itrbh->first, itrbh->first);
				Serial.println(bfr);
			}
			else
			{
				Serial.println("No notification");
			}
		}
		else
		{
			sprintf(bfr, "Found Characteristic UUID: %s\n", itrbh->second->getUUID().toString().c_str()); 
			Serial.println(bfr);
		}
	}
}

bool connectToServer()
{
	Serial.print("Forming a connection to ");
	Serial.println(myDevice->getAddress().toString().c_str());
	BLEClient* pClient = BLEDevice::createClient();
	Serial.println(" - Created client");
	pClient->setClientCallbacks(new MyClientCallback());
	pClient->connect(myDevice);
	Serial.println(" - Connected to server");
	BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
	if (pRemoteService == nullptr)
	{
		Serial.print("Failed to find HID service UUID: ");
		Serial.println(serviceUUID.toString().c_str());
		pClient->disconnect();
		return false;
	}
	Serial.println(" - Found HID service");
	setupCharacteristics(pRemoteService, notifyCallback);
	connected = true;
}

void taskCheckoff(void *parameter)
{
	while(true)
	{
		vTaskSuspend(NULL);
		check_off();
	}
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
	void onResult(BLEAdvertisedDevice advertisedDevice)
	{
		Serial.print("BLE Advertised Device found: ");
		Serial.println(advertisedDevice.toString().c_str());
		if (advertisedDevice.haveName())
		{
			if (0 == strcmp(ServerName, advertisedDevice.getName().c_str()))
			{
				Serial.println("Found VRBOX Server");
				if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
				{
					Serial.println("Server has HID service");
					BLEDevice::getScan()->stop();
					myDevice = new BLEAdvertisedDevice(advertisedDevice);
					doConnect = true;
					doScan = true;
				}
				else
				{
					Serial.println("Server does not have an HID service, not our server");
				}
			}
		}
		else
		{
			Serial.println("Server name does not match, not our server");
		}
		vTaskResume(HandleCh);
	}
};

void ble_init()
{
	BaseType_t xReturned;
	// create tasks to handle the joystick and buttons
	xReturned = xTaskCreate(taskCheckoff, 			// task to handle activity on the joystick.
							"CheckOff",				// String with name of task.
							TaskStackSize,			// Stack size in 32 bit words.
							NULL, 					// Parameter passed as input of the task
							1,						// Priority of the task.
							&HandleCh);				// Task handle.
	if (pdPASS == xReturned)
	{
		Serial.println("CheckOff Task Created");
	}

	Serial.println("Starting ESP32 BLE Client...");
	BLEDevice::init("");
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setInterval(1349);
	pBLEScan->setWindow(449);
	pBLEScan->setActiveScan(true);
	pBLEScan->start(5, false);      // scan for 5 seconds
	if (!connected)
	{
		doScan = true;
		Serial.println("Offline, start a scan");
	}
}

void ble_poll()
{
	if (doConnect == true) 
	{
		if (connectToServer())
		{
			Serial.println("We are now connected to the BLE Server.");
		} 
		else
		{
			Serial.println("We have failed to connect to the server; there is nothin more we will do.");
		}
		doConnect = false;
	}
	if (connected)
	{
		vTaskResume(HandleCh);
	}
	else if (doScan)
	{
		Serial.println("Start scanning after disconnect");
		BLEDevice::getScan()->start(1);
	}
}

#endif