#include <unity.h>

#include <map>
#include <string>
#include <vector>

#include "adapters/implementations/mqtt_radio_diagnostics.h"

struct Publication
{
    std::string topic;
    std::string payload;
    bool retain;
};

static std::vector<Publication> publications;
static std::map<std::string, std::string> retained;

static void recordPublication(const char *topic, const char *payload, bool retain)
{
    publications.push_back({topic, payload, retain});
    if (retain) retained[topic] = payload;
}

void setUp()
{
    publications.clear();
    retained.clear();
}

void tearDown() {}

static tmeter_data validReading()
{
    tmeter_data data{};
    data.volume = 123;
    data.reads_counter = 1;
    data.rssi_dbm = -80;
    data.lqi = 0;
    data.freqest = 4;
    data.signal_valid = true;
    data.lqi_valid = true;
    return data;
}

static void assertRetainedDiagnostics()
{
    TEST_ASSERT_EQUAL_UINT(5, publications.size());
    for (const auto &publication : publications)
    {
        TEST_ASSERT_TRUE(publication.retain);
        TEST_ASSERT_FALSE(publication.payload.empty());
    }
}

static void test_success_publishes_numeric_retained_diagnostics()
{
    publishMqttRadioDiagnostics(validReading(), recordPublication);
    assertRetainedDiagnostics();
    TEST_ASSERT_EQUAL_STRING("-80", retained["rssi_dbm"].c_str());
    TEST_ASSERT_EQUAL_STRING("50", retained["rssi_percentage"].c_str());
    TEST_ASSERT_EQUAL_STRING("0", retained["lqi"].c_str());
    TEST_ASSERT_EQUAL_STRING("100", retained["lqi_percentage"].c_str());
    TEST_ASSERT_EQUAL_STRING("6.348", retained["frequency_estimate"].c_str());
}

static void test_corrupt_frame_keeps_signed_signal_measurements()
{
    tmeter_data data = validReading();
    data.volume = data.reads_counter = 0;
    data.failure = ReadFailure::CrcFailed;
    data.rssi_dbm = -138;
    data.lqi = 127;
    data.freqest = -128;
    publishMqttRadioDiagnostics(data, recordPublication);
    assertRetainedDiagnostics();
    TEST_ASSERT_EQUAL_STRING("-138", retained["rssi_dbm"].c_str());
    TEST_ASSERT_EQUAL_STRING("0", retained["rssi_percentage"].c_str());
    TEST_ASSERT_EQUAL_STRING("127", retained["lqi"].c_str());
    TEST_ASSERT_EQUAL_STRING("0", retained["lqi_percentage"].c_str());
    TEST_ASSERT_EQUAL_STRING("-203.136", retained["frequency_estimate"].c_str());
}

static void test_silence_overwrites_all_retained_measurements_with_none()
{
    publishMqttRadioDiagnostics(validReading(), recordPublication);
    publications.clear();
    tmeter_data data{};
    data.failure = ReadFailure::NoReply;
    publishMqttRadioDiagnostics(data, recordPublication);
    assertRetainedDiagnostics();
    TEST_ASSERT_EQUAL_UINT(5, retained.size());
    for (const auto &entry : retained)
        TEST_ASSERT_EQUAL_STRING("None", entry.second.c_str());
}

static void test_partial_frame_clears_lqi_but_keeps_signal()
{
    publishMqttRadioDiagnostics(validReading(), recordPublication);
    publications.clear();
    tmeter_data data = validReading();
    data.volume = data.reads_counter = 0;
    data.failure = ReadFailure::NoReply;
    data.lqi_valid = false;
    data.freqest = 0;
    publishMqttRadioDiagnostics(data, recordPublication);
    assertRetainedDiagnostics();
    TEST_ASSERT_EQUAL_STRING("-80", retained["rssi_dbm"].c_str());
    TEST_ASSERT_EQUAL_STRING("50", retained["rssi_percentage"].c_str());
    TEST_ASSERT_EQUAL_STRING("0.000", retained["frequency_estimate"].c_str());
    TEST_ASSERT_EQUAL_STRING("None", retained["lqi"].c_str());
    TEST_ASSERT_EQUAL_STRING("None", retained["lqi_percentage"].c_str());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_success_publishes_numeric_retained_diagnostics);
    RUN_TEST(test_corrupt_frame_keeps_signed_signal_measurements);
    RUN_TEST(test_silence_overwrites_all_retained_measurements_with_none);
    RUN_TEST(test_partial_frame_clears_lqi_but_keeps_signal);
    return UNITY_END();
}
