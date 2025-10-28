#pragma once
#include "framework.h" // MFC core and standard components
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// STL - Ensure these are included if used globally
#include <string>
#include <vector>
#include <list>
#include <array>
#include <memory>
#include <queue>   // Added for CTcpCommunicator
#include <mutex>   // Added for CTcpCommunicator
#include <filesystem> // Added for CLivePanel (Requires C++17 or later)

// OpenCV - Make sure vcpkg provides this or path is set
// NOTE: Make sure opencv4[contrib] is installed via vcpkg for BackgroundSubtractorMOG2
#include <opencv2/opencv.hpp>
// Explicitly include core and imgproc if needed elsewhere, though opencv.hpp usually covers it
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp> // Needed for cv::imencode/decode? (often included by opencv.hpp)
#include <opencv2/imgcodecs.hpp> // Needed for cv::imencode/decode
#include <opencv2/video/background_segm.hpp> // For BackgroundSubtractorMOG2

// JSON (vcpkg: nlohmann-json) - Make sure vcpkg provides this or path is set
#include <nlohmann/json.hpp>

// GDI+
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// Pylon SDK (Needs correct include paths configured in Project Properties -> C/C++ -> General -> Additional Include Directories if not using vcpkg)
// Make sure PYLON_WIN_BUILD environment variable might be needed or paths set correctly.
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h> // Ensure this specific header exists

// Project Specific Headers needed across multiple files
#include "resource.h" // Include resource IDs here if used widely
// Include SharedData *after* its dependencies (like opencv types, nlohmann::json, Pylon types) are declared/included above.
#include "SharedData.h"

// Define NOMINMAX before including windows.h or related headers if min/max conflicts occur
// #define NOMINMAX
// Define WIN32_LEAN_AND_MEAN if not already done by framework.h to speed up builds
// #define WIN32_LEAN_AND_MEAN
