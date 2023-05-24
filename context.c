#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <jansson.h>

#define API_KEY "YOUR_API_KEY"

// Flight data API endpoint
const char* API_ENDPOINT = "https://api.flightdata.com";

// Structure to hold flight data
typedef struct {
    const char* flightNumber;
    const char* flightStatus;
    const char* departureAirport;
    const char* arrivalAirport;
    const char* departureTime;
    const char* arrivalTime;
    double fare;
} FlightData;

// Function to perform an HTTP GET request
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* user_data) {
    size_t real_size = size * nmemb;
    char* response = (char*)user_data;

    strncat(response, (const char*)contents, real_size);
    return real_size;
}

// Function to fetch flight data
void fetchFlightData(const char* flightNumber) {
    CURL* curl;
    CURLcode res;
    char url[256];
    char response[4096] = "";

    snprintf(url, sizeof(url), "%s/flight?number=%s&api_key=%s", API_ENDPOINT, flightNumber, API_KEY);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parse JSON response
            json_error_t error;
            json_t* root = json_loads(response, 0, &error);

            if (!root) {
                fprintf(stderr, "Failed to parse JSON response: %s\n", error.text);
            } else {
                // Extract flight data from JSON
                size_t numFares = json_array_size(json_object_get(root, "fares"));
                FlightData* flightData = malloc(numFares * sizeof(FlightData));

                for (size_t i = 0; i < numFares; i++) {
                    json_t* fareObj = json_array_get(json_object_get(root, "fares"), i);
                    json_t* fareValue = json_object_get(fareObj, "value");

                    flightData[i].flightNumber = flightNumber;
                    flightData[i].flightStatus = json_string_value(json_object_get(root, "status"));
                    flightData[i].departureAirport = json_string_value(json_object_get(root, "departure_airport"));
                    flightData[i].arrivalAirport = json_string_value(json_object_get(root, "arrival_airport"));
                    flightData[i].departureTime = json_string_value(json_object_get(root, "departure_time"));
                    flightData[i].arrivalTime = json_string_value(json_object_get(root, "arrival_time"));
                    flightData[i].fare = json_real_value(fareValue);
                }

                // Sort flight data by fare (lowest to highest)
                for (size_t i = 0; i < numFares - 1; i++) {
                    for (size_t j = i + 1; j < numFares; j++) {
                        if (flightData[j].fare < flightData[i].fare) {
                            FlightData
