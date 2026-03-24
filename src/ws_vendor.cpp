#include "ws_vendor.hpp"
#include "push-widget.h"
#include "multi-output-widget.h"
#include "output-config.h"
#include "plugin-support.h"
#include <obs-websocket-api.h>
#include <QMainWindow>
#include <QThread>
#include <QMetaObject>
#include <QAction>
#include <QTimer>
#include <QRegularExpression>

// ✅ ADD WINDOWS HEADER FOR GetModuleHandleA AND GetProcAddress
#ifdef _WIN32
#include <Windows.h>
#endif

MultiRTMPWebsocketVendor* MultiRTMPWebsocketVendor::s_instance = nullptr;

MultiRTMPWebsocketVendor* MultiRTMPWebsocketVendor::Instance() {
    if (!s_instance) {
        s_instance = new MultiRTMPWebsocketVendor();
    }
    return s_instance;
}

bool MultiRTMPWebsocketVendor::Initialize() {
#ifdef ENABLE_WEBSOCKET
    blog(LOG_INFO, TAG "Websocket support enabled, attempting to register vendor");
    
    // ✅ Use the header API directly - no dynamic loading!
    // First ensure the proc handler is available
    if (!obs_websocket_ensure_ph()) {
        blog(LOG_WARNING, TAG "obs-websocket proc handler not available yet");
        
        // Retry after a short delay
        QTimer::singleShot(1000, []() {
            blog(LOG_INFO, TAG "Retrying vendor registration...");
            MultiRTMPWebsocketVendor::Instance()->Initialize();
        });
        
        return false;
    }
    
    // Register the vendor using the header API
    obs_websocket_vendor vendor_handle = obs_websocket_register_vendor(m_vendorName.c_str());
    if (!vendor_handle) {
        blog(LOG_ERROR, TAG "Failed to register vendor: %s", m_vendorName.c_str());
        return false;
    }
    
    // Store the vendor handle for later use
    m_vendorHandle = vendor_handle;
    blog(LOG_INFO, TAG "Vendor registered successfully: %s", m_vendorName.c_str());
    
    // Now register all our request handlers
    RegisterVendorRequests(vendor_handle);
    
    return true;
#else
    blog(LOG_WARNING, TAG "Websocket support not compiled in");
    return false;
#endif
}

void MultiRTMPWebsocketVendor::Shutdown() {
#ifdef ENABLE_WEBSOCKET
    if (m_vendorHandle) {
        // You might want to unregister requests here if needed
        m_vendorHandle = nullptr;
    }
#endif
    delete s_instance;
    s_instance = nullptr;
}

void MultiRTMPWebsocketVendor::RegisterVendorRequests(obs_websocket_vendor vendor_handle) {
#ifdef ENABLE_WEBSOCKET
    blog(LOG_INFO, TAG "Registering vendor requests...");
    
    // Register each request with its specific callback
    bool success = true;
    
    success &= obs_websocket_vendor_register_request(vendor_handle, "ListTargets", 
                                                    &MultiRTMPWebsocketVendor::HandleListTargetsRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "GetTargetState", 
                                                    &MultiRTMPWebsocketVendor::HandleGetTargetStateRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "StartTarget", 
                                                    &MultiRTMPWebsocketVendor::HandleStartTargetRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "StopTarget", 
                                                    &MultiRTMPWebsocketVendor::HandleStopTargetRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "ToggleTarget", 
                                                    &MultiRTMPWebsocketVendor::HandleToggleTargetRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "StartAll", 
                                                    &MultiRTMPWebsocketVendor::HandleStartAllRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "StopAll", 
                                                    &MultiRTMPWebsocketVendor::HandleStopAllRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "AddTarget", 
                                                    &MultiRTMPWebsocketVendor::HandleAddTargetRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "CloneTarget", 
                                                    &MultiRTMPWebsocketVendor::HandleCloneTargetRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "UpdateTargetName", 
                                                    &MultiRTMPWebsocketVendor::HandleUpdateTargetNameRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "UpdateStreamKey", 
                                                    &MultiRTMPWebsocketVendor::HandleUpdateStreamKeyRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "UpdateServiceParam", 
                                                    &MultiRTMPWebsocketVendor::HandleUpdateServiceParamRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "DeleteTarget", 
                                                    &MultiRTMPWebsocketVendor::HandleDeleteTargetRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "UpdateSyncStart", 
                                                    &MultiRTMPWebsocketVendor::HandleUpdateSyncStartRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "UpdateSyncStop", 
                                                    &MultiRTMPWebsocketVendor::HandleUpdateSyncStopRequest, this);
    success &= obs_websocket_vendor_register_request(vendor_handle, "GetTargetStats", 
                                                    &MultiRTMPWebsocketVendor::HandleGetTargetStatsRequest, this);
    
    if (success) {
        blog(LOG_INFO, TAG "✅ All vendor requests registered successfully");
    } else {
        blog(LOG_ERROR, TAG "❌ Failed to register some vendor requests");
    }
#endif
}

// Individual callback implementations
void MultiRTMPWebsocketVendor::HandleListTargetsRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleListTargets(response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleGetTargetStateRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleGetTargetState(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleStartTargetRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleStartTarget(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleStopTargetRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleStopTarget(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleToggleTargetRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleToggleTarget(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleStartAllRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleStartAll(response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleStopAllRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleStopAll(response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleAddTargetRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleAddTarget(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleCloneTargetRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleCloneTarget(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleUpdateTargetNameRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleUpdateTargetName(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleUpdateStreamKeyRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleUpdateStreamKey(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleUpdateServiceParamRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleUpdateServiceParam(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleDeleteTargetRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleDeleteTarget(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::ExecuteInUIThread(std::function<void()> task) {
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (mainwin) {
        QMetaObject::invokeMethod(mainwin, [task]() {
            task();
        }, Qt::QueuedConnection);
    }
}
void MultiRTMPWebsocketVendor::HandleUpdateSyncStartRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleUpdateSyncStart(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleUpdateSyncStopRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleUpdateSyncStop(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

void MultiRTMPWebsocketVendor::HandleGetTargetStatsRequest(obs_data_t* request_data, obs_data_t* response_data, void* private_data) {
    auto vendor = static_cast<MultiRTMPWebsocketVendor*>(private_data);
    bool success = vendor->HandleGetTargetStats(request_data, response_data);
    obs_data_set_bool(response_data, "success", success);
}

// The rest of your individual handler methods remain unchanged (they return bool)
bool MultiRTMPWebsocketVendor::HandleListTargets(obs_data_t* response_data) {
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    // ✅ FIX: Find the dock widget first, then get the MultiOutputWidget from it
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    auto pushWidgets = multiOutputWidget->GetAllPushWidgets();
    auto targets_array = obs_data_array_create();
    
    for (auto* pushWidget : pushWidgets) {
        auto target_data = obs_data_create();
        
        std::string targetId = pushWidget->GetTargetId().toStdString();
        std::string targetName = pushWidget->GetTargetName().toStdString();
        
        obs_data_set_string(target_data, "id", targetId.c_str());
        obs_data_set_string(target_data, "name", targetName.c_str());
        
        bool isRunning = pushWidget->IsRunning();
        obs_data_set_string(target_data, "state", isRunning ? "running" : "stopped");
        
        obs_data_array_push_back(targets_array, target_data);
        obs_data_release(target_data);
    }
    
    obs_data_set_array(response_data, "targets", targets_array);
    obs_data_array_release(targets_array);
    
    return true;
}

bool MultiRTMPWebsocketVendor::HandleGetTargetState(obs_data_t* request_data, obs_data_t* response_data) {
    PushWidget* targetWidget = FindPushWidgetByIdOrName(request_data);
    if (!targetWidget) {
        obs_data_set_string(response_data, "error", "Target not found");
        return false;
    }
    
    bool isRunning = targetWidget->IsRunning();
    obs_data_set_string(response_data, "state", isRunning ? "running" : "stopped");
    obs_data_set_string(response_data, "id", targetWidget->GetTargetId().toUtf8().constData());
    obs_data_set_string(response_data, "name", targetWidget->GetTargetName().toUtf8().constData());
    
    return true;
}

bool MultiRTMPWebsocketVendor::HandleStartTarget(obs_data_t* request_data, obs_data_t* response_data) {
    PushWidget* targetWidget = FindPushWidgetByIdOrName(request_data);
    if (!targetWidget) {
        obs_data_set_string(response_data, "error", "Target not found");
        return false;
    }
    
    // ✅ FIX: Call method directly instead of using invokeMethod
    targetWidget->StartStreaming();
    
    obs_data_set_string(response_data, "status", "start_requested");
    obs_data_set_string(response_data, "id", targetWidget->GetTargetId().toUtf8().constData());
    
    return true;
}

bool MultiRTMPWebsocketVendor::HandleStopTarget(obs_data_t* request_data, obs_data_t* response_data) {
    PushWidget* targetWidget = FindPushWidgetByIdOrName(request_data);
    if (!targetWidget) {
        obs_data_set_string(response_data, "error", "Target not found");
        return false;
    }
    
    // ✅ FIX: Call method directly instead of using invokeMethod
    targetWidget->StopStreaming();
    
    obs_data_set_string(response_data, "status", "stop_requested");
    obs_data_set_string(response_data, "id", targetWidget->GetTargetId().toUtf8().constData());
    
    return true;
}

bool MultiRTMPWebsocketVendor::HandleToggleTarget(obs_data_t* request_data, obs_data_t* response_data) {
    PushWidget* targetWidget = FindPushWidgetByIdOrName(request_data);
    if (!targetWidget) {
        obs_data_set_string(response_data, "error", "Target not found");
        return false;
    }
    
    bool isRunning = targetWidget->IsRunning();
    
    // ✅ FIX: Call methods directly instead of using invokeMethod
    if (isRunning) {
        targetWidget->StopStreaming();
    } else {
        targetWidget->StartStreaming();
    }
    
    obs_data_set_string(response_data, "status", isRunning ? "stop_requested" : "start_requested");
    obs_data_set_string(response_data, "id", targetWidget->GetTargetId().toUtf8().constData());
    
    return true;
}

bool MultiRTMPWebsocketVendor::HandleStartAll(obs_data_t* response_data) {
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    auto pushWidgets = multiOutputWidget->GetAllPushWidgets();
    
    // ✅ FIX: Use ExecuteInUIThread to run in the correct thread
    ExecuteInUIThread([pushWidgets]() {
        for (auto* widget : pushWidgets) {
            widget->StartStreaming();
        }
    });
    
    obs_data_set_string(response_data, "status", "start_all_requested");
    obs_data_set_int(response_data, "target_count", (int)pushWidgets.size());
    
    return true;
}

bool MultiRTMPWebsocketVendor::HandleStopAll(obs_data_t* response_data) {
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    auto pushWidgets = multiOutputWidget->GetAllPushWidgets();
    
    // ✅ FIX: Use ExecuteInUIThread to run in the correct thread
    ExecuteInUIThread([pushWidgets]() {
        for (auto* widget : pushWidgets) {
            widget->StopStreaming();
        }
    });
    
    obs_data_set_string(response_data, "status", "stop_all_requested");
    obs_data_set_int(response_data, "target_count", (int)pushWidgets.size());
    
    return true;
}

// Configuration management handlers
bool MultiRTMPWebsocketVendor::HandleAddTarget(obs_data_t* request_data, obs_data_t* response_data) {
    const char* name = obs_data_get_string(request_data, "name");
    const char* protocol = obs_data_get_string(request_data, "protocol");
    
    if (!name) {
        obs_data_set_string(response_data, "error", "Missing name parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    // ✅ FIX: Capture variables properly
    bool success = false;
    QString nameQStr = QString::fromUtf8(name);
    QString protocolQStr = protocol ? QString::fromUtf8(protocol) : QString("RTMP");
    
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, nameQStr, protocolQStr, &success]() {
        success = multiOutputWidget->AddNewTarget(nameQStr, protocolQStr);
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    
    if (success) {
        obs_data_set_string(response_data, "status", "target_added");
    } else {
        obs_data_set_string(response_data, "error", "Failed to add target");
    }
    
    return success;
}

bool MultiRTMPWebsocketVendor::HandleCloneTarget(obs_data_t* request_data, obs_data_t* response_data) {
    const char* sourceId = obs_data_get_string(request_data, "sourceId");
    const char* newName = obs_data_get_string(request_data, "newName");
    const char* newStreamKey = obs_data_get_string(request_data, "newStreamKey");
    
    if (!sourceId || !newName) {
        obs_data_set_string(response_data, "error", "Missing sourceId or newName parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    QString streamKeyQStr = newStreamKey ? QString::fromUtf8(newStreamKey) : QString();
    
    // ✅ FIX: Execute in UI thread to avoid threading issues
    bool success = false;
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, sourceId, newName, streamKeyQStr, &success]() {
        success = multiOutputWidget->CloneTarget(QString::fromUtf8(sourceId), 
                                               QString::fromUtf8(newName),
                                               streamKeyQStr);
    }, Qt::BlockingQueuedConnection);  // Use blocking to get the result
    
    obs_data_set_bool(response_data, "success", success);
    
    if (success) {
        obs_data_set_string(response_data, "status", "target_cloned");
    } else {
        obs_data_set_string(response_data, "error", "Failed to clone target");
    }
    
    return success;
}

// Fix HandleUpdateTargetName
bool MultiRTMPWebsocketVendor::HandleUpdateTargetName(obs_data_t* request_data, obs_data_t* response_data) {
    const char* targetId = obs_data_get_string(request_data, "targetId");
    const char* newName = obs_data_get_string(request_data, "newName");
    
    if (!targetId || !newName) {
        obs_data_set_string(response_data, "error", "Missing targetId or newName parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    // ✅ FIX: Capture variables properly and use correct function signature
    bool success = false;
    QString targetIdQStr = QString::fromUtf8(targetId);
    QString newNameQStr = QString::fromUtf8(newName);
    
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, targetIdQStr, newNameQStr, &success]() {
        success = multiOutputWidget->UpdateTargetName(targetIdQStr, newNameQStr);
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    
    if (success) {
        obs_data_set_string(response_data, "status", "name_updated");
    } else {
        obs_data_set_string(response_data, "error", "Failed to update name");
    }
    
    return success;
}

// Fix HandleUpdateStreamKey
bool MultiRTMPWebsocketVendor::HandleUpdateStreamKey(obs_data_t* request_data, obs_data_t* response_data) {
    const char* targetId = obs_data_get_string(request_data, "targetId");
    const char* streamKey = obs_data_get_string(request_data, "streamKey");
    
    if (!targetId || !streamKey) {
        obs_data_set_string(response_data, "error", "Missing targetId or streamKey parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    // ✅ FIX: Capture variables properly and use correct function signature
    bool success = false;
    QString targetIdQStr = QString::fromUtf8(targetId);
    QString streamKeyQStr = QString::fromUtf8(streamKey);
    
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, targetIdQStr, streamKeyQStr, &success]() {
        success = multiOutputWidget->UpdateTargetStreamKey(targetIdQStr, streamKeyQStr);
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    
    if (success) {
        obs_data_set_string(response_data, "status", "stream_key_updated");
    } else {
        obs_data_set_string(response_data, "error", "Failed to update stream key");
    }
    
    return success;
}

// Fix HandleUpdateServiceParam
bool MultiRTMPWebsocketVendor::HandleUpdateServiceParam(obs_data_t* request_data, obs_data_t* response_data) {
    const char* targetId = obs_data_get_string(request_data, "targetId");
    const char* key = obs_data_get_string(request_data, "key");
    const char* value = obs_data_get_string(request_data, "value");
    
    if (!targetId || !key || !value) {
        obs_data_set_string(response_data, "error", "Missing targetId, key, or value parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    // ✅ FIX: Capture variables properly and use correct function signature
    bool success = false;
    QString targetIdQStr = QString::fromUtf8(targetId);
    QString keyQStr = QString::fromUtf8(key);
    QString valueQStr = QString::fromUtf8(value);
    
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, targetIdQStr, keyQStr, valueQStr, &success]() {
        success = multiOutputWidget->UpdateTargetServiceParam(targetIdQStr, keyQStr, valueQStr);
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    
    if (success) {
        obs_data_set_string(response_data, "status", "service_param_updated");
    } else {
        obs_data_set_string(response_data, "error", "Failed to update service parameter");
    }
    
    return success;
}

bool MultiRTMPWebsocketVendor::HandleDeleteTarget(obs_data_t* request_data, obs_data_t* response_data) {
    const char* targetId = obs_data_get_string(request_data, "targetId");
    
    if (!targetId) {
        obs_data_set_string(response_data, "error", "Missing targetId parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    // ✅ FIX: Execute in UI thread
    bool success = false;
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, targetId, &success]() {
        success = multiOutputWidget->DeleteTarget(QString::fromUtf8(targetId));
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    
    if (success) {
        obs_data_set_string(response_data, "status", "target_deleted");
    } else {
        obs_data_set_string(response_data, "error", "Failed to delete target");
    }
    
    return success;
}

bool MultiRTMPWebsocketVendor::HandleUpdateSyncStart(obs_data_t* request_data, obs_data_t* response_data) {
    const char* targetId = obs_data_get_string(request_data, "targetId");
    bool syncStart = obs_data_get_bool(request_data, "syncStart");
    
    if (!targetId) {
        obs_data_set_string(response_data, "error", "Missing targetId parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    bool success = false;
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, targetId, syncStart, &success]() {
        success = multiOutputWidget->UpdateSyncStart(QString::fromUtf8(targetId), syncStart);
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    return success;
}

bool MultiRTMPWebsocketVendor::HandleUpdateSyncStop(obs_data_t* request_data, obs_data_t* response_data) {
    const char* targetId = obs_data_get_string(request_data, "targetId");
    bool syncStop = obs_data_get_bool(request_data, "syncStop");
    
    if (!targetId) {
        obs_data_set_string(response_data, "error", "Missing targetId parameter");
        return false;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) {
        obs_data_set_string(response_data, "error", "Cannot find main window");
        return false;
    }
    
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        obs_data_set_string(response_data, "error", "Multi RTMP dock not found");
        return false;
    }
    
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        obs_data_set_string(response_data, "error", "Failed to cast to MultiOutputWidget");
        return false;
    }
    
    bool success = false;
    QMetaObject::invokeMethod(multiOutputWidget, [multiOutputWidget, targetId, syncStop, &success]() {
        success = multiOutputWidget->UpdateSyncStop(QString::fromUtf8(targetId), syncStop);
    }, Qt::BlockingQueuedConnection);
    
    obs_data_set_bool(response_data, "success", success);
    return success;
}

bool MultiRTMPWebsocketVendor::HandleGetTargetStats(obs_data_t* request_data, obs_data_t* response_data) {
    PushWidget* targetWidget = FindPushWidgetByIdOrName(request_data);
    if (!targetWidget) {
        obs_data_set_string(response_data, "error", "Target not found");
        return false;
    }
    
    // Get the status text (this is thread-safe)
    QString statusText;
    QMetaObject::invokeMethod(targetWidget, [targetWidget, &statusText]() {
        statusText = targetWidget->GetStatusText();
    }, Qt::BlockingQueuedConnection);
    
    // Parse the status text to extract structured data
    ParseStatusText(statusText, response_data);
    
    // Add basic target info
    obs_data_set_string(response_data, "id", targetWidget->GetTargetId().toUtf8().constData());
    obs_data_set_string(response_data, "name", targetWidget->GetTargetName().toUtf8().constData());
    obs_data_set_bool(response_data, "isRunning", targetWidget->IsRunning());
    obs_data_set_string(response_data, "rawStatus", statusText.toUtf8().constData());
    
    return true;
}

// NEW: Parse status text helper method
void MultiRTMPWebsocketVendor::ParseStatusText(const QString& statusText, obs_data_t* response_data) {
    if (statusText.isEmpty()) {
        // Stream is not active or no stats available
        obs_data_set_string(response_data, "duration", "00:00:00");
        obs_data_set_string(response_data, "bitrate", "0 bps");
        obs_data_set_string(response_data, "fps", "0 FPS");
        obs_data_set_double(response_data, "bitrateValue", 0);
        obs_data_set_double(response_data, "fpsValue", 0);
        return;
    }
    
    // Parse the format: "HH:MM:SS  X.XX Mbps  Y FPS"
    // Use Qt6 compatible split - Qt::SkipEmptyParts might not be available in older Qt6 versions
    QStringList parts = statusText.split("  ", Qt::SplitBehaviorFlags::SkipEmptyParts);
    
    if (parts.size() >= 3) {
        // Duration (first part)
        obs_data_set_string(response_data, "duration", parts[0].toUtf8().constData());
        
        // Bitrate (second part)
        QString bitrateStr = parts[1];
        obs_data_set_string(response_data, "bitrate", bitrateStr.toUtf8().constData());
        
        // Extract numeric bitrate value
        double bitrateValue = ParseBitrateValue(bitrateStr);
        obs_data_set_double(response_data, "bitrateValue", bitrateValue);
        
        // FPS (third part)
        QString fpsStr = parts[2];
        obs_data_set_string(response_data, "fps", fpsStr.toUtf8().constData());
        
        // Extract numeric FPS value
        double fpsValue = ParseFpsValue(fpsStr);
        obs_data_set_double(response_data, "fpsValue", fpsValue);
    }
}

// NEW: Parse bitrate value helper
double MultiRTMPWebsocketVendor::ParseBitrateValue(const QString& bitrateStr) {
    // Examples: "1.23 Mbps", "456 Kbps", "789 bps"
    QRegularExpression regex("([\\d.]+)\\s*(\\w+)ps");
    QRegularExpressionMatch match = regex.match(bitrateStr);
    if (match.hasMatch()) {
        double value = match.captured(1).toDouble();
        QString unit = match.captured(2).toLower();
        
        if (unit == "g") return value * 1000000000;
        if (unit == "m") return value * 1000000;
        if (unit == "k") return value * 1000;
        return value; // bps
    }
    return 0;
}

// NEW: Parse FPS value helper
double MultiRTMPWebsocketVendor::ParseFpsValue(const QString& fpsStr) {
    // Example: "30 FPS"
    QRegularExpression regex("(\\d+)\\s*FPS");
    QRegularExpressionMatch match = regex.match(fpsStr);
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    return 0;
}

PushWidget* MultiRTMPWebsocketVendor::FindPushWidgetByIdOrName(obs_data_t* request_data) {
    const char* targetId = obs_data_get_string(request_data, "id");
    const char* targetName = obs_data_get_string(request_data, "name");
    
    if (!targetId && !targetName) {
        return nullptr;
    }
    
    auto mainwin = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainwin) return nullptr;
    
    // ✅ FIX: The dock widget has the name "obs-multi-rtmp-dock" but it's a QDockWidget
    // We need to find the actual MultiOutputWidget inside it
    auto dock = mainwin->findChild<QDockWidget*>("obs-multi-rtmp-dock");
    if (!dock) {
        blog(LOG_WARNING, TAG "Dock widget not found");
        return nullptr;
    }
    
    // ✅ Get the MultiOutputWidget from the dock's widget
    auto multiOutputWidget = qobject_cast<MultiOutputWidget*>(dock->widget());
    if (!multiOutputWidget) {
        blog(LOG_WARNING, TAG "Failed to cast dock widget to MultiOutputWidget");
        return nullptr;
    }
    
    if (targetId) {
        return multiOutputWidget->FindPushWidgetById(QString::fromUtf8(targetId));
    }
    
    // If only name is provided, search by name
    auto pushWidgets = multiOutputWidget->GetAllPushWidgets();
    for (auto* widget : pushWidgets) {
        if (widget->GetTargetName() == QString::fromUtf8(targetName)) {
            return widget;
        }
    }
    
    return nullptr;
}