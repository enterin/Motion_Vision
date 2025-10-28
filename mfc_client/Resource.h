#pragma once

#define IDR_MAINFRAME               128
#define IDS_APP_TITLE               129
#define IDS_HELLO                   130

// Icons
#define IDI_MAINFRAME               200
#define IDI_DOCDOC                  201

// Toolbar bitmap
#define IDB_MAINTOOLBAR             300

// Commands (match code)
#define ID_SETTINGS_BTN             (WM_USER + 211)
#define ID_MANUAL_CAPTURE_BTN       (WM_USER + 210)

// Custom camera button base (redeclare to RC scope)
#define ID_CAM_BTN_BASE             (WM_USER + 200)

// Dialogs
#define IDD_ABOUTBOX                100
#define IDD_CAMERA_SETUP            101

#define ID_EXPORT_CSV               140


// ==== Live Panel ====
#define IDD_LIVE_PANEL              3000
#define IDC_TAB_LIVE                3001

// ==== Commands (Buttons / Menu) ====
#define ID_VIEW_TOGGLE_LIVEPANEL    3100   // 도킹 토글 버튼
#define ID_VIEW_DOCK_LEFT           3101
#define ID_VIEW_DOCK_RIGHT          3102
#define ID_VIEW_FLOAT               3103
#define ID_VIEW_HIDE                3104
#define ID_LIVE_SNAPSHOT            3110   // 탭 컨텍스트: 스냅샷 저장
#define ID_LIVE_OPEN_CAPTUREFOLDER  3111   // 탭 컨텍스트: 캡처폴더 열기

// ==== Popup Menu ====
#define IDR_MENU_LIVE_TAB           3200

