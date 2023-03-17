// CIJ El-Light Module With Smartphones 
// 2023/03/05 Ver1.00
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Easing.h>

const std::string DeviceName = "CijElLightModule2";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
static BLEUUID ServiceUUID("164aedcc-bb49-11ed-afa1-0242ac120002");
static BLEUUID CharacteristicUUID("164af06a-bb49-11ed-afa1-0242ac120002");


namespace Constants
{
    namespace Channels
    {
        const unsigned int PinC = A17;
        const unsigned int PinI = A18;
        const unsigned int PinJ = A19;

        const unsigned int ChannelC = 0;
        const unsigned int ChannelI = 1;
        const unsigned int ChannelJ = 2;

        const unsigned int CijAll[] = {ChannelC, ChannelI, ChannelJ};
        const int CijAllLength = sizeof(CijAll) / sizeof(CijAll[0]);

        struct ChannelSetting
        {
            int Channel;
            int Pin;
        };

        const ChannelSetting ChannelSettings[3] =
        {
            {ChannelC, PinC},
            {ChannelI, PinI},
            {ChannelJ, PinJ}
        };
    }

    namespace Commands
    {
        const byte ON = 0x01;
        const byte OFF = 0x00;

        const byte C = 0x10;
        const byte I = 0x20;
        const byte J = 0x30;

        const byte AUTO = 0x50;
        const byte MANUAL = 0x51;

        const byte C_ON = C | ON;
        const byte C_OFF = C | OFF;
        const byte I_ON = I | ON;
        const byte I_OFF = I | OFF;
        const byte J_ON = J | ON;
        const byte J_OFF = J | OFF;
    }
}

EasingFunc<Ease::SineOut> _easing;

enum PlayModeType
{
    Auto,
    Manual,
};
PlayModeType _playMode;

byte _stateC;
byte _stateI;
byte _stateJ;
byte _oldStateC;
byte _oldStateI;
byte _oldStateJ;

// タスクハンドル
TaskHandle_t _autoBlinkTaskHandle = NULL;
TaskHandle_t _manualBlinkTaskHandle = NULL;

class BleReciveCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (value.length() > 0)
        {
            Serial.println("*********");
            Serial.print("New value: ");
            for (int i = 0; i < value.length(); i++)
            {
                char c = value[i];
                Serial.printf("%#x,", c);

                if (c == Constants::Commands::AUTO)
                {
                    _playMode = PlayModeType::Auto;
                }
                else if (c == Constants::Commands::MANUAL)
                {
                    _playMode = PlayModeType::Manual;
                }
                else
                {
                    switch (c & 0xf0)
                    {
                    case Constants::Commands::C:
                        _stateC = c;
                        break;
                    case Constants::Commands::I:
                        _stateI = c;
                        break;
                    case Constants::Commands::J:
                        _stateJ = c;
                        break;
                    default:
                        break;
                    }
                }
            }
            Serial.println();
            Serial.println("*********");
        }
    }
};

void BleInit()
{
    BLEDevice::init(DeviceName);
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(ServiceUUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CharacteristicUUID,
        BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new BleReciveCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void ElInit()
{
    for (int i = 0; i < Constants::Channels::CijAllLength; i++)
    {
        Constants::Channels::ChannelSetting setting = Constants::Channels::ChannelSettings[i];
        Serial.println(setting.Channel);
        pinMode(setting.Pin, OUTPUT);
        ledcSetup(setting.Channel, 12800, 8);
        ledcAttachPin(setting.Pin, setting.Channel);
    }
}

void SetChannels(const unsigned int *channels, int msec, bool isOn)
{
    if (isOn)
    {
        SetDigitalChannels(channels, isOn);
        delay(msec);
    }
    else
    {
        int fadeTime = 150;
        if (msec > fadeTime)
        {
            msec -= fadeTime;
            for (int i = 0; i < fadeTime; i++)
            {
                double t = (double)i / (double)fadeTime;
                int val = _easing.get(1.0 - t);
                SetAnalogChannels(channels, val);
                delay(1);
            }
        }

        SetDigitalChannels(channels, isOn);
        delay(msec);
    }
}

void SetChannel(const unsigned int channel, int msec, bool isOn)
{
    if (isOn)
    {
        SetDigitalChannel(channel, isOn);
        delay(msec);
    }
    else
    {
        int fadeTime = 130;
        if (msec > fadeTime)
        {
            msec -= fadeTime;
            for (int i = 0; i < fadeTime; i++)
            {
                double t = (double)i / (double)fadeTime;
                int val = _easing.get(1.0 - t);
                SetAnalogChannel(channel, val);
                delay(1);
            }
        }

        SetDigitalChannel(channel, isOn);
        delay(msec);
    }
}

void SetDigitalChannels(const unsigned int *channels, bool isOn)
{
    for (int i = 0; i < sizeof(channels) + 1; i++)
        DigitalWrite(channels[i], isOn);
}

void SetDigitalChannel(const unsigned int channel, bool isOn)
{
    DigitalWrite(channel, isOn);
}

void SetAnalogChannels(const unsigned int *channels, int val)
{
    for (int i = 0; i < sizeof(channels) + 1; i++)
        AnalogWrite(channels[i], val);
}

void SetAnalogChannel(const unsigned int channel, int val)
{
    AnalogWrite(channel, val);
}

void DigitalWrite(const unsigned int channel, bool val)
{
    ledcWrite(channel, val ? 255 : 0);
}

void AnalogWrite(const unsigned int channel, int val)
{
    ledcWrite(channel, val);
}

void ClearAllChannels()
{
    for (int i = 0; i < Constants::Channels::CijAllLength; i++)
    {
        DigitalWrite(Constants::Channels::CijAll[i], false);
    }
}

// 自動点灯用のタスク
void CreateAutoBlinkTask()
{
    xTaskCreateUniversal(
        AutoBlinkTask,
        "AutoBlinkTask",
        8192,
        NULL,
        1, // 優先度
        &_autoBlinkTaskHandle, // タスクハンドル
        APP_CPU_NUM
    );
}

void AutoBlinkTask(void *pvParameters)
{
    // 無限に既定の点灯パターンを繰り返す
    while (true)
    {
        SetChannel(Constants::Channels::ChannelC, 900, HIGH);
        SetChannel(Constants::Channels::ChannelC, 450, LOW);

        SetChannel(Constants::Channels::ChannelI, 900, HIGH);
        SetChannel(Constants::Channels::ChannelI, 450, LOW);

        SetChannel(Constants::Channels::ChannelJ, 900, HIGH);
        SetChannel(Constants::Channels::ChannelJ, 1430, LOW);

        SetChannel(Constants::Channels::ChannelC, 480, HIGH);
        SetChannel(Constants::Channels::ChannelI, 480, HIGH);
        SetChannel(Constants::Channels::ChannelJ, 1230, HIGH);
        SetChannels(Constants::Channels::CijAll, 1100, LOW);

        SetChannels(Constants::Channels::CijAll, 700, HIGH);
        SetChannels(Constants::Channels::CijAll, 600, LOW);

        SetChannels(Constants::Channels::CijAll, 730, HIGH);
        SetChannels(Constants::Channels::CijAll, 1250, LOW);
    }
}

// 手動点灯用のタスク
void CreateManualBlinkTask()
{
    xTaskCreateUniversal(
        ManualBlinkTask,
        "ManualBlinkTask",
        8192,
        NULL,
        1, // 優先度
        &_manualBlinkTaskHandle, // タスクハンドル
        APP_CPU_NUM
    );
}

void ManualBlinkTask(void *pvParameters)
{
    // スマホから送られてくるコマンドを解析して指定のライトを点灯させる
    while (true)
    {
        if (_stateC != _oldStateC)
        {
            _oldStateC = _stateC;
            bool sw = _stateC == Constants::Commands::C_ON;
            SetDigitalChannel(Constants::Channels::ChannelC, sw);
            Serial.printf("EL C %d", sw);
        }
        if (_stateI != _oldStateI)
        {
            _oldStateI = _stateI;
            bool sw = _stateI == Constants::Commands::I_ON;
            SetDigitalChannel(Constants::Channels::ChannelI, sw);
            Serial.printf("EL I %d", sw);
        }
        if (_stateJ != _oldStateJ)
        {
            _oldStateJ = _stateJ;
            bool sw = _stateJ == Constants::Commands::J_ON;
            SetDigitalChannel(Constants::Channels::ChannelJ, sw);
            Serial.printf("EL J %d", sw);
        }
        delay(1);
    }
}

void DeleteTask(TaskHandle_t *taskHandle)
{
    if (*taskHandle != NULL)
    {
        vTaskDelete(*taskHandle);
        (*taskHandle) = NULL;
    }
}

// setup
void setup()
{
    Serial.begin(115200);
    BleInit();
    ElInit();
    CreateAutoBlinkTask();
}

// main loop
void loop()
{
    switch (_playMode)
    {
    case PlayModeType::Auto:
        DeleteTask(&_manualBlinkTaskHandle);
        if (_autoBlinkTaskHandle == NULL)
        {
            ClearAllChannels();
            CreateAutoBlinkTask();
            Serial.printf("Auto\n");
        }
        break;

    case PlayModeType::Manual:
        DeleteTask(&_autoBlinkTaskHandle);
        if (_manualBlinkTaskHandle == NULL)
        {
            ClearAllChannels();
            CreateManualBlinkTask();
            Serial.printf("Manual\n");
        }
        break;

    default:
        break;
    }
    delay(1);
}
