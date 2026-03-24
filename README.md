# 🎥 obs-multi-rtmp (WebSocket Edition)

Adds **OBS WebSocket API support** to the [obs-multi-rtmp](https://github.com/sorayuki/obs-multi-rtmp) plugin,
enabling full remote control of RTMP targets.

------------------------------------------------------------------------

## 🚀 Features

-   Full control over RTMP targets via WebSocket
-   Start / Stop individual or all targets
-   Dynamic target management (add, clone, delete)
-   Sync with OBS streaming state
-   Retrieve detailed stats per target

------------------------------------------------------------------------

## 🔌 WebSocket API

### 📡 Target Management

-   `ListTargets` -- Get all configured RTMP targets\
-   `GetTargetState` -- Get current state of a specific target\
-   `AddTarget` -- Add a new target\
-   `CloneTarget` -- Clone an existing target\
-   `DeleteTarget` -- Delete a target

### ▶️ Stream Control

-   `StartTarget` -- Start a specific target\
-   `StopTarget` -- Stop a specific target\
-   `ToggleTarget` -- Toggle start/stop state\
-   `StartAll` -- Start all targets\
-   `StopAll` -- Stop all targets

### ⚙️ Configuration

-   `UpdateTargetName` -- Rename a target\
-   `UpdateStreamKey` -- Update stream key\
-   `UpdateServiceParam` -- Update service parameters\
-   `UpdateSyncStart` -- Sync start with OBS\
-   `UpdateSyncStop` -- Sync stop with OBS

### 📊 Monitoring

-   `GetTargetStats` -- Retrieve stats for a target

------------------------------------------------------------------------

## 🛠️ Build Instructions (Windows)

``` bash
cls
cd "C:\obs-multi-rtmp-websocket"   # adjust path
rmdir /s /q build
mkdir build
cd build
cls

cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DENABLE_QT=ON ^
  -DENABLE_FRONTEND_API=ON ^
  -DENABLE_WEBSOCKET=ON

cmake --build . --config Release
```

------------------------------------------------------------------------

## 📦 Requirements

-   OBS Studio (with plugin support)
-   Visual Studio 2022
-   CMake
-   Qt (enabled via flag)

------------------------------------------------------------------------

## 📚 Based on

This project uses **[obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate)**\

------------------------------------------------------------------------

## 💡 Notes

-   WebSocket API extends OBS functionality significantly
-   Designed for automation & multi-stream setups
-   Compatible with existing obs-multi-rtmp workflows

------------------------------------------------------------------------

## 🧠 Author Notes

Clean, minimal, and focused on performance + remote control use cases.
