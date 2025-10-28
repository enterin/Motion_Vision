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

// OpenCV - Make sure vcpkg provides this or path is set
#include <opencv2/opencv.hpp>

// JSON (vcpkg: nlohmann-json) - Make sure vcpkg provides this or path is set
#include <nlohmann/json.hpp>

// GDI+
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// Pylon SDK (Needs correct include paths if not using vcpkg)
// Assuming Pylon includes are handled correctly elsewhere or by vcpkg
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>

// Project Specific Headers needed across multiple files
#include "resource.h" // Include resource IDs here if used widely
#include "SharedData.h" // Include this after dependencies like opencv, nlohmann
