# Fork of main obs-multi-rtmp adding OBS websocket server API support

Adds OBS websocket server API support to the obs-multi-rtmp plugin.

## Complete Websocket API Function List:
ListTargets - Get all configured RTMP targets  
GetTargetState - Get current state of a specific target  
StartTarget - Start a specific target  
StopTarget - Stop a specific target  
ToggleTarget - Toggle start/stop state of a target  
StartAll - Start all targets  
StopAll - Stop all targets  
AddTarget - Add a new target  
CloneTarget - Clone an existing target  
UpdateTargetName - Update target name  
UpdateStreamKey - Update target stream key  
UpdateServiceParam - Update service parameter  
DeleteTarget - Delete a target  
UpdateSyncStart - Sync start with OBS  
UpdateSyncStop - Sync stop with OBS  
GetTargetStats - Get stats of a specific target  

## Python test files:
Look in the folder python_tests for 9 test files that implement each of the API functions for confirmation of websocket functionality

## Notes
I could only compile this for Windows PC. If anyone can help compile and/or package for Mac OS, Linux or Windows installer please let me know!

## Added files:
- multi-output-widget.cpp  
- multi-output-widget.h  
- ws_vendor.cpp  
- ws_vendor.hpp  

## Altered files from source:
CMakeLists.txt  
  - Added websocket support:  
  - lines 41-60  
  - lines 79-82  
  - line 88 ??

pch.h  
  - altered line 27-30 & 37-40 for websocket support

obs-multi-rtmp.cpp  
  - Changes for websocket support:  
  - lines 12-14  
  - line 55-57  
  - line 71 & 87-92  
  - line 99-104  

push-widget.h  
  - Updated for websocket support:  
  - line 1-2  
  - line 6-7  
  - line 9-12  
  - line 19-27  

push-widget.cpp  
  - Updated for websocket support:  
  - line 13-18  
  - line 555  
  - ** I'm not sure if this is all the changes, compare them! **  

I used obs-websocket-5.6.3 for this version found here: https://github.com/obsproject/obs-websocket  
You specifically need to include obs-websocket-api.h  
I added it here: obs-multi-rtmp\\.deps\obs-studio-31.0.0\plugins\obs-websocket\obs-websocket-api.h  

# Build commands used for compiling on PC:
cls  
cd "C:\projects\v2 - 0.7.3\obs-multi-rtmp" -> adjust for your path!  
rmdir /s /q build  
mkdir build  
cd build  
cls  
cmake .. -G "Visual Studio 17 2022" -A x64 -DENABLE_QT=ON -DENABLE_FRONTEND_API=ON -DENABLE_WEBSOCKET=ON  
cmake --build . --config Release  

# [Homepage / 主页](https://sorayuki.github.io/obs-multi-rtmp)

## 为什么首页是日语？ / Why is the homepage in Japanese?

因为最初是做给管人用的。

Because it's originally made for virtual Youtubers (VTubers).

# 声明 

近日发现百度贴吧有个叫 maggot 的用户在售卖此插件。咸鱼上也有，没得救了。 

本插件免费使用，作者不收取费用。 

举报之后百度贴吧找我要软件著作权证明，累不爱。 


# Announcement

This plugin is provided for free, without a fee. 

Recently a Baidu Tieba account 'maggot' is selling this plugin. Please, don't buy it.


# お知らせ

本プラグインは無償で提供されるものです。

最近、Baidu Tiebaに「maggot」というアカウント名のユーザーがこのプラグインを販売する行為をしています。

決して購入はしないでください。


# Donate

如果你觉得这个工具很有用想要捐赠，这里是链接。注意：这不是提需求的渠道。

このツールの開発に支援もとい投げ銭をしたいと思った方は以下のリンクからお願いします。(機能のリクエストは受け付けていません)

If you find this tool useful and want to doante, here is the link. (Please do not donate for feature requests.)

## [paypal / 贝宝](https://paypal.me/sorayuki0)

## alipay / 支付宝

![alipay](./docs/zhi.png) 

## wechat / 微信
![wechat](./docs/wechat.jpg)

## Build

This project uses obs-plugintemplate.   
Please refer to obs-plugintemplate to understand how it works.
